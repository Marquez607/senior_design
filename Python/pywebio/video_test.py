
import pywebio 
from pywebio import start_server
from pywebio.output import *
from pywebio.input import *
from pywebio.session import *

import time
import os

#iterate through folder and display contents on website forever
folder = "../../archive/DATASET/DATASET/TEST/R"
fps = 'FPS'
stream = "STREAM"

def main():
    set_env(output_animation=False)
    for filename in os.listdir(folder):
        if ".jpg" in filename:
            img = open(os.path.join(folder,filename), 'rb').read() 
            with use_scope(stream, clear=True): 
                put_image(img)
            time.sleep(0.1)

if __name__ == '__main__':
    # pywebio.session.set_env(output_animation=False)  
    # while True:
    #     main()
    start_server(main, debug=True, port=4040,output_animation=False)     