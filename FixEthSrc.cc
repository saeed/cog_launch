#include <click/config.h>
#include <elements/local/launch.hh>
#include <elements/local/FixEthSrc.hh>
#include <click/args.hh>
#include <clicknet/ether.h>
#include <click/error.hh>
#include <click/glue.hh>
#include <click/confparse.hh>
CLICK_DECLS

FixEthSrc::FixEthSrc()
{
}

FixEthSrc::~FixEthSrc()
{
}

int
FixEthSrc::configure(Vector<String> &conf, ErrorHandler * errh)
{
	if (Args(conf, this, errh)
      .read_mp("ETH", _eth)
      .complete() < 0)
      return -1;
	
	return 0;
}




Packet *
FixEthSrc::simple_action(Packet *p_)
{
 
  WritablePacket * p_in = p_->uniqueify();
  
  click_ether *ethh = p_in->ether_header();
  memcpy(ethh->ether_shost, _eth.data(), 6);
		  
  return p_in;
}

CLICK_ENDDECLS
EXPORT_ELEMENT(FixEthSrc)

