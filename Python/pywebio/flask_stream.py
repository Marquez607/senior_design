'''
https://blog.miguelgrinberg.com/post/video-streaming-with-flask
'''

#!/usr/bin/env python
from flask import Flask, render_template, Response
import time
import os

folder = "../../archive/DATASET/DATASET/TEST/R"

app = Flask(__name__)

@app.route('/video')
def index():
    return render_template('./index.html')

def gen():
    while True:
        for filename in os.listdir(folder):
            if ".jpg" in filename:
                frame = open(os.path.join(folder,filename), 'rb').read() 
                time.sleep(0.1)
                yield (b'--frame\r\n'
                    b'Content-Type: image/jpeg\r\n\r\n' + frame + b'\r\n')
                
@app.route('/video_feed')
def video_feed():
    return Response(gen(),
                    mimetype='multipart/x-mixed-replace; boundary=frame')

if __name__ == '__main__':
    app.run(host='localhost', port=4040,threaded=True)