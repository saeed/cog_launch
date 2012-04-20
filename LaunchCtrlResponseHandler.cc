#include <click/config.h>
#include <elements/local/launch.hh>
#include <elements/local/launchrouter.hh>
#include <elements/local/LaunchCtrlResponseHandler.hh>
#include <click/args.hh>
#include <clicknet/ether.h>
#include <click/error.hh>
#include <click/glue.hh>
#include <click/confparse.hh>
CLICK_DECLS


LaunchCtrlResponseHandler::LaunchCtrlResponseHandler()
{
}

LaunchCtrlResponseHandler::~LaunchCtrlResponseHandler()
{
}

int
LaunchCtrlResponseHandler::configure(Vector<String> &conf, ErrorHandler * errh)
{
	if (Args(conf, this, errh)
      .read_mp("ROUTER", reinterpret_cast<Element *&>(_router))
      .complete() < 0)
      return -1;
	
	return 0;
}




Packet *
LaunchCtrlResponseHandler::simple_action(Packet *p_)
{
  struct launch_ctrl_hdr * format = (struct launch_ctrl_hdr *) p_->data();
  WritablePacket * p_in = p_->uniqueify();
  //Packet should come here annotated with neighbor IP and ethernet address
  _router->insert_route(p_in->dst_ip_anno(),
	      format->my_lat, format->my_long, 
	       p_in->get_dst_eth_anno(), 	format->channel,
	       	format->pu_behavior, format->switching_time);
  
    return 0;
}

CLICK_ENDDECLS
EXPORT_ELEMENT(LaunchCtrlResponseHandler)

