#ifndef Launch_Router_HH
#define Launch_Router_HH
#include <click/element.hh>
#include <click/glue.hh>
#include <click/ipaddress.hh>
#include <click/etheraddress.hh>
#include <clicknet/ether.h>
#include <elements/local/launch.hh>
#include <elements/local/launchrequester.hh>
#include <elements/local/launchlockrequester.hh>
#include <click/confparse.hh>
#include <click/timer.hh>
#include <click/hashmap.hh>
#include <list>		// list class library
using namespace std;
CLICK_DECLS

class LaunchRouter : public Element { public:

	LaunchRouter();
	~LaunchRouter();

	const char *class_name() const		{ return "LaunchRouter"; }
	const char *port_count() const		{ return PORTS_1_1; }
	const char *processing() const		{ return AGNOSTIC; }
	
	int configure(Vector<String> &, ErrorHandler *);
	int initialize(ErrorHandler *);
	
	Packet * simple_action(Packet *p_in);

	void insert_route( const IPAddress &nip,uint32_t nlat, uint32_t nlong,uint8_t * ne,uint8_t chl, uint32_t pub, uint32_t swt);
	void update_route(const IPAddress &nip, 	uint8_t chl);
	
	void cant_use_channel(int channel_number);
	
	void set_channel_loc_positive();
	void set_channel_loc_negative();

	bool can_use_11;
	bool can_use_6;
	bool can_use_1;

private:

	list<Packet *> packets_holded;

	Packet * _holded_packet;
	IPAddress _dst_ip;

	//IPAddress ip("10.0.0.5"); 	
	IPAddress _ip;
	EtherAddress _ether_address_eth;
	uint8_t _eth [6];

	double _pu_behavior;

	bool _channel_lock_positive;
	class IPAddress locked_neighbor_ip;

	bool _routingtable_available;

	bool _ready_for_another_packet;


	uint32_t my_lat;			// this node's Latitude.
	uint32_t my_long;			// this node's Longitude.


	/* destination node's addresses */
	IPAddress dst_ip;
	EtherAddress dst_eth;

	uint32_t dst_lat;			// this node's Latitude.
	uint32_t dst_long;			// this node's Longitude.

	
	/* timers 
	1. _respone_waiting_timer for waiting on responses to reach this node from its nieghbors
	2. _lock_waiting_timer for waiting on channel lock to reach this node from the node it chose as the next hop
	3. _routing_table_entry_timer for making route table expire */
	Timer _respone_waiting_timer;
	Timer _lock_waiting_timer;
	Timer _routing_table_entry_timer;

	uint32_t _repsonse_waiting_ms;
	uint32_t _lock_waiting_ms;	
	uint32_t _routing_table_entry_ms;

	static void static_use_responses(Timer *, void *e) { ((LaunchRouter *) e)->use_responses(); }
	void use_responses();

	static void static_use_lock(Timer *, void *e) { ((LaunchRouter *) e)->use_lock(); }
	void use_lock();

	static void static_make_routetable_expire(Timer *, void *e) { ((LaunchRouter *) e)->make_routetable_expire(); }
	void make_routetable_expire();
	

	LaunchCtrlRequester *_requester;
	LaunchLockRequester * _lock_requester;
  
	//Metric Calculation Function

	double distance(double lat1, double lon1, double lat2, double lon2, char unit);
	double deg2rad(double deg);
	double rad2deg(double deg);

	//Routing Table Entry

	class RouteEntry {

	public:
		class IPAddress  neighbor_ip;      // IP address of this destination

		uint32_t neighbor_lat;			// Sender's Latitude.
		uint32_t neighbor_long;			// Sender's Longitude.

		uint8_t   neighbor_eth[6]; // hardware address of next hop

		uint8_t channel;
		uint32_t pu_behavior;
		uint32_t switching_time;



		RouteEntry(const IPAddress &nip,
			  uint32_t nlat, uint32_t nlong, 
			   uint8_t * ne, 	uint8_t chl,
			   	uint32_t pub, 	uint32_t swt) :
		  neighbor_ip(nip), neighbor_lat(nlat), neighbor_long(nlong), 
		  channel(chl), switching_time(swt)
		{memcpy(neighbor_eth,ne,6);}

		RouteEntry() {}
		~RouteEntry() {}
	};


	class LocationEntry {

	public:
		class IPAddress  neighbor_ip;      // IP address of this destination

		uint32_t neighbor_lat;			// Sender's Latitude.
		uint32_t neighbor_long;			// Sender's Longitude.
		class EtherAddress   neighbor_eth; // hardware address of next hop

		LocationEntry(const IPAddress &nip,
			  uint32_t nlat, uint32_t nlong) :
		  neighbor_ip(nip), neighbor_lat(nlat), neighbor_long(nlong)
		{}

		LocationEntry() {}
		~LocationEntry() {}
	};

	typedef HashMap<IPAddress, RouteEntry> RTable;
	typedef RTable::const_iterator RTIter;


	typedef HashMap<IPAddress, LocationEntry> LocationTable;
	typedef LocationTable::const_iterator LTIter;

	RTable _rtes;
	LocationTable _ltable;
	
	double calculate_metric(RouteEntry r, LocationEntry l);
	
	RouteEntry * choose_bestneighbor(IPAddress _current_dst_addr,HashMap<IPAddress, RouteEntry>  _rtes)
	{


		if(_rtes.findp(_current_dst_addr) != 0)
		return _rtes.findp(_current_dst_addr);
	
		double last_metric = 10000;
		IPAddress best_ip ;
		double current_metric;

		for (RTIter iter = _rtes.begin(); iter.live(); iter++) {
			
			RouteEntry rte = iter.value();
			
			LocationEntry lentry =  _ltable.find(rte.neighbor_ip);

			current_metric = calculate_metric(rte, lentry);
			
			switch(rte.channel)
			{
				case 1:
					if (!can_use_1)
						continue;
				break;
				case 6:
					if (!can_use_6)
						continue;
				break;
				case 11:
					if (!can_use_11)
						continue;
				break;
			}


			if(current_metric < last_metric)
			{
				last_metric = current_metric;
				best_ip = rte.neighbor_ip;
			}	
		}
		channel_used = _rtes.findp(best_ip)->channel;
		return _rtes.findp(best_ip);
	};


	uint8_t channel_used ;
	
};
CLICK_ENDDECLS
#endif
