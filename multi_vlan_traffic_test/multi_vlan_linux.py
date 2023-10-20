import subprocess
from scapy.all import *
import time
import threading
def get_info():
    global eth_name
    global vlan_count
    global ip_set_num
    global vlan_mac_num
    global remote_ip_set_num
    global iperf_setting
    global dut_lan_mac
    global arp_pk2_num
    global arp_pk1_num
    arp_pk2_num=[]
    arp_pk1_num=[]
    ip_set_num=[]
    vlan_mac_num=[]
    remote_ip_set_num=[]
    eth_name=input("###############################please input 【eth_name】:\n")
    vlan_count=input("###############################please input 【vlan count】:\n")
    dut_lan_mac=input("###############################will add linux pc route and arp list please input 【gateway lan mac 】\n")
    iperf_setting=input("###############################please input 【iperf setting】\nsuch as:\nsudo iperf3 -t 10000 \n")
    for i in range(2,int(vlan_count)+2):
        vlan_id = i
        vlan_name = str(eth_name) +"." + str(vlan_id)
        vlan_if_mac=str(i)
        #vlan_mac
        for k in range(12-len(str(i))):
            vlan_if_mac = "0"+vlan_if_mac
        vlan_if_MAC=vlan_if_mac[0:2]+":" +vlan_if_mac[2:4]+":" +vlan_if_mac[4:6]+":" +vlan_if_mac[6:8]+":" +vlan_if_mac[8:10]+":" +vlan_if_mac[10:12]
        vlan_mac_num.append(vlan_if_MAC)
        if i<=254:
            ip_set="192.167.0."+str(i)
        else:
            if (i>254)and(i<=508):
                ip_set = "192.167.1." + str(i-254)
            else:
                ip_set = "192.167.2." + str(i - 508)
        ip_set_num.append(ip_set)
        remote_ip_set="192.166."+ip_set[8:]
        remote_ip_set_num.append(remote_ip_set)
        arp_pk1=(Ether(dst="ff:ff:ff:ff:ff:ff",src=vlan_if_MAC)/Dot1Q(vlan=i)/ARP(op=1,psrc=ip_set,hwsrc=vlan_if_MAC,pdst="192.167.0.1",hwdst=dut_lan_mac))
        arp_pk2=(Ether(dst=dut_lan_mac,src=vlan_if_MAC)/Dot1Q(vlan=i)/ARP(op=2,psrc=ip_set,hwsrc=vlan_if_MAC,pdst="192.167.0.1",hwdst=dut_lan_mac))
        arp_pk1_num.append(arp_pk1)
        arp_pk2_num.append(arp_pk2)
    
    
def vlan_init():
    for i in range(2,int(vlan_count)+2):
        vlan_name=eth_name+"."+str(i)
        subprocess.check_output("sudo vconfig add " + str(eth_name) + " "+str(i), shell=True)
        subprocess.check_output("sudo ifconfig " + vlan_name + " hw ether " + vlan_mac_num[i-2], shell=True)
        subprocess.check_output("sudo ifconfig " + vlan_name + " up", shell=True)
        subprocess.check_output("sudo ifconfig " + vlan_name +" " +ip_set_num[i-2] + " netmask 255.255.0.0" , shell=True)
        subprocess.check_output("sudo ip route add "+remote_ip_set_num[i-2]+"/32 dev " + vlan_name , shell=True)
        subprocess.check_output("sudo arp -i "+vlan_name+" -s "+remote_ip_set_num[i-2]+" "+dut_lan_mac, shell=True)
        
def iperf_run():
    start_port=61000
    for i in range(2,int(vlan_count)+2):
        subprocess.Popen(iperf_setting+" -c "+remote_ip_set_num[i-2]+" -B "+ip_set_num[i-2]+" -p "+str(start_port+2), shell=True)
        start_port=start_port+1
         
def arp_update():
    for item in arp_pk1_num:
        sendp(item)
    while(1):
        for item in arp_pk2_num:
            sendp(item)
        time.sleep(18)


if __name__ == '__main__':
    print("【Before doing test ，please read readme】")
    get_info()
    vlan_init()
   
    thread1=threading.Thread(target=arp_update,name="thread1")
    thread2=threading.Thread(target=iperf_run,name="thread2")
    thread1.start()
    time.sleep(10)
    thread2.start()


    
 
