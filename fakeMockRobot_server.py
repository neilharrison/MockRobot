import socket
import random

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
            print(data.decode("utf-8"))
            msg = data.decode("utf-8")
            msgList = msg.split('%')
            if msgList[0]=="home":
                msg = str(random.randint(0,1000))
            elif msgList[0]=="status":
                processID = int(msgList[1])
                msg = statusList[random.randint(0,2)]
            if not data:
                break
            conn.sendall(msg.encode("utf-8"))