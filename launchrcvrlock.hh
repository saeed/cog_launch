#ifndef LAUNCH_RCVR_LOCK_HH
#define LAUNCH_RCVR_LOCK_HH
#include <click/element.hh>
#include <click/glue.hh>
#include <click/ipaddress.hh>
#include <click/etheraddress.hh>
#include <clicknet/ether.h>
#include <elements/local/launch.hh>
#include <click/confparse.hh>
CLICK_DECLS

class LaunchRcvrLock : public Element { public:

	LaunchRcvrLock();
	~LaunchRcvrLock();

	const char *class_name() const		{ return "LaunchRcvrLock"; }
	const char *port_count() const		{ return PORTS_1_1; }
	const char *processing() const		{ return AGNOSTIC; }
	
	int configure(Vector<String> &, ErrorHandler *);
	void run_timer(Timer *);

	Packet *simple_action(Packet *);

	private:

	String _ifname;
	uint8_t _locked_channel;
	uint8_t _lock_count;
	uint32_t _lock_timeout_ms;
	Timer _lock_timeout_timer;

	uint32_t _pu_behavior0;
	uint32_t _pu_behavior1;
	uint32_t _pu_behavior2;
	

};
CLICK_ENDDECLS
#endif
