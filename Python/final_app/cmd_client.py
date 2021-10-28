'''
Name: cmd_client.py
Author: Marquez Jones
Desc: TCP client threads/processes for sending 
      commands to wheelson and receiving position updates 

'''

import wheelsonTCP as wTCP
import time
import sys
import argparse
import time
import os
import multiprocessing as mp
import atexit

PDU_START_BYTE0 = 0x5E
PDU_START_BYTE1 = 0xFF
PDU_SIZE = 36 #36 bytes without 2 header bytes

class pdu():
    def __init__(self):
        self.MAX_MSG_LEN = 32
        self.cmd = None 
        self.x = None
        self.y = None 
        self.msg_len = None
        self.msg = None

        #SUPPORTED COMMANDS
        self.MOVE   = 0
        self.STOP   = 1
        self.RESET  = 2
        self.BLOCK  = 3
        self.UPDATE = 4

    
    def get_buff(self,bytes):
        '''
        populate class data from byte array
        '''
        #look for starting bytes
        temp = 0
        temp_last = 0 
        for i in range(len(bytes)):
            temp_last = temp
            temp = bytes[i]
            if temp_last == PDU_START_BYTE0 and temp == PDU_START_BYTE1:
                # print("FOUND START BYTES")
                self.cmd = bytes[i+1]
                self.x = bytes[i+2]
                self.y = bytes[i+3]
                self.msg_len = bytes[i+4]
                self.msg = bytes[i+5:]
                break

    def put_buff(self):
        '''
        fill bytearray with pdu data
        '''
        ret = []
        ret.append(PDU_START_BYTE0)
        ret.append(PDU_START_BYTE1)
        ret.append(self.cmd)
        ret.append(self.x) 
        ret.append(self.y)
        ret.append(self.msg_len)
        for i in range(0,self.msg_len-1):
            ret.append( ord(self.msg[i] ) )
        ret.append(0)
        return bytearray(ret)
        
def wait_for_server(server_ip,port,buffer_size):    
    '''
    Waits for server connection
    '''
    server = wTCP.wheelsonTCP(server_ip,port,buffer_size) #reconnect to server
    while server.socket is None:
        server = wTCP.wheelsonTCP(server_ip,port,buffer_size) #reconnect to server
        if server.socket is None:
            print("FAILED TO CONNECT TO SERVER")
            print("TRYING AGAIN")
            time.sleep(1) #sleep a second    
    print("CONNECTED TO SERVER")
    return server

def cmd_process(ip,port,in_fifo,debug = True):
    tx_pdu = pdu()
    server = wait_for_server(ip,port,PDU_SIZE+2)

    while True:
        try:
            # data = server.receive()
            # tx_pdu.get_buff(data)

            if in_fifo is not None:
                # push to fifo 
                in_fifo.get(tx_pdu)
                data = tx_pdu.put_buff()
                server.send(data)
            
            elif debug: #test pdu
                tx_pdu.cmd = tx_pdu.MOVE 
                tx_pdu.x = 10
                tx_pdu.y = 10
                tx_pdu.msg_len = 9
                tx_pdu.msg = "TEST MSG"
                data = tx_pdu.put_buff()
                server.send(data)  
        except:
            server = wait_for_server(ip,port,PDU_SIZE+2)

def update_process(ip,port,out_fifo):
    '''
    process receives updates from wheelson 
    '''
    rx_pdu = pdu()
    server = wait_for_server(ip,port,PDU_SIZE+2)

    while True:
        try:
            data = server.receive()
            rx_pdu.get_buff(data)

            # print(f"RX CMD : {rx_pdu.cmd}")
            # print(f"RX MSG LEN: {rx_pdu.msg_len}")
            # print(f"RX MSG: {rx_pdu.msg[0:rx_pdu.msg_len]}")

            if out_fifo is not None:
                # push to fifo 
                out_fifo.put(rx_pdu)
        except:
            # print(e)    #server probably dropped
            # print("WAITIN FOR SERVER CONNECTION")
            server = wait_for_server(ip,port,PDU_SIZE+2)           

    

def parse_args():
    '''
    if using as script, you can pass in the ip and port via these flags
    '''
    parser = argparse.ArgumentParser(description='command sending/update receiving over socket')
    parser.add_argument('-c','--cport',required=True,dest='cport', type=int,
                        help='port number of command server')
    parser.add_argument('-u','--uport',required=True,dest='uport', type=int,
                        help='port number of update server')
    parser.add_argument('-i','--ip',required=True,dest='ip',
                        help='IP address of wheelson server')
    args = parser.parse_args()

    return args.ip,args.cport,args.uport


cmd_proc = None 
update_proc = None

def exit_handler():
    print("exit_handler : killing all processes")
    if cmd_proc is not None: 
        cmd_proc.kill()
    if update_proc is not None:
        update_proc.kill()

if __name__ == "__main__": #if running source file directly
    atexit.register(exit_handler)

    ip,cport,uport = parse_args()
    
    #fork command process
    cmd_proc = mp.Process(target=cmd_process,args=(ip,cport,None))
    cmd_proc.start()

    #fork update process
    update_proc = mp.Process(target=update_process,args=(ip,uport,None))
    update_proc.start()
