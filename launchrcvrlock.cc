#include <click/config.h>
#include <elements/local/launch.hh>
#include <elements/local/launchrcvrlock.hh>
#include <click/args.hh>
#include <clicknet/ether.h>
#include <click/error.hh>
#include <click/glue.hh>
#include <click/confparse.hh>
CLICK_DECLS


LaunchCtrlResponder::LaunchCtrlResponder()
: timer(this)
{
}

LaunchCtrlResponder::~LaunchCtrlResponder()
{
}

int
LaunchCtrlResponder::configure(Vector<String> &conf, ErrorHandler * errh)
{
	if (Args(conf, this, errh)
      .read_mp("DEVNAME", _ifname)
      .read_mp("WAIT", _lock_timeout_ms)
	  .read_mp("CH0", _pu_behavior0)
	  .read_mp("CH1", _pu_behavior1)
	  .read_mp("CH2", _pu_behavior2)
	  .complete() < 0)
      return -1;


	_lock_timeout_timer.schedule_after_ms(_lock_timeout_ms);
	return 0;
}



Packet *
LaunchCtrlResponder::simple_action(Packet *p_in)
{
	WritablePacket *p = p_in->uniqueify();
	struct launch_ctrl_hdr * lauch_hdr_ptr = (grid_hdr *) (p->data());
	if((_lock_count > 0 && lauch_hdr_ptr->channel == _locked_channel)||_lock_count)
	{
		_locked_channel = lauch_hdr_ptr->channel;
		lauch_hdr_ptr->type = launch_ctrl_hdr::LAUNCH_LOCK_RES;
		lauch_hdr_ptr->lock_response = 1;
		_lock_count++;
		return p;
	}
	lauch_hdr_ptr->type = launch_ctrl_hdr::LAUNCH_LOCK_RES;
	lauch_hdr_ptr->lock_response = 0;

	switch(_locked_channel)
	{
		case 1:
			lauch_hdr_ptr->channel = 1;
			lauch_hdr_ptr->pu_behavior = _pu_behavior0;
			break;
		case 6:
			lauch_hdr_ptr->channel = 6;
			lauch_hdr_ptr->pu_behavior = _pu_behavior1;
			break;
		case 11:
			lauch_hdr_ptr->channel = 11;
			lauch_hdr_ptr->pu_behavior = _pu_behavior2;
			break; 
	}

	return p;

}


void
run_timer(Timer * t)
{
	if(_lock_count > 0)
	{
		_lock_count--;
	}
	_lock_timeout_timer.schedule_after_ms(_lock_timeout_ms);
}

CLICK_ENDDECLS
EXPORT_ELEMENT(LaunchCtrlResponder)

