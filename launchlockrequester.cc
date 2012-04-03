
#include <click/config.h>
#include <elements/launch/launch.hh>
#include <elements/launch/launchlockrequester.hh>
#include <click/args.hh>
#include <click/error.hh>
#include <click/glue.hh>

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
LAUNCHLockRequester::send_lock_request(uint8_t channel, IPAddress distination_ip,EtherAddress distination_eth )
{
	
	int tailroom = 0;
	int packetsize = sizeof(_lh);
	int headroom = 0; 

	WritablePacket *packet = Packet::make(headroom,0,packetsize, tailroom);
	if (packet == 0 )
		return click_chatter( "cannot make packet!");
	
	memset(packet->data(), 0, packet->length());
	struct launch_ctrl_hdr * format = (struct launch_ctrl_hdr *) packet->data();
	
	
	format->type = launch_ctrl_hdr::LAUNCH_LOCK_REQ;
	format->channel = channel;

	memcpy(packet->data(), &_lh, sizeof(_lh));
	
	//annotate packet with distination for wrapper
	
	output(0).push(packet);
}


CLICK_ENDDECLS
EXPORT_ELEMENT(LAUNCHRequester)

