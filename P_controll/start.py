#!/usr/bin/python

import socket
import time
from time import gmtime, strftime
from datetime import datetime
import sys, getopt
import requests # pip install requests  /// https://stackoverflow.com/questions/4476373/simple-url-get-post-function-in-python

def main(argv):
	bow = ''
	person = ''
	lat=''
	lon=''
	try:
	  opts, args = getopt.getopt(argv,"hb:p:a:o:",["bow=","person=","lat=","lon="])
	except getopt.GetoptError:
	  print 'start.py  -b <bowid> -p <personid> -a <latitude decimal> -o <longtude decimal>'
	  sys.exit(2)
	for opt, arg in opts:
	  if opt == '-h':
	     print 'start.py  -b <bowid> -p <personid> -a <latitude decimal> -o <longtude decimal>'
	     sys.exit()
	  elif opt in ("-b", "--bow"):
	     bow = arg
	  elif opt in ("-p", "--person"):
	     person = arg
	  elif opt in ("-a","--lat"):
	     lat = arg
	  elif opt in ("-o","--lon"):
	     lon = arg

	t=strftime("%Y-%m-%d %H:%M:%S", gmtime())

	# Post header to database - response the headerb id if succeed
	pstmessage=t+","+bow+","+person+","+lat+","+lon
	print 'Post message :', pstmessage
	url = 'http://bergulix.dyndns.org:8099/m_head.php'
	payload = {'data': pstmessage}

	r = requests.post(url, data=payload)
	print 'response: ',r.text[0:2]
	print 'status: ',r.status_code

	if r.status_code==200 and r.text[0:2]=="OK":

		udpmessage = b"Start    "+chr(0)+t+chr(0)+" "+r.text[3:99]+chr(0)
		print 'UDP  message :', udpmessage+"@@@"
		# Init udp
		server = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
		server.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
		server.settimeout(0.2)
		server.bind(("", 37667))
		server.sendto(udpmessage, ('192.168.9.255', 37666))
		time.sleep(1)


if __name__ == "__main__":
   main(sys.argv[1:])


