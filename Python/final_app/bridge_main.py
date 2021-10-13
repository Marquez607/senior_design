'''
Name: bridge_main
Author: Marquez Jones
Desc: this is the main entrance for the bridge system which
       this primarily holds all the flask code that deals with
       video handling

       this code will fork the main page which is written using
       pywebio; the reason for starting in the flask app is 
       because flask simply won't work when forked using the 
       multiprocess library or being a subprocess but pywebio will

       this will also setup the tcp connection for video feed from
       InvestiGator/Wheelson

based on
https://blog.miguelgrinberg.com/post/video-streaming-with-flask
'''

#!/usr/bin/env python
from flask import Flask, render_template, Response
import time
import os
import multiprocessing as mp
import argparse
import jpeg_client as jCli
import webStream as bridge
import atexit

folder = "./rx_images"

app = Flask(__name__)
app.debug=False

# IPC FOR GETTING IMAGE DATA
camPipe = mp.Queue()

#PROCESSES 
jpegProc = None   #jpeg tcp connection
bridgeProc = None #bridge webpage 

def gen():
    '''
    Receive and post jpegs to the page
    '''
    global camPipe
    while True:
        t0 = time.time()
        frame = bytearray(camPipe.get())
        yield (b'--frame\r\n'
               b'Content-Type: image/jpeg\r\n\r\n' + frame + b'\r\n')
        # time.sleep(0.05) #limite to 20 FPS
        t1 = time.time()
        print(f"FPS = {1/(t1-t0)}")
                
@app.route('/video_feed')
def video_feed():
    return Response(gen(),
                    mimetype='multipart/x-mixed-replace; boundary=frame')

def parse_args():
    '''
    if using as script, you can pass in the ip and port via these flags
    '''
    parser = argparse.ArgumentParser(description='jpeg receiving over socket')

    #IP FOR INVESTIGATOR/WHEELSON                
    parser.add_argument('-i','--ip',required=True,dest='ip',
                        help='IP address of jpeg server')
    
    #PORTS FOR INVESTIGATOR SERVICES
    parser.add_argument('-v','--vport',required=False,dest='port', type=int,default=1000,
                        help='port number of jpeg server on robot')
                       
    parser.add_argument('-c','--cport',required=False,dest='port', type=int,default=1001,
                        help='port number of command server on robot')   

    parser.add_argument('-u','--uport',required=False,dest='port', type=int,default=1002,
                        help='port number of update server on robot')
                        
    #PORTS FOR WEBSERVER
    parser.add_argument('--ui',dest='bridge_port',type=int,default=4040,
                        help='port for main web UI')

    parser.add_argument('--video',dest='vid_port',type=int,default=4041,
                        help='port for video streaming server')   

    args = parser.parse_args()

    return args.ip,args.port,args.bridge_port,args.vid_port

def exit_handler():
    print("exit_handler : killing all processes")
    if jpegProc is not None: 
        jpegProc.terminate()

    if bridgeProc is not None: 
        bridgeProc.terminate()

def main():

    global camPipe
    global jpegProc
    global bridgeProc

    atexit.register(exit_handler)

    server_ip,server_port,bridge_port,vid_port = parse_args()

    # #need to fork bridge page here, bridge will handle other Investagor tcp connections
    bridgeProc = mp.Process(target=bridge.main,args=(bridge_port,))
    bridgeProc.start()

    #start jpeg client for Investigator 
    jpegProc = mp.Process(target=jCli.jpeg_client,args=(server_ip,server_port,camPipe))
    jpegProc.start()

    app.run(host='127.0.0.1', port=vid_port,threaded=True,debug=False,use_reloader=False)

if __name__ == '__main__':
    # start jpeg client 
    main()