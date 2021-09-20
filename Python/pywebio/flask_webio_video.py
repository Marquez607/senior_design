from pywebio.platform.flask import webio_view
from flask import Flask
from pywebio import start_server
from pywebio.output import *
from pywebio.input import *

import time
import os

app = Flask(__name__)


folder = "../../archive/DATASET/DATASET/TEST/R"
fps = 'FPS'
stream = "STREAM"

def main():
    for filename in os.listdir(folder):
        if ".jpg" in filename:
            img = open(os.path.join(folder,filename), 'rb').read() 
            with use_scope(stream, clear=True): 
                put_image(img)
            time.sleep(1)


# `task_func` is PyWebIO task function
app.add_url_rule('/', 'webio_view', webio_view(main))  # need GET,POST and OPTIONS methods

app.run(host='localhost', port=4040,threaded=True)