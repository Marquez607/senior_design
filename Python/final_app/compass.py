'''
Name: compass.py
Author: Marquez Jones
Desc: script for sending compass calibration to wheelson
'''

#!/usr/bin/env python
import time
import os
import multiprocessing as mp
import argparse
import cmd_client as cmdCli
# import webStream as bridge


def send_calibrate(ip,port,N,S,E,W):
    tx_pdu = cmdCli.pdu()
    server = cmdCli.wait_for_server(ip,port,cmdCli.PDU_SIZE+2)

    if True:
        try:
            # data = server.receive()
            # tx_pdu.get_buff(data)
            data = tx_pdu.put_buff_new_heading(N,S,E,W)
            server.send(data)  
            # time.sleep(2)
        except:
            print("SERVER CONN FAILED")
            server = cmdCli.wait_for_server(ip,port,cmdCli.PDU_SIZE+2)

def parse_args():
    '''
    if using as script, you can pass in the ip and port via these flags
    '''
    parser = argparse.ArgumentParser(description='command sending/update receiving over socket')
    parser.add_argument('-c','--cport',required=False,dest='cport', type=int,default=1002,
                        help='port number of command server on robot')   

    parser.add_argument('-i','--ip',required=True,dest='ip',
                        help='IP address of wheelson server')

    parser.add_argument('-N','--North',required=True,dest='N', type=int,
                        help='')

    parser.add_argument('-S','--South',required=True,dest='S', type=int,
                        help='')

    parser.add_argument('-E','--East',required=True,dest='E', type=int,
                        help='')

    parser.add_argument('-W','--West',required=True,dest='W', type=int,
                        help='')

    args = parser.parse_args()

    return args


if __name__ == "__main__": #if running source file directly

    args = parse_args()
    send_calibrate(args.ip,args.cport,args.N,args.S,args.E,args.W)
    
    # #fork command process
    # cmd_proc = mp.Process(target=cmd_process,args=(args.ip,args.cport,None,True))
    # cmd_proc.start()

    # #fork update process
    # update_proc = mp.Process(target=update_process,args=(ip,uport,None,True))
    # update_proc.start()
