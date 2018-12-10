import socket
import time
from time import gmtime, strftime
from datetime import datetime

server = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
server.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
# Set a timeout so the socket does not block
# indefinitely when trying to receive data.
server.settimeout(0.2)
server.bind(("", 37667))
#t=datetime.now()


t=strftime("%Y-%m-%d %H:%M:%S", gmtime())
message = b"Start    "+chr(0)+t+chr(0)+"abcdef"
#server.sendto(message, ('<broadcast>', 37666))
server.sendto(message, ('192.168.9.255', 37666))
print("message sent: %s"%message)
time.sleep(1)