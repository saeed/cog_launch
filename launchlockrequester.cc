#include <click/config.h>
#include <elements/local/launch.hh>
#include <elements/local/launchlockrequester.hh>
#include <click/args.hh>
#include <click/error.hh>
#include <click/glue.hh>
#include <clicknet/ether.h>

CLICK_DECLS

LaunchLockRequester::LaunchLockRequester()
{
}

LaunchLockRequester::~LaunchLockRequester()
{
}

int
LaunchLockRequester::configure(Vector<String> &conf, ErrorHandler *errh)
{
   
	return 0;
}

int
LaunchLockRequester::initialize(ErrorHandler *)
{
	return 0;
}



// Function called from LaunchRouter to send lock request packets on certain channel to certain neighbor
void
LaunchLockRequester::send_lock_request(uint8_t channel, IPAddress distination_ip,uint8_t * destination_eth, uint8_t * source_eth )
{
	
	WritablePacket *packet = Packet::make((void *)&_lh, sizeof(_lh));
	
	if (packet == 0 )
	return click_chatter( "cannot make packet!");
	
	struct launch_ctrl_hdr * format = (struct launch_ctrl_hdr *) packet->data();
	
	format->type = launch_ctrl_hdr::LAUNCH_LOCK_REQ;
	format->channel = channel;
	
	//Push Ethernet header
	 struct click_ether ethh;
	 memcpy(ethh.ether_shost, source_eth, sizeof(ethh.ether_shost));
	 memcpy(ethh.ether_dhost, destination_eth, sizeof(ethh.ether_dhost));
	 ethh.ether_type = 0x0700;
	 
	WritablePacket *q = packet->push(14);
	memcpy(q->data(), &ethh, 14);
	
	
	
	output(0).push(q);
}


CLICK_ENDDECLS
EXPORT_ELEMENT(LaunchLockRequester)

