import socket
import threading
import random
# Threaded server help from:
# https://stackoverflow.com/questions/23828264/how-to-make-a-simple-multithreaded-socket-server-in-python-that-remembers-client

class ThreadedServer(object):
    def __init__(self, host, port):
        self.host = host
        self.port = port
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.sock.bind((self.host, self.port))
        self.statusList = ["In Progress", "Finished Successfully", "Terminated With Error"]

    def listen(self):
        self.sock.listen(5)
        while True:
            client, address = self.sock.accept()
            client.settimeout(60) # Shouldnt cause an issue as update should keep alive
            threading.Thread(target = self.listenToClient,args = (client,address)).start()

    def listenToClient(self, client, address):
        size = 1024
        while True:
            try:
                data = client.recv(size)
                msg = data.decode("utf-8")
                
                if msg!= "":
                    print(msg)
                    msgList = msg.split('%')
                    if msgList[0]=="home":
                        outmsg = str(random.randint(1,1000))
                    elif msgList[0]=="status":
                        processID = int(msgList[1])
                        outmsg = self.statusList[random.randint(0,2)]
                        print(outmsg)
                    elif msgList[0]=="pick" or msgList[0] == "place":
                        outmsg = str(random.randint(1,1000))
                    else:
                        outmsg = "-1"
                    
                # if not data:
                #     break
                    client.sendall(outmsg.encode("utf-8"))
                
            except:
                client.close()
                return False

if __name__ == "__main__":
    host = '127.0.0.1'
    port = 10320
    ThreadedServer(host,port).listen()