#ifndef LAUNCH_RCVR_LOCK_HH
#define LAUNCH_RCVR_LOCK_HH
#include <click/element.hh>
#include <click/glue.hh>
#include <click/ipaddress.hh>
#include <click/etheraddress.hh>
#include <clicknet/ether.h>
#include <elements/local/launch.hh>
#include <elements/local/launchctrlrequester.hh>
#include <elements/local/launchlockrequester.hh>
#include <click/confparse.hh>
CLICK_DECLS

class LaunchRouter : public Element { public:

	LaunchRouter();
	~LaunchRouter();

	const char *class_name() const		{ return "LaunchRouter"; }
	const char *port_count() const		{ return PORTS_1_1; }
	const char *processing() const		{ return "h/h"; }
	
	int configure(Vector<String> &, ErrorHandler *);
	int initialize(ErrorHandler *);
	
	Packet *simple_action(Packet *);

	void insert_route( const IPAddress &nip,uint32_t nlat, uint32_t nlong,const EtherAddress &ne,uint8_t chl, uint32_t pub, uint32_t swt);
	double calculate_metric(RouteEntry r);
	RouteEntry choose_bestneighbor();
	
	void set_channel_loc_positive();



private:
	WritablePacket *p _holded_packet;

	IPAddress _ip;
	EtherAddress _eth;

	uint32_t _pu_behavior;

	bool _channel_lock_positive;
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

		class EtherAddress   neighbor_eth; // hardware address of next hop

		uint8_t channel;
		uint32_t pu_behavior;
		uint32_t switching_time;



		RouteEntry(const IPAddress &nip,
			  uint32_t nlat, uint32_t nlong, 
			   const EtherAddress &ne, 	uint8_t chl,
			   	uint32_t pub, 	uint32_t swt) :
		  neighbor_ip(nip), neighbor_lat(nlat), neighbor_long(nlong), 
		  neighbor_eth(ne), channel(chl), switching_time(swt)
		{}

		RouteEntry() {}
		~RouteEntry() {}
	};

	typedef HashMap<IPAddress, RouteEntry> RTable;
	typedef RTable::const_iterator RTIter;

	RTable _rtes;
};
CLICK_ENDDECLS
#endif
