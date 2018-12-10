import socket
import time
from time import gmtime, strftime
from datetime import datetime

client = socket.socket(socket.AF_INET, socket.SOCK_DGRAM) # UDP
client.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
client.bind(("", 37666))
while True:
    data, addr = client.recvfrom(1024)
    t=strftime("%Y-%m-%d %H:%M:%S", gmtime())
    print("%s  -- %s %s"%(t,addr,data))