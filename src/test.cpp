
#include <vector>
#include <map>
#include <algorithm>

#include <iostream>
#include <stdio.h>
#include <string.h>
#include <sstream>

//Network related includes:
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
//Target host details:
#define PORT 10320

class DeviceDriver{
    private:
        int sock{0}; //Socket descriptor
        int processID; //Current processID
    public:
        std::string OpenConnection(std::string IPAddress) {
        //Device Driver to establish a connection with the MockRobot onboard software
            struct sockaddr_in server;
            
            if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
            {
                std::cout<<"Socket creation error \n";
                return "Socket creation error \n";
            }
            server.sin_family = AF_INET;
            server.sin_port = htons(PORT);

            // Convert IPv4 and IPv6 addresses from text to binary form
            if(inet_pton(AF_INET, (const char *)IPAddress.c_str(), &server.sin_addr)<=0) 
            {
                std::cout<<"Invalid address/ Address not supported \n";
                return "Invalid address/ Address not supported \n";
            }

            if (connect(sock, (const sockaddr *)&server, sizeof(server)) < 0)
            {
                std::cout<<"Connection Failed \n";
                return "Connection Failed \n";
            }
            else {
                std::cout<<"Connection Succesful\n";
                return "";
            }
        }

        std::string Initialize() {
        //Device Driver will put the MockRobot into an automation-ready (homed) state.
            char buffer[1024] = {0};
            std::string message{"home"};
            send(sock, message.c_str(), message.length(), 0); //Send home message
            read(sock,buffer, 1024); //Recieve processID into buffer
        
            std::stringstream ss(buffer); //Make sure processID is an int
            if (ss>>processID) {
                std::cout<<processID<<"\n";
                return "";
            }
            else std::cout<<"Didn't recognise process ID\n";
            
        }

        std::string ExecuteOperation(std::string operation, std::vector<std::string> parameterNames, std::vector<std::string> parameterValues) {
        // Device Driver will perform an operation determined by the parameter operation
        }

        std::string Abort() {

        }

};


int main() {
    std::string ip{"127.0.0.1"};
    DeviceDriver driver;
    driver.OpenConnection(ip);
    driver.Initialize();
       
}