#include <click/config.h>
#include <elements/local/launch.hh>
#include <elements/local/launchrequester.hh>
#include <click/args.hh>
#include <click/error.hh>
#include <click/glue.hh>

CLICK_DECLS

LaunchCtrlRequester::LaunchCtrlRequester()
{
}

LaunchCtrlRequester::~LaunchCtrlRequester()
{
}

int
LaunchCtrlRequester::configure(Vector<String> &conf, ErrorHandler *errh)
{
  
  return 0;
}

int
LaunchCtrlRequester::initialize(ErrorHandler *)
{
  return 0;
}


//Function called by LaunchRouter to broadcast control packets to all the neighbors

void
LaunchCtrlRequester::send_request()
{
	_lh.type = launch_ctrl_hdr::LAUNCH_REQ;
	
	WritablePacket *packet = Packet::make((void *)&_lh, sizeof(_lh));
	
	if (packet == 0 )
		return click_chatter( "cannot make packet!");
	
	output(0).push(packet);
}


CLICK_ENDDECLS
EXPORT_ELEMENT(LaunchCtrlRequester)
