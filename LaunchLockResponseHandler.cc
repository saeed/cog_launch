#include <click/config.h>
#include <elements/local/launch.hh>
#include <elements/local/launchrouter.hh>
#include <elements/local/launchlockrequester.hh>
#include <elements/local/LaunchLockResponseHandler.hh>
#include <click/args.hh>
#include <clicknet/ether.h>
#include <click/error.hh>
#include <click/glue.hh>
#include <click/confparse.hh>
#include <unistd.h>
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
	  .read_mp("IF", _ifname)
	  .read_mp("IP", _ip)
	  //.read_mp("ETH", _eth)
      .read_mp("ROUTER", reinterpret_cast<Element *&>(_router))
	  .read_mp("TO_DEV", reinterpret_cast<Element *&>(_to_dev))
      .complete() < 0)
      return -1;
	
	return 0;
}




Packet *
LaunchLockResponseHandler::simple_action(Packet *p_in)
{
	struct launch_ctrl_hdr * launch_hdr_ptr = (struct launch_ctrl_hdr *) (p_in->data()+14);
	click_chatter("channel lock positive ya3ny el mfrood 5alas keda");
	if(launch_hdr_ptr->lock_response == 1)
	{
		char buffer [3000];
		int n;
		
		n = sprintf (buffer, "sudo ifconfig %s down",_ifname.c_str());
		n = system(buffer);
		n = sprintf (buffer, "sudo iwconfig %s essid b7awel channel %d key off mode Ad-Hoc",_ifname.c_str(),launch_hdr_ptr->channel);//,launch_hdr_ptr->channel);
		n = system(buffer);
		
		n = sprintf (buffer, "sudo ifconfig %s %s netmask 255.255.255.0 broadcast 10.0.0.255",_ifname.c_str(),_ip.unparse().c_str());
		n = system(buffer);
		n = sprintf (buffer, "sudo ifconfig %s up",_ifname.c_str());
		n = system(buffer);
		ErrorHandler  teet;
		_to_dev->initialize(&teet);
		//sleep(10);

		_router->set_channel_loc_positive();
	}
	else
	{

//	RouteEntry temp = _router_rtes->findp(launch_hdr_ptr->neighbor_ip);
 	//const click_ether *ethh = p_in->ether_header();
    
   	//uint8_t source_address[6];
    //	memcpy(source_address, ethh->ether_shost, 6);
		_router->set_channel_loc_negative();
		_router->update_route(launch_hdr_ptr->neighbor_ip, launch_hdr_ptr->channel);
			
//			RouteEntry best_neighbor = _router.choose_bestneighbor();	
//		_lock_requester.send_lock_request(best_neighbor.channel/*channel selected*/, best_neighbor.neighbor_ip/*lock distantion ip*/, best_neighbor.neighbor_eth/*lock distantion eth*/,_eth);
	}
    return 0;
}


CLICK_ENDDECLS
EXPORT_ELEMENT(LaunchLockResponseHandler)

