#include <click/config.h>
#include <elements/local/launch.hh>
#include <elements/local/launchrouter.hh>
#include <elements/local/launchrequester.hh>
#include <elements/local/launchlockrequester.hh>
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
	_routingtable_available = false;
	_channel_lock_positive = false;
	_ready_for_another_packet = true;

	can_use_11= true;
	can_use_6 = true;
	can_use_1 = true;

	IPAddress n1 ("10.0.0.1");
	LocationEntry n1_l(n1, 1, 1);
	_ltable.insert(n1, n1_l);
	
	IPAddress n2 ("10.0.0.2");
	LocationEntry n2_l(n1, 2, 2);
	_ltable.insert(n2, n2_l);
	
	IPAddress n3 ("10.0.0.3");
	LocationEntry n3_l(n1, 3, 3);
	_ltable.insert(n3, n3_l);
	
	IPAddress n4 ("10.0.0.4");
	LocationEntry n4_l(n1, 4, 4);
	_ltable.insert(n4, n4_l);
	
}

LaunchRouter::~LaunchRouter()
{
}

int
LaunchRouter::configure(Vector<String> &conf, ErrorHandler * errh)
{
	if (Args(conf, this, errh)
      .read_mp("ETH", _ether_address_eth)
	  .read_mp("PU", _pu_behavior)
	  .read_mp("RES_T", _repsonse_waiting_ms)
	  .read_mp("LOCK_T", _lock_waiting_ms)
	  .read_mp("RT_T", _routing_table_entry_ms)
	  .read_p("REQUESTER", reinterpret_cast<Element *&>(_requester))
	  .read_p("LOCK_REQUESTER", reinterpret_cast<Element *&>(_lock_requester))
	  .complete() < 0)
      return -1;

	memcpy(_eth ,_ether_address_eth.data(),sizeof(_eth));
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
	if(_ready_for_another_packet)
	{
		click_chatter("i found it ready :)");
		_holded_packet = p_in->uniqueify();
		packets_holded.push_back(_holded_packet);
		//get dst ip from packet's annotations for later use	
		_dst_ip = _holded_packet->dst_ip_anno();
	
		//Forward packet if routingtable is available and we have lock 
		//call launchlocrequester if routingtable is available and we don't have lock
		//call launchctrlrequester to send REQ
		if(_routingtable_available &&  _channel_lock_positive)
		{
			RouteEntry * current_best_neighbor = choose_bestneighbor(_dst_ip,_rtes);
			IPAddress current_neighbor_ip  = current_best_neighbor->neighbor_ip;
			if(current_neighbor_ip == locked_neighbor_ip)
			{
				_holded_packet->set_dst_ip_anno(locked_neighbor_ip /*from calculating the metric*/);
				output(0).push(_holded_packet);
			}
			else
			{
				_lock_requester->send_lock_request(current_best_neighbor->channel/*channel selected*/, current_best_neighbor->neighbor_ip/*lock distantion ip*/, current_best_neighbor->neighbor_eth/*lock distantion eth*/,_eth);
				_lock_waiting_timer.schedule_after_msec(_lock_waiting_ms);
			}
			return 0;
			//calculate metric, annotate packet with distenation and output packet
		}
		else if(_routingtable_available)
		{
		
			RouteEntry * best_neighbor = choose_bestneighbor(_dst_ip,_rtes);	
			_lock_requester->send_lock_request(best_neighbor->channel/*channel selected*/, best_neighbor->neighbor_ip/*lock distantion ip*/, best_neighbor->neighbor_eth/*lock distantion eth*/,_eth/*my ethernet*/);
			_lock_waiting_timer.schedule_after_msec(_lock_waiting_ms);
		
			return 0;
		}
		else
		{
			click_chatter("send request and make it not ready");
			_requester->send_request();
			_respone_waiting_timer.schedule_after_msec(_repsonse_waiting_ms);
			_ready_for_another_packet = false;
			return 0;
		}
	}
	else
	{	
		click_chatter("i found it not ready");
				
		_holded_packet = p_in->uniqueify();
		click_chatter("%d",sizeof(_holded_packet->data()) );
		click_chatter("%x",_holded_packet->data() );
		
		packets_holded.push_back(_holded_packet);
		return 0;
	}
}


// Called by the _respone_waiting_timer when it times out
// to chech if the routingtable was loaded or not
void 
LaunchRouter::use_responses()
{
	
	if(_routingtable_available)
	{
		click_chatter("routing table became available");
		//lookup table and calculate the metric to choose next hop
		//issue lock request
		RouteEntry * best_neighbor = choose_bestneighbor(_dst_ip,_rtes);	
		locked_neighbor_ip = best_neighbor->neighbor_ip;

		//click_chatter("howa da el best neighbor");
		//EtherAddress tempo(best_neighbor->neighbor_eth);

		//click_chatter(tempo.unparse().c_str());
		//click_chatter(best_neighbor->neighbor_ip.unparse().c_str());

			
		_lock_requester->send_lock_request(best_neighbor->channel/*channel selected*/, best_neighbor->neighbor_ip/*lock distantion ip*/, best_neighbor->neighbor_eth/*lock distantion eth*/,_eth);
		_lock_waiting_timer.schedule_after_msec(_lock_waiting_ms);
		
	}
	else
	{
		_requester->send_request();
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
	if(_channel_lock_positive && _routingtable_available)
	{
		click_chatter("routing table available and lock received -> packet sent");		
		//annotate packet with distenation and output packet
		_holded_packet->set_dst_ip_anno(locked_neighbor_ip/*from calculating the metric*/);
		output(0).push(_holded_packet);
		packets_holded.pop_front();


		while (packets_holded.size() > 1)
		{

			Packet * value = packets_holded.front();
			RouteEntry * current_best_neighbor = choose_bestneighbor(value->dst_ip_anno(),_rtes);
			IPAddress current_neighbor_ip  = current_best_neighbor->neighbor_ip;
			if(current_neighbor_ip == locked_neighbor_ip)
			{
				click_chatter("%d",sizeof(value->data()) );
				click_chatter("%x",value->data() );
				value->set_dst_ip_anno(locked_neighbor_ip /*from calculating the metric*/);
				output(0).push(value);
				packets_holded.pop_front();
				click_chatter("trying to send holded packets");
			}
			else
			{
				_dst_ip = value->dst_ip_anno();
				_lock_requester->send_lock_request(current_best_neighbor->channel/*channel selected*/, current_best_neighbor->neighbor_ip/*lock distantion ip*/, current_best_neighbor->neighbor_eth/*lock distantion eth*/,_eth);
				_lock_waiting_timer.schedule_after_msec(_lock_waiting_ms);
				return;
			}
		} 

		_ready_for_another_packet = true;
	}
	else
	{
		_respone_waiting_timer.schedule_after_msec(_repsonse_waiting_ms);
	}
}

void 
LaunchRouter::make_routetable_expire()
{
	//for (RTIter iter = _rtes.begin(); iter.live(); iter++) {
	//	const RouteEntry &rte = iter.value();
	//	_rtes.remove(rte.neighbor_ip);
	//}
	//_routingtable_available = false;
}



//Should be called by LaunchCtrlResponseHandler


void
LaunchRouter::insert_route(const IPAddress &nip,
	      uint32_t nlat, uint32_t nlong, 
	       uint8_t * ne, 	uint8_t chl,
	       	uint32_t pub, 	uint32_t swt)
{
	_routingtable_available = true;
	_routing_table_entry_timer.reschedule_after_msec(_routing_table_entry_ms);
	
	
	RouteEntry * temp = _rtes.findp(nip);
	if(temp != NULL)
	{
  	
		temp->neighbor_lat = nlat;
		temp->neighbor_long = nlong;
		temp->channel = chl;
		temp->pu_behavior = pub;
		temp->switching_time = swt;
	}
	else
	{
		
		RouteEntry r(nip, nlat, nlong, ne, chl, pub, swt);
		_rtes.insert(nip, r);
	}
}





void
LaunchRouter::update_route(const IPAddress &nip, 	uint8_t chl)
{
	RouteEntry* temp = _rtes.findp(nip);
	temp->channel = chl;
}



/*
// Function to pick the best neighbor in terms of the launch metric
RouteEntry 
LaunchRouter::choose_bestneighbor(IPAddress _current_dst_addr)
{

	if(_rtes.findp(_current_dst_addr) != 0)
	return _rtes.findp(_current_dst_addr);
	
	double last_metric = 10000;
	uint8_t best_ip ;
	double current_metric;
	for (RTIter iter = _rtes.begin(); iter.live(); iter++) {
		if(_current_dst_addr == iter.neighbor_ip)
		{
			best_ip = rte.neighbor_ip;
			break;
		}

		RouteEntry rte = iter.value();

		LocationEntry lentry =  _ltable.find(rte.neighbor_ip);

		current_metric = calculate_metric(rte, lentry);
		if(current_metric < last_metric)
		{
			last_metric = current_metric;
			best_ip = rte.neighbor_ip;
		}	
	}

	return _rtes.findp(best_ip);
}
*/
//Function to calculate the metric for certain neighbor in the table
double 
LaunchRouter::calculate_metric(RouteEntry r, LocationEntry l)
{


	uint8_t distance_value = distance(((double)l.neighbor_lat/1000.00),((double)l.neighbor_long/1000.00),((double)r.neighbor_lat/1000.00), ((double)r.neighbor_long/1000.00),'M');

	return ((distance_value/(3*(10^6)))  + r.switching_time)/(1- r.pu_behavior*_pu_behavior);
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

void 
LaunchRouter::set_channel_loc_positive(){
	_channel_lock_positive = true;	
}

void 
LaunchRouter::set_channel_loc_negative()
{
	_channel_lock_positive = false;	
}

void 
LaunchRouter::cant_use_channel(int channel_number)
{
	switch(channel_number)
	{
		case 1:
			can_use_1 =false;
			break;
		case 6:
			can_use_6 =false;
			break;
		case 11:
			can_use_11 =false;
			break;
	}
	if(channel_used == channel_number)
		set_channel_loc_negative();
}


CLICK_ENDDECLS
EXPORT_ELEMENT(LaunchRouter)
