import socket
import time

HOST = '127.0.0.1'  # The server's hostname or IP address
PORT = 10000        # The port used by the server

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect((HOST, PORT))
    for i in range(10):
        msg = "pick%{}".format(i)
        s.sendall(msg.encode('utf-8'))
        data = s.recv(1024)
        print('Received', data.decode('utf-8'))
        time.sleep(1)
        

