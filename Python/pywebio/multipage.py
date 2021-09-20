
from pywebio.platform.flask import webio_view
from flask import Flask
from pywebio import start_server
from pywebio.output import *
from pywebio.input import *
from pywebio.session import *

def task_1():
    put_text('task_1')
    put_buttons(['Go task 2'], [lambda: go_app('task_2')])
    put_buttons(['Go home'], [lambda: go_app('index')])
    hold()

def task_2():
    put_text('task_2')
    put_buttons(['Go task 1'], [lambda: go_app('task_1')])
    put_buttons(['Go home'], [lambda: go_app('index')])
    hold()

def index():
    put_link('Go task 1', app='task_1')  # Use `app` parameter to specify the task name
    put_link('Go task 2', app='task_2')
    put_link('Video Stream', url='192.168.1.128:4041/video')
    hold()

# equal to `start_server({'index': index, 'task_1': task_1, 'task_2': task_2})`
start_server([index, task_1, task_2],host='localhost',port=4040)