#ifndef FIX_ETH_SRC_HH
#define FIX_ETH_SRC_HH
#include <click/element.hh>
#include <click/glue.hh>
#include <click/ipaddress.hh>
#include <click/etheraddress.hh>
#include <clicknet/ether.h>
#include <elements/local/launch.hh>
#include <click/confparse.hh>

CLICK_DECLS

class FixEthSrc : public Element { public:

	FixEthSrc();
	~FixEthSrc();

	const char *class_name() const		{ return "FixEthSrc"; }
	const char *port_count() const		{ return PORTS_1_1; }
	const char *processing() const		{ return AGNOSTIC; }
	
	int configure(Vector<String> &, ErrorHandler *);


	Packet *simple_action(Packet *);

	private:

	EtherAddress _eth;
		
};
CLICK_ENDDECLS
#endif
