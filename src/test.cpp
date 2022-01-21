
#include <vector>
#include <map>
#include <algorithm>

#include <iostream>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//Network related includes:
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
//Target host details:
#define PORT 10330

std::string OpenConnection(std::string IPAddress) {
//Device Driver to establish a connection with the MockRobot onboard software
    int sock{0};
    int valread;
    struct sockaddr_in server;
    char buffer[1024] = {0};
 
    std::string message = "Hello";
    
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        std::cout<<"Socket creation error \n";
        return "Socket creation error \n";
    }
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, (char *)IPAddress.c_str(), &server.sin_addr)<=0) 
    {
        std::cout<<"Invalid address/ Address not supported \n";
        return "Invalid address/ Address not supported \n";
    }

    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        std::cout<<"Connection Failed \n";
        return "Connection Failed \n";
    }

    connect(sock, (const sockaddr *)&server, sizeof(server));
    
    send(sock, (char *)message.c_str(), strlen((char *)message.c_str()), 0);
    
    read(sock,buffer, 1024);
    std::cout<<buffer<<"\n";
}

std::string Initialize() {
//Device Driver will put the MockRobot into an automation-ready (homed) state.
}

std::string ExecuteOperation(std::string operation, std::vector<std::string> parameterNames, std::vector<std::string> parameterValues) {
// Device Driver will perform an operation determined by the parameter operation
}

std::string Abort() {

}




int main() {
    OpenConnection("127.0.0.1");

       
}