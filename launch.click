Twlan0 :: ToDevice(wlan0);
Fwlan0 :: FromDevice(wlan0);
Twlan1 :: ToDevice(wlan1);
Fwlan1 :: FromDevice(wlan1);
Teth0 :: ToDevice(eth0);
Feth0 :: FromDevice(eth0);

CtrlRequester :: LaunchCtrlRequester();
LockRequester :: LaunchLockRequester();

RoutingQueue :: Queue;
eth0Queue :: Queue;

tap :: KernelTap(1.0.0.1/8, ETHER 1:1:1:1:1:1);

Fwlan1 -> c1 :: Classifier(12/0800, 12/0806);

//handle ARP
c1[1] -> ARPResponder(10.0.0.1 00:27:10:a0:f6:b4) -> Twlan0;

c1[0] -> RoutingQueue;  
tap -> RoutingQueue;

RoutingQueue
	-> GetIPAddress(30) 
	-> LR:: LaunchRouter(0.1, 50, 50, 2, CtrlRequester, LockRequester) 
	-> ARPQuerier(10.0.0.1 00:27:10:a0:f6:b4)
	-> Twlan0;

LCR :: LaunchCtrlResponder(0,0,1,0.1,0.2,0.3);
LLR :: LaunchLockResponder(wlan0, 50, 0.1, 0.2,0.3);

Feth0 -> c2 :: Classifier(14/0100%ff00, 14/0200%ff00, 14/0300%ff00, 14/0400%ff00, 14/0500%ff00, -);

c2[0] -> LCR -> eth0Queue;

c2[1] -> 

c2[2] -> 

c2[3] -> LLR -> eth0Queue;

c2[4] -> 

CtrlRequester -> EtherEncap(0x0700, 1:1:1:1:1:1, 2:2:2:2:2:2) -> eth0Queue;
LockRequester -> EtherEncap(0x0700, 1:1:1:1:1:1, 2:2:2:2:2:2) -> eth0Queue;

eth0Queue -> ARPQuerier(11.0.0.1 5c:26:0a:03:f2:f6) -> Teth0;
// what's missing 
// 1. Lock Respoder doesn't switch the channel (done)
// 2. Router doesn't handle multiple destinations (done)
// 3. Router doesn't handle if next hop is the destination (done)
// 4. Update responder is not implemented
// 5. Router doesn't have destintation location
// 6.
