#!/usr/bin/python

#Ver: 2019-01-09 21:30

import socket
import time
from time import gmtime, strftime
from datetime import datetime
import sys, getopt
import requests # pip install requests  /// https://stackoverflow.com/questions/4476373/simple-url-get-post-function-in-python

url = 'http://bergulix.dyndns.org:8100/bow/web/m_head.php'

def main(argv):
	bow = ''
	person = ''
	lat=''
	lon=''
	try:
	  opts, args = getopt.getopt(argv,"hb:p:a:o:",["bow=","person=","lat=","lon="])
	except getopt.GetoptError:
	  print ('start.py  -b <bowid> -p <personid> -a <latitude decimal> -o <longtude decimal>')
	  sys.exit(2)
	optNo=0
	for opt, arg in opts:
	  if opt == '-h':
	     print ('start.py  -b <bowid> -p <personid> -a <latitude decimal> -o <longtude decimal>')
	     sys.exit()
	  elif opt in ("-b", "--bow"):
	     bow = arg
	     optNo += 1
	  elif opt in ("-p", "--person"):
	     person = arg
	     optNo += 1
	  elif opt in ("-a","--lat"):
	     lat = arg
	     optNo += 1
	  elif opt in ("-o","--lon"):
	     lon = arg
	     optNo += 1
	
	t=strftime("%Y-%m-%d %H:%M:%S", gmtime())

	if optNo==4:
		print ("Start connection")
		s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
		print ("Connect 8.8.8.8")
		s.connect(("8.8.8.8", 80))
		print ("Get answer")
		ip = s.getsockname()[0].split('.')
		print ("Calculate broadcast address")
		broadcastip=ip[0]+"."+ip[1]+"."+ip[2]+".255"
		print ("Broadcast address:",broadcastip)
		s.close()

		# Post header to database - response the headerb id if succeed
		pstmessage=t+","+bow+","+person+","+lat+","+lon
		print ('Post message :', pstmessage)
		payload = {'data': pstmessage}

		r = requests.post(url, data=payload)
		print ('response: ',r.text)
		print ('status: ',r.status_code)

		if r.status_code==200 and r.text[0:2]=="OK":
			udpmessage_str = "Start    "+chr(0)+t+chr(0)+" "+r.text[3:99]+chr(0)
			udpmessage=udpmessage_str.encode('utf-8')
			print ('UDP  message :', udpmessage+b"@@@")
			# Init udp
			server = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
			server.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
			server.settimeout(0.2)
			server.bind(("", 37667))
			server.sendto(udpmessage, (broadcastip, 37666))
			time.sleep(1)
	else:
		print ('Hianyzo parameter. Hasznalata:')
		print ('start.py  -b <bowid> -p <personid> -a <latitude decimal> -o <longtude decimal>')

if __name__ == "__main__":
   main(sys.argv[1:])


