#ifndef LAUNCHLOCKRESPONDER_HH
#define LAUNCHLOCKRESPONDER_HH
#include <click/element.hh>
#include <click/glue.hh>
#include <click/ipaddress.hh>
#include <click/etheraddress.hh>
#include <clicknet/ether.h>
#include <elements/local/launch.hh>
#include <click/confparse.hh>
#include <click/timer.hh>
#include <elements/local/launchrouter.hh>
#include <elements/userlevel/fromdevice.hh>
CLICK_DECLS

class LaunchLockResponder : public Element { public:

	LaunchLockResponder();
	~LaunchLockResponder();

	const char *class_name() const		{ return "LaunchLockResponder"; }
	const char *port_count() const		{ return PORTS_1_1; }
	const char *processing() const		{ return AGNOSTIC; }
	void add_handlers();
	int configure(Vector<String> &, ErrorHandler *);
	void run_timer(Timer *);

	Packet *simple_action(Packet *);
	
	LaunchRouter * _router;

	EtherAddress _my_eth;

	private:
//What is this???
	String _ifname;
//Channel locked 	
	uint8_t _locked_channel;
//Lock counter	
	uint8_t _lock_count;
//channel will be locked for time = _loc_timeout_ms in millisecond	
	uint32_t _lock_timeout_ms;
//Timer of the lock	
	Timer _lock_timeout_timer;
	FromDevice * _from_dev;

//PU behavior of the three candidates channels
	double _pu_behavior0;
	double _pu_behavior1;
	double _pu_behavior2;

//This node's ip address	
	IPAddress _ip;
// PU Handler
	static int pu_sensed(const String &s, Element *e, void *, ErrorHandler *errh);
	

};
CLICK_ENDDECLS
#endif
