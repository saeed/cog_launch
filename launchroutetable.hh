#ifndef LAUNCHROUTETABLE_HH
#define LAUNCHROUTETABLE_HH
#include <click/element.hh>
#include <click/ipaddress.hh>
#include <click/etheraddress.hh>
#include <click/vector.hh>
#include "launch.hh"
CLICK_DECLS


class LaunchRouteTable : public Element {

public:
  // generic rt methods
  bool current_gateway(RouteEntry &entry);
  bool get_one_entry(const IPAddress &dest_ip, RouteEntry &entry);
  void get_all_entries(Vector<RouteEntry> &vec);
  unsigned get_number_direct_neigbors();

  LaunchRouteTable();
  ~LaunchRouteTable();

  const char *class_name() const		{ return "LaunchRouteTable"; }
  void *cast(const char *);
  const char *port_count() const		{ return PORTS_1_1; }
  const char *processing() const		{ return "push"; }
  const char *flow_code() const                 { return "x/y"; }

  int configure(Vector<String> &, ErrorHandler *);
  int initialize(ErrorHandler *);

 
  Packet *simple_action(Packet *);





  void insert_route( const IPAddress &nip,uint32_t nlat, uint32_t nlong,const EtherAddress &ne,uint8_t chl, uint32_t pub, uint32_t swt);
  double calculate_metric(RouteEntry r);
  RouteEntry choose_bestneighbor();


private:

double distance(double lat1, double lon1, double lat2, double lon2, char unit);
  double deg2rad(double deg);
  double rad2deg(double deg);
 
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
    { }

    RouteEntry() { }
  };



  
	typedef HashMap<IPAddress, RouteEntry> RTable;
	typedef RTable::const_iterator RTIter;

	RTable _rtes;


 
  


  /* this node's addresses */
  IPAddress _ip;
  EtherAddress _eth;
  
  uint32_t my_lat;			// this node's Latitude.
  uint32_t my_long;			// this node's Longitude.


  /* destination node's addresses */
  IPAddress dst_ip;
  EtherAddress dst_eth;
  
  uint32_t dst_lat;			// this node's Latitude.
  uint32_t dst_long;			// this node's Longitude.












};

CLICK_ENDDECLS
#endif
