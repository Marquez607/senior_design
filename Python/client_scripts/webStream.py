from pywebio import *
from pywebio.output import *
from pywebio.input import *
import multiprocessing as mp
from pywebio.session import *

import time
import atexit
import os 
import subprocess

cam_server_ip = "192.168.0.102"
cam_port = 1000

def webPage():
    put_markdown("""# Pywebio Test Video Stream """,lstrip=True)
    put_html("<iframe src= \"http://localhost:4041/video_feed\" width=\"320\" height=\"240\"></iframe>")
    # put_html("<iframe src=\"https://www.youtube.com/embed/YQHsXMglC9A\" width=\"853\" height=\"480\" frameborder=\"0\" allowfullscreen></iframe>")

if __name__ == '__main__':
    start_server(webPage,port=4040)     