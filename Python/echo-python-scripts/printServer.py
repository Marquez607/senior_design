'''
Name: printServer
Desc: basic test script to print messages received by the server
'''

import wheelsonTCP as wTCP
import time
import sys

server_ip = "192.168.0.102"
port = 1000
RX_SIZE = 2048
TX_SIZE = 64


server = wTCP.wheelsonTCP(server_ip,port,RX_SIZE)
if server.socket is None:
    print("FAILED TO CONNECT TO SERVER")
    sys.exit()

while True:
    print("Test")
    data = bytearray(server.receive())
    #str = data.decode('utf-8')
    print(data) 
    
    #send an ack as a heart beat
    #buffer = bytearray(b'\x37\x37\x37\x37')
    #server.send(buffer)
