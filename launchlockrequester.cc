

#include <click/config.h>
#include <elements/launch/launch.hh>
#include <elements/launch/launchlockrequester.hh>
#include <click/args.hh>
#include <click/error.hh>
#include <click/glue.hh>

CLICK_DECLS

LAUNCHLockRequester::LAUNCHLockRequester()
{
}

LAUNCHLockRequester::~LAUNCHLockRequester()
{
}

int
LAUNCHLockRequester::configure(Vector<String> &conf, ErrorHandler *errh)
{
   
	return 0;
}

int
LAUNCHLockRequester::initialize(ErrorHandler *)
{
	return 0;
}

/*Packet *
LAUNCHLockRequester::simple_action(Packet *p_in)
{
	int extra = sizeof(_lh);
	WritablePacket *p = p_in->push(extra);
	if (!p)
		return 0;


	memcpy(p->data(), &_lh, sizeof(_lh));

	return p;
}*/



void
LAUNCHLockRequester::send_lock_request(uint8_t channel, IPAddress distination_ip,EtherAddress distination_eth )
{
	
	int tailroom = 0;
	int packetsize = sizeof(_lh);
	int headroom = 0; //sizeof(click_ip)+sizeof(click_udp)+sizeof(click_eth);

	WritablePacket *packet = Packet::make(headroom,0,packetsize, tailroom);
	if (packet == 0 )
		return click_chatter( "cannot make packet!");
	
	memset(packet->data(), 0, packet->length());
	struct launch_ctrl_hdr * format = (struct launch_ctrl_hdr *) packet->data();
	
	format->type = launch_ctrl_hdr::LAUNCH_LOCK_REQ;
	format->channel = channel;

	memcpy(packet->data(), &_lh, sizeof(_lh));
	
	//annotate packet with distenation for wrapper
	
	output(0).push(packet);
}


CLICK_ENDDECLS
EXPORT_ELEMENT(LAUNCHRequester)

