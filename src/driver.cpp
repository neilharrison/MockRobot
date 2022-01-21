
#include <vector>
#include <map>
#include <algorithm>

#include <iostream>
#include <stdio.h>
#include <string.h>
#include <sstream>

#include <chrono>
#include <thread>

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
        int m_processID; //Current processID
        std::vector<std::string> m_operationList;


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
            if (ss>>m_processID) {
                std::cout<<m_processID<<"\n";
                return "";
            }
            else {
                std::cout<<"Didn't recognise process ID\n";
                return "Didn't recognise process ID\n" ;
            }
            
        }

        std::string ExecuteOperation(std::string operation, std::vector<std::string> parameterNames, std::vector<std::string> parameterValues) {
        // Device Driver will perform an operation determined by the parameter operation
            std::cout<<parameterNames[0]<<"\n";
            std::string message;
            std::string source,destination;

            for (int i = 0;i!=parameterNames.size();++i) {
                if (parameterNames[i] == "Source Location") source = parameterValues[i];
                else if (parameterNames[i] == "Destination Location")  destination = parameterValues[i];
            }

            if (operation=="Transfer"){
                message = "pick%" + source;
                m_operationList.push_back(message);
                message = "place%" + destination;
            }
            else if (operation == "Pick"){
                message = "pick%" + source;
            }
            else if (operation == "Place"){
                message = "place%" + destination;
            }
            m_operationList.push_back(message);
            char buffer[1024] = {0};

            for (auto& op:m_operationList){
                send(sock,op.c_str(),op.length(),0);
                read(sock,buffer, 1024);
                std::this_thread::sleep_for(std::chrono::milliseconds(100));

                int pid;
                std::stringstream ss(buffer);
                ss>>pid;

                std::cout<<op<<std::endl;
                std::cout<<pid<<std::endl;
            }

            return "";
        }

        std::string Abort() {
            return "";
        }

    private:
        std::string getStatus(int processID) {
            char buffer[1024] = {0};
            std::string message{"status%"};
            message = message + std::to_string(processID);
            send(sock,message.c_str(),message.length(),0);
            read(sock,buffer,1024);
            
            std::string result(buffer);
            if (result == "In Progress") {
                return "In prog\n";
            }
            else if (result == "Finished Successfully"){
                return "Fin\n";
            }
            else if (result == "Terminated With Error"){
                return "Term\n";
            } 
            else {
                return "Message Error";
            }
        }


};


int main() {
    std::string ip{"127.0.0.1"};
    DeviceDriver driver;
    driver.OpenConnection(ip);
    driver.Initialize();
    driver.ExecuteOperation("Transfer", {"Destination Location", "Source Location"}, {"5", "12"});
       
}