
#include <vector>
#include <map>
#include <algorithm>

#include <queue>
#include <mutex>
#include <thread>

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
        int m_processID{0}; //Current processID 
        std::queue<std::string> m_operationList;
        bool m_connected{false};
        bool m_initialised{false};
        std::thread update;
        std::mutex m;


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
                m_connected = true;
                update = std::thread{&DeviceDriver::doUpdate, this};
                return "";
            }
        }

        std::string Initialize() {
        //Device Driver will put the MockRobot into an automation-ready (homed) state.
            if (!m_initialised) {
                std::string message{"home"};
                std::lock_guard<std::mutex> guard(m);
                m_operationList.push(message);
                std::cout<<m_operationList.front()<<"\n";
                m_initialised = true; //maybe check when process is finished?
            }
            
            
        }

        std::string ExecuteOperation(std::string operation, std::vector<std::string> parameterNames, std::vector<std::string> parameterValues) {
        // Device Driver will perform an operation determined by the parameter operation
            std::string message;
            std::string source,destination;

            for (int i = 0;i!=parameterNames.size();++i) {
                if (parameterNames[i] == "Source Location") source = parameterValues[i];
                else if (parameterNames[i] == "Destination Location")  destination = parameterValues[i];
            }

            std::lock_guard<std::mutex> guard(m);
            if (operation=="Transfer"){
                message = "pick%" + source;
                m_operationList.push(message);
                message = "place%" + destination;
            }
            else if (operation == "Pick"){
                message = "pick%" + source;
            }
            else if (operation == "Place"){
                message = "place%" + destination;
            }
            m_operationList.push(message);
            
            return "";
        }

        std::string Abort() {
            std::lock_guard<std::mutex> guard(m);
            while (!m_operationList.empty())m_operationList.pop(); //remove everything from queue
            close(sock);
            m_connected = false;
            m_initialised = false;
            update.join();
            return "";
        }

        void doUpdate(){
            while (m_connected) {
                
                char buffer[1024] = {0};
                int status = getStatus(m_processID);
                while (status == 0) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    status = getStatus(m_processID);
                }
                if (status == 1) {
                    std::cout<<"Process ID finished: "<<m_processID<<"\n";
                    std::string returnMsg = "Process ID finished: " + std::to_string(m_processID);

                    std::lock_guard<std::mutex> guard(m);
                    
                    send(sock,m_operationList.front().c_str(),m_operationList.front().length(),0);
                    std::cout<<"sending: "<<m_operationList.front()<<"\n";
                    read(sock,buffer, 1024);
                    
                    std::stringstream ss(buffer);
                    ss>>m_processID;
                    std::cout<<"Reading: "<<m_processID<<"\n";
                    
                    m_operationList.pop();
            
                    // return returnMsg; 

                }
                else if (status<0) {
                    std::cout<<"Process ID failed: "<<m_processID<<"\n"; 
                    // return "Process ID failed: " + std::to_string(m_processID);
                }
            }
        }

    private:
        int getStatus(int processID) {
            if (processID != 0) {
                char buffer[1024] = {0};
                std::string message{"status%"};
                message = message + std::to_string(processID);
                send(sock,message.c_str(),message.length(),0);
                read(sock,buffer,1024);
                
                std::string result(buffer);
                if (result == "In Progress") {
                    return 0;
                }
                else if (result == "Finished Successfully"){
                    return 1;
                }
                else if (result == "Terminated With Error"){
                    return -1;
                } 
                else {
                    return -2;
                }
            }
            else return 1; //process id = 0, not started yet - move onto first in op list (assumes mockRobot doesnt return processIds of 0)
        }
    public:
        ~DeviceDriver(){
            m_connected = false;
            update.join();
        }


};


int main() {
    std::string ip{"127.0.0.1"};
    DeviceDriver driver;
    driver.OpenConnection(ip);
    driver.Initialize();
    driver.ExecuteOperation("Transfer", {"Destination Location", "Source Location"}, {"5", "12"});
    driver.ExecuteOperation("Transfer", {"Source Location","Destination Location"}, {"12", "5"});
    while (true) {}; // Not the best way to keep driver alive
    // driver.Abort();
}