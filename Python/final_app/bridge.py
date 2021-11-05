from pywebio import *
from pywebio.output import *
from pywebio.input import *
import multiprocessing as mp
from pywebio.session import *
import time
import atexit

import cmd_client as cCli #needed for pdu framing

WHEELSON_ICON = '🚗'   #wheelson gps location
CMD_CENTER_ICON = '💻'
WAYPOINT_ICON = '📍'
TRASH_ICON = '🗑'

ICONS = {
    "WHEELSON" : WHEELSON_ICON,
    "CMD CENTER" : CMD_CENTER_ICON,
    "WAYPOINT" : WAYPOINT_ICON,
    "TRASH" : TRASH_ICON
}

COMMAND_CENTER_POS = (7,7)
WHEELSON_DEFAULT_POS = (0,0)
CMD_MAP_SIZE = 10

cmd_map = [
    [-1] * CMD_MAP_SIZE
    for _ in range(CMD_MAP_SIZE)
]

cmdPipe_g = None
updatePipe_g = None
wheelson_proc = None #had to use a separate process because python multithreading sucks
port_g = None

wheelson_pos = WHEELSON_DEFAULT_POS

session_count = 0

#================================== WEB PAGE THREAD =======================================#
def webpage(): 
    global cmd_map,session_count

    session_count +=1 
    
    if session_count <= 1:
        # cmd_map[COMMAND_CENTER_POS[0]][COMMAND_CENTER_POS[1]] = "CMD CENTER"
        cmd_map[WHEELSON_DEFAULT_POS[0]][WHEELSON_DEFAULT_POS[1]] = "WHEELSON"

    set_env(output_animation=False)

    chat_msg = []
    #table = []
    msg_box = output()

    def print_term(content):
        chat_msg.append(content)
    
    def refresh_term():
        for m in chat_msg: 
            msg_box.append(put_markdown(m,sanitize=True))

    print = print_term

    def send_reset(blank):
        pdu = cCli.pdu()
        pdu.cmd = pdu.RESET
        pdu.x = x 
        pdu.y = y          
        pdu.msg = "RESET_CMD"
        pdu.msg_len = len(pdu.msg) + 1
        
        # add to command pipe
        cmdPipe_g.put(pdu)    

    def send_stop(blank):
        pdu = cCli.pdu()
        pdu.cmd = pdu.STOP
        pdu.x = x 
        pdu.y = y          
        pdu.msg = "STOP_CMD"
        pdu.msg_len = len(pdu.msg) + 1
        
        # add to command pipe
        cmdPipe_g.put(pdu)       

    def place_waypoint(pos):
        x,y = pos
        cmd_map[x][y] = "WAYPOINT"

        # add to command list
        print(f"CMD SENT: MOVE {x},{y}")

        #pack pdu
        pdu = cCli.pdu()
        pdu.cmd = pdu.MOVE
        pdu.x = x 
        pdu.y = y          
        pdu.msg = "MOVE_CMD"
        pdu.msg_len = len(pdu.msg) + 1
        
        # add to command pipe
        cmdPipe_g.put(pdu)

        show_cmd_map() 

    def move_wheelson(pos):
        global wheelson_pos
        global cmdPipe_g 

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
    The Bridge is an web interface for issuing commands and receiving updates from the Wheelson IoT robotics platform""",lstrip=True)

    put_html("<iframe src= \"http://localhost:4041/video\" width=\"320\" height=\"240\"></iframe>")

    show_cmd_map()

    # # put_button("SEND RESET",onclick=send_reset)
    # put_buttons([dict(label="SEND STOP",value=(),color='light' )],onclick=send_stop)

    while True:
        time.sleep(1)
        #read from update pipe
        rx_pdu = updatePipe_g.get()
        if rx_pdu is not None:
            if rx_pdu.cmd == rx_pdu.UPDATE or rx_pdu.cmd == rx_pdu.BLOCK:
                new_pos = (rx_pdu.x ,rx_pdu.y)
                move_wheelson(new_pos)
                x,y = new_pos
                print(rx_pdu.msg)
                show_cmd_map()
            # elif rx_pdu.header == rx_pdu.MSG_HEADER:
            #     print(f"WHEELSON MSG: {rx_pdu.data}")
            #     show_cmd_map()
            rx_pdu = None

def bridgeProcess(cmdPipe,updatePipe,port=4040,debug=False):

    global cmdPipe_g
    global updatePipe_g

    cmdPipe_g = cmdPipe
    updatePipe_g = updatePipe

    if debug: #debug uses simulated robot
        atexit.register(exit_handler) #in final app this won't be necessary 
        wheelson_proc = mp.Process(target=wheelson_sim,args=(cmdPipe,updatePipe))
        wheelson_proc.start()
    
    start_server(webpage,port=port)    
    

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
        cmd_pdu = cmd_fifo.get()
        print(cmd_pdu)
        cmd = (cmd_pdu.x,cmd_pdu.y)

        print(f"CMD RECEIVED: MOVE {cmd_pdu.x},{cmd_pdu.y}")

        if cmd is not None: #simulate moving to new waypoint
            pdu = cCli.pdu()
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

                #pack and send pdu
                pdu.x,pdu.y = current_pos
                pdu.cmd = pdu.UPDATE
                wheelson_loc_fifo.put(pdu)
            
            #mission complete
            fin_pdu = cCli.pdu()
            fin_pdu.msg = f"FINISHED CMD MOVE TO {cmd[0]},{cmd[1]}"
            fin_pdu.msg_len = len(fin_pdu.msg) + 1 # +1 for NULL terminator
            fin_pdu.msg = pdu.UPDATE
            wheelson_loc_fifo.put(fin_pdu)
            cmd = None

def exit_handler():
    print("exit_handler : killing all processes")
    if wheelson_proc is not None: 
        wheelson_proc.terminate()

if __name__ == '__main__':
    # global cmdPipe_g
    # global updatePipe_g

    cmdPipe_g = mp.Queue()
    updatePipe_g = mp.Queue()
   
    bridgeProcess(cmdPipe_g,updatePipe_g,debug=True)