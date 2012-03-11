
#include <click/config.h>
#include <stddef.h>
#include <click/args.hh>
#include <click/error.hh>
#include <clicknet/ether.h>
#include <clicknet/ip.h>
#include <click/standard/scheduleinfo.hh>
#include <click/router.hh>
#include <click/element.hh>
#include <click/glue.hh>
#include <click/straccum.hh>
#include <click/packet_anno.hh>
#include <elements/local/launchroutetable.hh>

#include <math.h>
#define pi 3.14159265358979323846


void
LaunchRouteTable::insert_route(const IPAddress &nip,
	      uint32_t nlat, uint32_t nlong, 
	       const EtherAddress &ne, 	uint8_t chl,
	       	uint32_t pub, 	uint32_t swt)
{
  
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



RouteEntry choose_bestneighbor()
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

double calculate_metric(RouteEntry r)
{


	uint8_t distance = distance(((double)dst_lat/1000.00),((double)dst_long/1000.00),((double)r.neighbor_lat/1000.00), ((double)r.neighbor_long/1000.00),'M');

	return ((distance/(3*(10^6)))  + r.switching_time)/(1- r.pu_behavior*_pu_behavior);
}



double distance(double lat1, double lon1, double lat2, double lon2, char unit) {
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


double deg2rad(double deg) {
  return (deg * pi / 180);
}


double rad2deg(double rad) {
  return (rad * 180 / pi);
}


EXPORT_ELEMENT(LaunchRouteTable)
CLICK_ENDDECLS
