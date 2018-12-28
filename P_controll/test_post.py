#!/usr/bin/python

#Ver: 2018-12-11 00:19

import socket
import time
from time import gmtime, strftime
from datetime import datetime
import sys, getopt
import requests # pip install requests  /// https://stackoverflow.com/questions/4476373/simple-url-get-post-function-in-python

url = 'http://bergulix.dyndns.org:8100/bow/web/m_data.php'

def main(argv):
	i=0
	while i<1000: 
		i=i+1
		data="203,2304,16356,-460;204,2384,16324,-588;205,2316,16340,-452;206,2344,16392,-380;207,2392,16368,-260;208,2344,16320,-284;209,2408,16488,-372;210,2392,16348,-392;211,2344,16336,-360;212,2364,16376,-380;"
		m_id="100"
		payload = {'m_id':m_id,'data': data}
		r = requests.post(url, data=payload)
		print 'Count: ',i, '  response: ',r.text, '  status: ',r.status_code
if __name__ == "__main__":
   main(sys.argv[1:])


