#include <click/config.h>
#include <elements/local/launch.hh>
#include <elements/local/launchrcvrlock.hh>
#include <click/args.hh>
#include <clicknet/ether.h>
#include <click/error.hh>
#include <click/glue.hh>
#include <click/confparse.hh>
#include <click/straccum.hh>
CLICK_DECLS


LaunchLockResponder::LaunchLockResponder()
: _lock_timeout_timer(this)
{
	_locked_channel = 0;
	_lock_count = 0;
}

LaunchLockResponder::~LaunchLockResponder()
{
}

int
LaunchLockResponder::configure(Vector<String> &conf, ErrorHandler * errh)
{
	if (Args(conf, this, errh)
		.read_mp("IP", _ip)
		.read_mp("DEVNAME", _ifname)
		.read_mp("ETH", _my_eth)
		.read_mp("WAIT", _lock_timeout_ms)
		.read_mp("CH0", _pu_behavior0)
		.read_mp("CH1", _pu_behavior1)
		.read_mp("CH2", _pu_behavior2)
		.read_mp("FROM_DEV", reinterpret_cast<Element *&>(_from_dev))
		.read_mp("ROUTER", reinterpret_cast<Element *&>(_router))
		.complete() < 0)
      return -1;

	_lock_timeout_timer.initialize(this);

	_lock_timeout_timer.schedule_after_msec(_lock_timeout_ms);
	return 0;
}



Packet *
LaunchLockResponder::simple_action(Packet *p_in)
{
	WritablePacket *p = p_in->uniqueify();
	struct launch_ctrl_hdr * lauch_hdr_ptr = (launch_ctrl_hdr *) (p->data()+14);

	lauch_hdr_ptr->neighbor_ip = _ip;
	
	//Need to be modified 
	//Don't understand it
	if(_lock_count ==0)
	{
		char buffer [3000];
		int n;
		//click_chatter("sudo iwconfig %s channel %d",_ifname.c_str(),lauch_hdr_ptr->channel);
		
		n = sprintf (buffer, "sudo ifconfig %s down",_ifname.c_str());
		n = system(buffer);
		n = sprintf (buffer, "sudo iwconfig %s essid teetteet%d channel %d key off mode Ad-Hoc",_ifname.c_str(),lauch_hdr_ptr->channel, lauch_hdr_ptr->channel);
		n = system(buffer);
		n = sprintf (buffer, "sudo ifconfig %s %s netmask 255.255.255.0 broadcast 10.0.0.255 promisc",_ifname.c_str(),_ip.unparse().c_str());
		n = system(buffer);
		n = sprintf (buffer, "sudo ifconfig %s up",_ifname.c_str());
		n = system(buffer);
		ErrorHandler  teet;
		_from_dev->initialize(&teet);

		_locked_channel = lauch_hdr_ptr->channel;
		lauch_hdr_ptr->type = launch_ctrl_hdr::LAUNCH_LOCK_RES;
		lauch_hdr_ptr->lock_response = 1;
		_lock_count++;
		click_ether *ethh = p->ether_header();
		uint8_t source_address[6];
		memcpy(source_address, ethh->ether_shost, 6);
		memcpy(ethh->ether_shost, ethh->ether_dhost, 6);
		memcpy(ethh->ether_dhost, source_address, 6);

		return p;
	}
	if((_lock_count > 0 && lauch_hdr_ptr->channel == _locked_channel))
	{
		
		_locked_channel = lauch_hdr_ptr->channel;
		lauch_hdr_ptr->type = launch_ctrl_hdr::LAUNCH_LOCK_RES;
		lauch_hdr_ptr->lock_response = 1;
		_lock_count++;
		click_ether *ethh = p->ether_header();
		uint8_t source_address[6];
		memcpy(source_address, ethh->ether_shost, 6);
		memcpy(ethh->ether_shost, ethh->ether_dhost, 6);
		memcpy(ethh->ether_dhost, source_address, 6);

		return p;
	}
	lauch_hdr_ptr->type = launch_ctrl_hdr::LAUNCH_LOCK_RES;
	lauch_hdr_ptr->lock_response = 0;

	switch(_locked_channel)
	{
		case 1:
			lauch_hdr_ptr->channel = 1;
			lauch_hdr_ptr->pu_behavior = _pu_behavior0;
			break;
		case 6:
			lauch_hdr_ptr->channel = 6;
			lauch_hdr_ptr->pu_behavior = _pu_behavior1;
			break;
		case 11:
			lauch_hdr_ptr->channel = 11;
			lauch_hdr_ptr->pu_behavior = _pu_behavior2;
			break; 
	}


	click_ether *ethh = p->ether_header();
	uint8_t source_address[6];
	memcpy(source_address, ethh->ether_shost, 6);
	memcpy(ethh->ether_shost, ethh->ether_dhost, 6);
	memcpy(ethh->ether_dhost, source_address, 6);


	return p;

}


void
LaunchLockResponder::run_timer(Timer * t)
{
	if(_lock_count > 0)
	{
		_lock_count--;
	}
	_lock_timeout_timer.schedule_after_msec(_lock_timeout_ms);
}


int
LaunchLockResponder::pu_sensed(const String &s, Element *e, void *, ErrorHandler *errh)
{
    LaunchLockResponder *llr = static_cast<LaunchLockResponder *>(e);
	
	uint32_t channel_sensed = atoi(s.c_str());

    click_chatter("sensed channel %d ya ged3aaaaaaaaaaaaaaaaan", channel_sensed);
	
	//this machine can no longer use the sensed channel
	llr->_router->cant_use_channel(channel_sensed);


	if(llr->_locked_channel == channel_sensed)
	{	
		llr->_locked_channel = 0;
		llr->_lock_count = 0;
	}
	


	
	//neighbors shouldn't communicate with this machine on the sensed channel
	//send LOCK_RES to update the state of the lock
	struct launch_ctrl_hdr  _lh;

	if (llr->_pu_behavior0 >= llr->_pu_behavior1 && llr->_pu_behavior0 >= llr->_pu_behavior2 && llr->_router->can_use_1)
	{
		_lh.channel=1;
		_lh.pu_behavior = llr->_pu_behavior0;
	}
	else if (llr->_pu_behavior1 >= llr->_pu_behavior0 && llr->_pu_behavior1 >= llr->_pu_behavior2 && llr->_router->can_use_6)
	{
		_lh.channel=6;
		_lh.pu_behavior = llr->_pu_behavior1;
	}
	else if (llr->_pu_behavior2 >= llr->_pu_behavior0 && llr->_pu_behavior2 >= llr->_pu_behavior1 && llr->_router->can_use_11)
	{
		_lh.channel=11;
		_lh.pu_behavior = llr->_pu_behavior2;
	}
	else
	{
		_lh.channel=11;
		_lh.pu_behavior = 0;
	}

	
	WritablePacket *packet = Packet::make((void *)&_lh, sizeof(_lh));

	if (packet == 0 )
	{	
		click_chatter( "cannot make packet!");
		return 0;
	} 
	
	struct launch_ctrl_hdr * format = (struct launch_ctrl_hdr *) packet->data();
	
	
	format->type = launch_ctrl_hdr::LAUNCH_LOCK_RES;
	format->lock_response = 0;
	
	//Push Ethernet header
	struct click_ether ethh;
	

	memset(ethh.ether_dhost,0xff,6);

	memcpy(ethh.ether_shost, llr->_my_eth.data(), 6);
	ethh.ether_type = 0x0007;
 
	WritablePacket *q = packet->push(14);
	memcpy(q->data(), &ethh, 14);
	
	llr->output(0).push(q);

    return 0;
}


void
LaunchLockResponder::add_handlers()
{
    add_write_handler("pu_sensed", pu_sensed, 0);
}




CLICK_ENDDECLS
EXPORT_ELEMENT(LaunchLockResponder)

