#include <click/config.h>
#include <elements/local/launch.hh>
#include <elements/local/launchrouter.hh>
#include <elements/local/launchlockrequester.hh>
#include <elements/local/launchlockresponsehandler.hh>
#include <click/args.hh>
#include <clicknet/ether.h>
#include <click/error.hh>
#include <click/glue.hh>
#include <click/confparse.hh>
CLICK_DECLS


LaunchLockResponseHandler::LaunchLockResponseHandler()
{
}

LaunchLockResponseHandler::~LaunchLockResponseHandler()
{
}

int
LaunchLockResponseHandler::configure(Vector<String> &conf, ErrorHandler * errh)
{
	if (Args(conf, this, errh)
	 .read_mp("ETH", _eth)
      .read_mp("ROUTER", reinterpret_cast<Element *&>(_router))
      .complete() < 0)
      return -1;
	
	return 0;
}




Packet *
LaunchLockResponseHandler::simple_action(Packet *p_in)
{
  struct launch_ctrl_hdr * launch_hdr_ptr = (struct launch_ctrl_hdr *) p_in->data();
  
  if(launch_hdr_ptr->lock_response == 1)
	_router.set_channel_lock_positive();
	else
	{

	RouteEntry temp = _router_rtes.findp(launch_hdr_ptr->neighbor_ip);
 	click_ether *ethh = p_in->ether_header()
    
   	uint8_t source_address[6];
    	memcpy(source_address, ethh->ether_shost, 6);

	_router.insert_route(launch_hdr_ptr->neighbor_ip,
	      temp.neighbor_lat, temp.neighbor_long, 
	       source_address, launch_hdr_ptr->channel,
	       	launch_hdr_ptr->pu_behavior, temp.neighbor_long);
			
			RouteEntry best_neighbor = _router.choose_bestneighbor();	
		_lock_requester.send_lock_request(best_neighbor.channel/*channel selected*/, best_neighbor.neighbor_ip/*lock distantion ip*/, best_neighbor.neighbor_eth/*lock distantion eth*/,_eth);
		}
    return 0;
}

CLICK_ENDDECLS
EXPORT_ELEMENT(LaunchLockResponseHandler)

