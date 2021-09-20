'''
Name: printServer
Desc: basic test script to print messages received by the server
'''

import wheelsonTCP as wTCP
import time
import sys

server_ip = "192.168.0.101"
port = 1000
RX_SIZE = 64
TX_SIZE = 64


server = wTCP.wheelsonTCP(server_ip,port,UDPPACKETSIZE)
if server.socket is None:
    print("FAILED TO CONNECT TO SERVER")
    sys.exit()

while True:
    print("Test")
    data = server.receive()
    print(data) 
    
    #send an ack as a heart beat
    buffer = bytearray(b'\x37\x37\x37\x37')
    server.send(buffer)
