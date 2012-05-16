#ifndef CLICK_LAUNCHLOCREQUESTER_HH
#define CLICK_LAUNCHLOCREQUESTER_HH
#include <click/element.hh>
#include <click/glue.hh>
#include <elements/local/launch.hh>
CLICK_DECLS

/*
 * =c
 *
 * launchlocrequester()
 *
 */

class LaunchLockRequester : public Element { public:

  LaunchLockRequester();
  ~LaunchLockRequester();

  const char *class_name() const		{ return "LaunchLockRequester"; }
  const char *port_count() const		{ return "0/1"; }
  const char *processing() const		{ return PUSH; }

  int configure(Vector<String> &, ErrorHandler *);
  bool can_live_reconfigure() const		{ return true; }
  int initialize(ErrorHandler *);

  
  void send_lock_request(uint8_t channel, IPAddress distination_ip,uint8_t * distination_eth, uint8_t * source_eth );


 private:

	launch_ctrl_hdr _lh;

};

CLICK_ENDDECLS
#endif
