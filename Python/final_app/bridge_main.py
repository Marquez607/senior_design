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

https://blog.miguelgrinberg.com/post/video-streaming-with-flask
'''

#!/usr/bin/env python
from flask import Flask, render_template, Response
import time
import os
import subprocess
import multiprocessing as mp
import argparse
import jpeg_client as jCli
import atexit

folder = "./rx_images"

app = Flask(__name__)
app.debug=False

# IPC FOR GETTING IMAGE DATA
wheelson_pipe = None
jpegTCP = None

def gen():
    global wheelson_pipe
    while True:
        frame = wheelson_pipe.get()
        yield (b'--frame\r\n'
               b'Content-Type: image/jpeg\r\n\r\n' + frame + b'\r\n')

        # for filename in os.listdir(folder):
        #     if ".jpg" in filename:
        #         frame = open(os.path.join(folder,filename), 'rb').read() 
        #         time.sleep(0.05)
        #         yield (b'--frame\r\n'
        #             b'Content-Type: image/jpeg\r\n\r\n' + frame + b'\r\n')
                
@app.route('/video_feed')
def video_feed():
    return Response(gen(),
                    mimetype='multipart/x-mixed-replace; boundary=frame')

def parse_args():
    '''
    if using as script, you can pass in the ip and port via these flags
    '''
    parser = argparse.ArgumentParser(description='jpeg receiving over socket')
    parser.add_argument('-p','--port',required=True,dest='port', type=int,
                        help='port number of jpeg server')
    parser.add_argument('-i','--ip',required=True,dest='ip',
                        help='IP address of jpeg server')

    parser.add_argument('-w',dest='web_port',type=int,default=4040,
                        help='Port for main webpage')

    parser.add_argument('-v',dest='vid_port',type=int,default=4041,
                        help='port for video streaming webpage')   

    args = parser.parse_args()

    return args.ip,args.port,args.web_port,args.vid_port

def exit_handler():
    print("exit_handler : killing all processes")
    if jpegTCP is not None: 
        jpegTCP.kill()

def main():

    global wheelson_pipe
    global jpegTCP

    atexit.register(exit_handler)

    server_ip,server_port,_,vid_port = parse_args()

    #need to fork bridge page here, bridge will handle other Investagor tcp connections
    bridge = ["python3","webStream.py"] 
    p = subprocess.Popen(bridge)

    #start jpeg client for Investigator 
    jpegTCP = mp.Process(target=jCli.jpeg_client,args=(server_ip,server_port,wheelson_pipe))
    jpegTCP.start()

    app.run(host='localhost', port=vid_port,threaded=True,debug=True,use_reloader=False)

if __name__ == '__main__':
    # start jpeg client 
    main()