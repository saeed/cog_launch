#ifndef LAUNCH_LOCK_RESPONSE_HANDLER_HH
#define LAUNCH_LOCK_RESPONSE_HANDLER_HH
#include <click/element.hh>
#include <click/glue.hh>
#include <click/ipaddress.hh>
#include <click/etheraddress.hh>
#include <clicknet/ether.h>
#include <elements/local/launch.hh>
#include <elements/local/launchrouter.hh>
#include <elements/local/launchlockrequester.hh>
#include <click/confparse.hh>
#include <elements/userlevel/todevice.hh>
CLICK_DECLS

class LaunchLockResponseHandler : public Element { public:

	LaunchLockResponseHandler();
	~LaunchLockResponseHandler();

	const char *class_name() const		{ return "LaunchLockResponseHandler"; }
	const char *port_count() const		{ return PORTS_1_1; }
	const char *processing() const		{ return AGNOSTIC; }
	
	int configure(Vector<String> &, ErrorHandler *);


	Packet *simple_action(Packet *);

	private:

	String _ifname;
	IPAddress _ip;
	
	//Pointer to LaunchRouter to insert response
	LaunchRouter * _router;
	ToDevice * _to_dev;
	LaunchLockRequester * _lock_requester;
	EtherAddress _eth;	
	
};
CLICK_ENDDECLS
#endif
