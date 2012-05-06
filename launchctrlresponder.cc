#include <click/config.h>
#include <elements/local/launch.hh>
#include <elements/local/launchctrlresponder.hh>
#include <click/args.hh>
#include <clicknet/ether.h>
#include <click/error.hh>
#include <click/glue.hh>
#include <click/confparse.hh>
CLICK_DECLS


LaunchCtrlResponder::LaunchCtrlResponder()
{
}

LaunchCtrlResponder::~LaunchCtrlResponder()
{
}

int
LaunchCtrlResponder::configure(Vector<String> &conf, ErrorHandler * errh)
{

 	if (Args(conf, this, errh)
 .read_mp("IP", lch.neighbor_ip)
 .read_mp("LONG", lch.my_long)
      .read_mp("LAT", lch.my_lat)
	  .read_mp("TSWTCH", lch.switching_time)
	  .read_mp("CH0", _pu_behavior0)
	  .read_mp("CH1", _pu_behavior1)
	  .read_mp("CH2", _pu_behavior2)
      .complete() < 0)
      return -1;
	lch.type =  launch_ctrl_hdr::LAUNCH_RES;
	//A node responds with the best channel from three candidates 
	//in terms of PU behaviour(1/(probability of the presence of PU))

	if (_pu_behavior0 >= _pu_behavior1 && _pu_behavior0 >= _pu_behavior2)
	{
		lch.channel=1;
		lch.pu_behavior = _pu_behavior0;
	}
	else if (_pu_behavior1 >= _pu_behavior0 && _pu_behavior1 >= _pu_behavior2)
	{
		lch.channel=6;
		lch.pu_behavior = _pu_behavior1;
	}
	else if (_pu_behavior2 >= _pu_behavior0 && _pu_behavior2 >= _pu_behavior1)
	{
		lch.channel=11;
		lch.pu_behavior = _pu_behavior2;
	}
	return 0;
}




Packet *
LaunchCtrlResponder::simple_action(Packet *p_in)
{
  int extra = sizeof(lch);
  WritablePacket *p = p_in->push(extra);
  if (!p)
    return 0;

  memcpy(p->data(), &lch, sizeof(lch));

  return p;
}

CLICK_ENDDECLS
EXPORT_ELEMENT(LaunchCtrlResponder)

