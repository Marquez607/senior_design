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

def cmd_process(ip,port,in_fifo):
    pass 

def update_process(ip,port,out_fifo):
    pass

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
    
    #fork update process
    cmd_proc = mp.Process(target=cmd_process,args=(ip,cport,None))
    cmd_proc.start()

    #fork command process
    update_proc = mp.Process(target=update_process,args=(ip,uport,None))
    update_proc.start()