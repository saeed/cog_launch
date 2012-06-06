#ifndef LAUNCH_CTRL_RESPONDER_HH
#define LAUNCH_CTRL_RESPONDER_HH
#include <click/element.hh>
#include <click/glue.hh>
#include <click/ipaddress.hh>
#include <click/etheraddress.hh>
#include <clicknet/ether.h>
#include <elements/local/launch.hh>
#include <click/confparse.hh>
CLICK_DECLS

class LaunchCtrlResponder : public Element { public:

	LaunchCtrlResponder();
	~LaunchCtrlResponder();

	const char *class_name() const		{ return "LaunchCtrlResponder"; }
	const char *port_count() const		{ return PORTS_1_1; }
	const char *processing() const		{ return AGNOSTIC; }
	
	int configure(Vector<String> &, ErrorHandler *);


	Packet *simple_action(Packet *);

	private:

	uint32_t _my_lat;			// Sender's Latitude.
        
    uint32_t _my_long;			// Sender's Longitude.
	
	// PU behavior of the three candidates channels
	double _pu_behavior0;
	double _pu_behavior1;
	double _pu_behavior2;

	EtherAddress _my_eth;	
	uint32_t switching_time;
	
	//The header of Launch control packet
	struct launch_ctrl_hdr lch;
};
CLICK_ENDDECLS
#endif
