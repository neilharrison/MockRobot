import socket
import threading
import random
import time

HOST = '127.0.0.1'  # Standard loopback interface address (localhost)
PORT = 10320        # Port to listen on (non-privileged ports are > 1023)

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.bind((HOST, PORT))
    s.listen()
    conn, addr = s.accept()
    statusList = ["In Progress", "Finished Successfully", "Terminated With Error"]

    with conn:
        print('Connected by', addr)
        while True:
            data = conn.recv(1024)
            msg = data.decode("utf-8")
            
            if msg!= "":
                print(msg)
                msgList = msg.split('%')
                if msgList[0]=="home":
                    outmsg = str(random.randint(0,1000))
                elif msgList[0]=="status":
                    processID = int(msgList[1])
                    outmsg = statusList[random.randint(0,2)]
                elif msgList[0]=="pick" or msgList[0] == "place":
                    outmsg = str(random.randint(0,1000))
                else:
                    outmsg = "-1"
                
            # if not data:
            #     break
                conn.sendall(outmsg.encode("utf-8"))
            time.sleep(1)