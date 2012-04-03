#include <click/config.h>
#include <elements/local/launch.hh>
#include <elements/local/launchrouter.hh>
#include <click/args.hh>
#include <clicknet/ether.h>
#include <click/error.hh>
#include <click/glue.hh>
#include <click/confparse.hh>

#include <math.h>
#define pi 3.14159265358979323846
CLICK_DECLS


LaunchRouter::LaunchRouter()
: _respone_waiting_timer(static_use_responses, this), _lock_waiting_timer(static_use_lock, this), _routing_table_entry_timer(static_make_routetable_expire, this)
{
}

LaunchRouter::~LaunchRouter()
{
}

int
LaunchRouter::configure(Vector<String> &conf, ErrorHandler * errh)
{
	if (Args(conf, this, errh)
      .read_mp("ETH", _eth)
      .read_mp("IP", _ip)
	  .read_mp("PU", _pu_behavior)
	  .read_mp("RES_T", _repsonse_waiting_ms)
	  .read_mp("LOCK_T", _lock_waiting_ms)
	  .read_mp("RT_T", _routing_table_entry_ms)
	  .read_p("REQUESTER", reinterpret_cast<Element *&>(_requester))
	  .read_p("LOCK_REQUESTER", reinterpret_cast<Element *&>(_lock_requester))
	  .complete() < 0)
      return -1;

	
	return 0;
}

int 
LaunchRouter::initialize(ErrorHandler *)
{
	_respone_waiting_timer.initialize(this);
	_lock_waiting_timer.initialize(this);
	_routing_table_entry_timer.initialize(this);

}

Packet *
LaunchRouter::simple_action(Packet *p_in)
{
	//look up next hop for packet (route table has entries) and there is a positive channel lock
	//if exists annotate packet and push to next element
	//if not start response timer and broadcast lanch request
	_holded_packet = p_in->uniqueify();
	//Forward packet if routingtable is available and we have lock 
	//call launchlocrequester if routingtable is available and we don't have lock
	//call launchctrlrequester to send REQ
	if(_routingtable_available &&  _channel_lock_positive)
	{
		
		_holded_packet.set_dst_ip_anno(choose_bestneighbor().neighbor_ip /*from calculating the metric*/);
		output(0).push(_holded_packet);
		//calculate metric, annotate packet with distenation and output packet
	}
	else if(_routingtable_available)
	{
		
		RouteEntry best_neighbor = choose_bestneighbor();	
		_lock_requester.send_lock_request(best_neighbor.channel/*channel selected*/, best_neighbor.neighbor_ip/*lock distantion ip*/, best_neighbor.neighbor_eth/*lock distantion eth*/);
		_lock_waiting_timer.schedule_after_msec(_lock_waiting_ms);
	
	}
	else
	{
		_requester.send_request();
		_respone_waiting_timer.schedule_after_msec(_repsonse_waiting_ms);
	}
}

// Called by the _respone_waiting_timer when it times out
// to chech if the routingtable was loaded or not
void 
LaunchRouter::use_responses()
{
	
	
	if(_routingtable_available)
	{
		//lookup table and calculate the metric to choose next hop
		//issue lock request
		RouteEntry best_neighbor = choose_bestneighbor();	
		_lock_requester.send_lock_request(best_neighbor.channel/*channel selected*/, best_neighbor.neighbor_ip/*lock distantion ip*/, best_neighbor.neighbor_eth/*lock distantion eth*/);
		_lock_waiting_timer.schedule_after_msec(_lock_waiting_ms);
	}
	else
	{
		_requester.send_request();
		_respone_waiting_timer.schedule_after_msec(_repsonse_waiting_ms);
	}
}

// Called by the _lock_waiting_timer when it times out to check if lock was obtained
void 
LaunchRouter::use_lock()
{
	//check whether lock response is positive or negative
	//if positive push packet
	//if negative wait again
	if(_channel_lock_positive)
	{
		//annotate packet with distenation and output packet
		_holded_packet.set_dst_ip_anno(choose_bestneighbor().neighbor_ip /*from calculating the metric*/);
		output(0).push(_holded_packet);

	}
	else
	{
		_respone_waiting_timer.schedule_after_msec(_repsonse_waiting_ms);
	}
}

void 
LaunchRouter::make_routetable_expire()
{
	for (RTIter iter = _rtes.begin(); iter.live(); iter++) {
		const RTEntry &rte = iter.value();
		_rtes.remove(rte);
	}
	_routingtable_available = false;
}



//Should be called by LaunchCtrlResponseHandler


void
LaunchRouter::insert_route(const IPAddress &nip,
	      uint32_t nlat, uint32_t nlong, 
	       const EtherAddress &ne, 	uint8_t chl,
	       	uint32_t pub, 	uint32_t swt)
{
	_routingtable_available = true;
	_routing_table_entry_timer.reschedule_after_msec(_routing_table_entry_ms);
	
  
	RouteEntry temp = _rtes.findp(nip);
	if(temp != NULL)
	{
		temp.neighbor_lat = nlat;
		temp.neighbor_long = nlong;
		temp.channel = chl;
		temp.pu_behavior = pub;
		temp.switching_time = swt;
	}
	else
	{
		RouteEntry r(nip, nlat, nlong, ne, chl, pub, swt);
		_rtes.insert(nip, r);
	}
}


// Function to pick the best neighbor in terms of the launch metric
RouteEntry 
LaunchRouter::choose_bestneighbor()
{

	double last_metric = 10000;
	uint8_t best_ip ;
	double current_metric;
	for (RTIter iter = _rtes.begin(); iter.live(); iter++) {
		const RTEntry &rte = iter.value();
		current_metric = calculate_metric(rte);
		if(current_metric < last_metric)
		{
			last_metric = current_metric;
			best_ip = rte.neighbor_ip;
		}	
	}

	return _rtes.findp(best_ip);
}

//Function to calculate the metric for certain neighbor in the table
double 
LaunchRouter::calculate_metric(RouteEntry r)
{


	uint8_t distance = distance(((double)dst_lat/1000.00),((double)dst_long/1000.00),((double)r.neighbor_lat/1000.00), ((double)r.neighbor_long/1000.00),'M');

	return ((distance/(3*(10^6)))  + r.switching_time)/(1- r.pu_behavior*_pu_behavior);
}


//Function to calculate the distance between two nodes
double 
LaunchRouter::distance(double lat1, double lon1, double lat2, double lon2, char unit) {
	double theta, dist;
	theta = lon1 - lon2;
	dist = sin(deg2rad(lat1)) * sin(deg2rad(lat2)) + cos(deg2rad(lat1)) * cos(deg2rad(lat2)) * cos(deg2rad(theta));
	dist = acos(dist);
	dist = rad2deg(dist);
	dist = dist * 60 * 1.1515;
	switch(unit) {
		case 'M':
			break;
		case 'K':
			dist = dist * 1.609344;
			break;
		case 'N':
			dist = dist * 0.8684;
			break;
	}
	return (dist);
}


double 
LaunchRouter::deg2rad(double deg) {
	return (deg * pi / 180);
}


double 
LaunchRouter::rad2deg(double rad) {
	return (rad * 180 / pi);
}


CLICK_ENDDECLS
EXPORT_ELEMENT(LaunchRouter)