import subprocess
import time

vlan_count=input("please input vlan count:\n")
if_name=input("please input if name:\n")
start_port=61002
for i in range(2,int(vlan_count)+2):
    if i <= 254:
        ip_set = "192.166.0." + str(i)
    else:
        if (i > 254) and (i <= 508):
            ip_set = "192.166.1." + str(i - 254)
        else:
            ip_set = "192.166.2." + str(i - 508)
    subprocess.call('netsh interface ipv4 add address '+if_name+' address='+ip_set+' mask=255.255.0.0', shell=True)
    #按需更改
    subprocess.Popen("D:\\UserData\\Admindesktop\\iperf-3.1.3-win64\\iperf3 -s "+ip_set+' -D -p '+str(start_port)+' -i 20',shell=True)
    time.sleep(0.5)
    start_port=start_port+1



