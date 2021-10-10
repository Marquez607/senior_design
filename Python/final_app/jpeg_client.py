'''
Name: jpeg_client
Desc: script for receiving and saving jpegs from socket
'''

import wheelsonTCP as wTCP
import time
import sys
import argparse
import time
import os
import multiprocessing as mp

def wait_for_server(server_ip,port,buffer_size):    
    '''
    Waits for server connection
    '''
    server = wTCP.wheelsonTCP(server_ip,port,buffer_size) #reconnect to server
    while server.socket is None:
        server = wTCP.wheelsonTCP(server_ip,port,buffer_size) #reconnect to server
        if server.socket is None:
            print("FAILED TO CONNECT TO SERVER")
            print("TRYING AGAIN")
            time.sleep(1) #sleep a second    
    print("CONNECTED TO SERVER")
    return server

def jpeg_client(server_ip,port,
                write_queue = None,
                save_as_file = True,
                max_num_files = 5,
                outfolder = "./rx_images",
                out_file_name = "snap"):
    '''
    ip : jpeg server ip address
    port : jpeg server port
    proc_queue : if forking as part of multiprocess app, files can be outputted to IPC pipe
    save_as_file : save received images as file 
    max_num_files : max number of saved files 
    outfolder : output folder for saved files
    out_file_name : base name for output snapshots (index + .jpg will be appended)
    '''

    if not os.path.exists(outfolder):
        os.mkdir(outfolder)


    RX_SIZE = 20 * 1024

    server = wait_for_server(server_ip,port,RX_SIZE)

    temp = 0
    temp_last = 0
    image_index = 0 #keep track of number of files
    header = False 
    new_image = []
    while True:

        data = bytearray()
        try:
            data = server.receive()   
        except Exception as e:
            print(e)    #server probably dropped
            print("WAITIN FOR SERVER CONNECTION")
            server = wait_for_server(server_ip,port,RX_SIZE)

        for byte in data:
            temp_last = temp 
            temp = byte 
            if(header): #if header found 
                #look for end of image 
                if (temp == 0xD9) and (temp_last == 0xFF):
                    header = False 
                    new_image.append(temp) #end of image                     
                    print("Saving image") 

                    #save/store image 
                    if write_queue is not None: #write to pipe if applicable
                        try:
                            write_queue.put(new_image)
                        except:
                            print("IPC PIPE FAILED") 

                    if save_as_file: #save as file if applicable
                        fname = out_file_name + "_" + str(image_index) + ".jpg"
                        path = outfolder + "/" + fname
                        with open(path,"wb") as f:
                            f.write(bytearray(new_image))

                        image_index += 1
                        image_index %= max_num_files

                    new_image = [] #clear image
                        
                else:
                    new_image.append(temp)

            elif (temp == 0xD8) and (temp_last==0xFF):
                header = True
                new_image.append(temp_last)
                new_image.append(temp)

def parse_args():
    '''
    if using as script, you can pass in the ip and port via these flags
    '''
    parser = argparse.ArgumentParser(description='jpeg receiving over socket')
    parser.add_argument('-p','--port',required=True,dest='port', type=int,
                        help='port number of jpeg server')
    parser.add_argument('-i','--ip',required=True,dest='ip',
                        help='IP address of jpeg server')

    parser.add_argument('-N',type = int,dest='N',default=5,
                        help='Number of pictures to save')

    args = parser.parse_args()

    return args.ip,args.port,args.N
    
if __name__ == "__main__": #if running source file directly
    ip,port,N = parse_args()
    jpeg_client(ip,port,max_num_files=N)