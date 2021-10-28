from pywebio import *
from pywebio.output import *
from pywebio.input import *
import multiprocessing as mp
from pywebio.session import *

import time
import atexit
import os 
import subprocess

def webPage():
    put_markdown("""# Investigator Test Video Stream """,lstrip=True)
    put_html("<iframe src= \"http://localhost:4041/video\" width=\"320\" height=\"240\"></iframe>")
    # put_html("<iframe src=\"https://www.youtube.com/embed/YQHsXMglC9A\" width=\"853\" height=\"480\" frameborder=\"0\" allowfullscreen></iframe>")

def main(port=4040):
    start_server(webPage,port=port)  

if __name__ == '__main__':
    main(4040)  