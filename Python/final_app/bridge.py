from pywebio import *
from pywebio.output import *
from pywebio.input import *
import multiprocessing as mp
from pywebio.session import *
import time
import atexit

WHEELSON_ICON = '🚗'   #wheelson gps location
CMD_CENTER_ICON = '💻' #server gps location
WAYPOINT_ICON = '📍'
TRASH_ICON = '🗑'

ICONS = {
    "WHEELSON" : WHEELSON_ICON,
    "CMD CENTER" : CMD_CENTER_ICON,
    "WAYPOINT" : WAYPOINT_ICON,
    "TRASH" : TRASH_ICON
}

COMMAND_CENTER_POS = (7,7)
WHEELSON_DEFAULT_POS = (4,0)
CMD_MAP_SIZE = 10

cmd_map = [
    [-1] * CMD_MAP_SIZE
    for _ in range(CMD_MAP_SIZE)
]

cmd_fifo = mp.Queue()
wheelson_loc_fifo = mp.Queue()
wheelson_proc = None #had to use a separate process because python multithreading sucks

wheelson_pos = WHEELSON_DEFAULT_POS

session_count = 0

#================================== WHEELSON PDU ==========================================#
class wheelsonPDU():
    def __init__(self,data=None,header=None):
        self.LOC_HEADER = 0
        self.MSG_HEADER = 1
        self.data = data 
        self.header = header

#================================== WEB PAGE THREAD =======================================#
def webpage(): 
    global cmd_map,session_count

    session_count +=1 
    
    if session_count <= 1:
        # cmd_map[COMMAND_CENTER_POS[0]][COMMAND_CENTER_POS[1]] = "CMD CENTER"
        cmd_map[WHEELSON_DEFAULT_POS[0]][WHEELSON_DEFAULT_POS[1]] = "WHEELSON"

    set_env(output_animation=False)

    chat_msg = []
    msg_box = output()

    def print_term(content):
        chat_msg.append(content)
    
    def refresh_term():
        for m in chat_msg: 
            msg_box.append(put_markdown(m,sanitize=True))

    print = print_term

    def place_waypoint(pos):
        x,y = pos
        cmd_map[x][y] = "WAYPOINT"

        # add to command list
        print(f"CMD SENT: MOVE {x},{y}")

        # add to command pipe
        cmd_fifo.put(pos)

        show_cmd_map() 

    def move_wheelson(pos):
        global wheelson_pos
        global cmd_fifo

        x,y = pos 

        #set new wheelson location
        cmd_map[x][y] = "WHEELSON"
        
        #erase previous location 
        cmd_map[wheelson_pos[0]][wheelson_pos[1]] = -1
        wheelson_pos = pos

        show_cmd_map()

    @use_scope('cmd_map', clear=True)
    def show_cmd_map():
        table = [
            [
                put_buttons([dict(label=' ', value=(x, y), color='light')], onclick=place_waypoint) if cell == -1 else ICONS[cell]
                for y, cell in enumerate(row)
            ]
            for x, row in enumerate(cmd_map)
        ]
        put_row([ 
            put_table(table),put_scrollable(msg_box, height=300, keep_bottom=True)
        ])
        refresh_term()

    put_markdown("""# Welcome to the Bridge
    The Bridge is an online interface for issuing commands and receiving updates from the Wheelson IoT robot platform""",lstrip=True)

    show_cmd_map()
    while True:
        time.sleep(1)
        #read from command pipe
        rx_pdu = wheelson_loc_fifo.get()
        if rx_pdu:
            if rx_pdu.header == rx_pdu.LOC_HEADER:
                new_pos = rx_pdu.data
                move_wheelson(new_pos)
                x,y = new_pos
                print(f"WHEELSON UPDATE: LOC {x},{y}")
                show_cmd_map()
            elif rx_pdu.header == rx_pdu.MSG_HEADER:
                print(f"WHEELSON MSG: {rx_pdu.data}")
                show_cmd_map()

#================================== WHEELSON SIMULATOR PAGE THREAD =======================================#
def wheelson_sim(cmd_fifo,wheelson_loc_fifo):
    '''
    all this thread does is simulate the front end sending a command 
    to the wheelson handler and sending back updated locations from the robot

    in actuality, the wheelson handler will not immediately update the front end
    with a new coordinate but instead the robot will transmit its current location
    and that'll be what gets added to the wheelson pipe
    '''
    print(f"STARTING WHEELSON SIMULATOR")
    current_pos = WHEELSON_DEFAULT_POS
    cmd = None 
    while True:
       
        #get command 
        cmd = cmd_fifo.get()
        x,y, = cmd
        print(f"CMD RECEIVED: MOVE {x},{y}")

        if cmd is not None: #simulate moving to new waypoint
            pdu = wheelsonPDU()
            while current_pos[0] != cmd[0] or current_pos[1] != cmd[1]:
                x,y = current_pos
                if x > cmd[0]:
                    x = x - 1
                elif x < cmd[0]:
                    x = x + 1

                if y > cmd[1]:
                    y = y - 1 
                elif y < cmd[1]:
                    y = y + 1
                current_pos = (x,y)
                time.sleep(2)
                pdu.data = current_pos
                pdu.header = pdu.LOC_HEADER
                wheelson_loc_fifo.put(pdu)
            
            #mission complete
            fin_pdu = wheelsonPDU()
            fin_pdu.data = f"FINISHED CMD MOVE TO {cmd[0]},{cmd[1]}"
            fin_pdu.header = pdu.MSG_HEADER
            wheelson_loc_fifo.put(fin_pdu)
            cmd = None

def exit_handler():
    print("exit_handler : killing all processes")
    if wheelson_proc is not None: 
        wheelson_proc.kill()

if __name__ == '__main__':
    atexit.register(exit_handler)
    wheelson_proc = mp.Process(target=wheelson_sim,args=(cmd_fifo,wheelson_loc_fifo))
    wheelson_proc.start()
    start_server(webpage,port=4040)     