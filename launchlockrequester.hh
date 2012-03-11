#ifndef CLICK_LAUNCHREQUESTER_HH
#define CLICK_LAUNCHREQUESTER_HH
#include <click/element.hh>
#include <click/glue.hh>
#include <elements/launch/launch.hh>
CLICK_DECLS

/*
 * =c
 *
 * launchrequester()
 *
 */

class LAUNCHLockRequester : public Element { public:

  LAUNCHLockRequester();
  ~LAUNCHLockRequester();

  const char *class_name() const		{ return "LAUNCHLockRequester"; }
  const char *port_count() const		{ return "0/1"; }
  const char *processing() const		{ return PUSH; }

  int configure(Vector<String> &, ErrorHandler *);
  bool can_live_reconfigure() const		{ return true; }
  int initialize(ErrorHandler *);

  //Packet *simple_action(Packet *);
  void send_request();
  

 private:

	launch_ctrl_hdr _lh;

};

CLICK_ENDDECLS
#endif
