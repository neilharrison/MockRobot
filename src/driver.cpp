
#include <vector>

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

bool isNum(std::string);

class DeviceDriver{
    private:
        int sock{0}; //Socket descriptor
        int m_processID{0}; //Current processID 
        std::queue<std::string> m_operationQ; //Queue of operations left to do
        bool m_connected{false};
        bool m_initialised{false};
        std::thread update;
        std::mutex m;


    public:
        std::string OpenConnection(std::string IPAddress) {
        //Device Driver to establish a connection with the MockRobot onboard software
            struct sockaddr_in server;
            if (!m_connected){
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
        }


        std::string Initialize() {
        //Device Driver will put the MockRobot into an automation-ready (homed) state.
            if (m_connected){
                if (!m_initialised) {
                    std::string message{"home"};
                    std::lock_guard<std::mutex> guard(m);
                    m_operationQ.push(message);
                    m_initialised = true; //maybe check when process is finished?
                    return "";
                }
                else return "Already initialised!";
            }    
            else return "Not connected yet!";
        }

        std::string ExecuteOperation(std::string operation, std::vector<std::string> parameterNames, std::vector<std::string> parameterValues) {
        // Device Driver will perform an operation determined by the parameter operation
        // Check all input is valid before pushing it to operation queue
            if (m_initialised){
                std::string message, source, destination;
                //Check parameter names and values are valid
                if (parameterNames.size() == parameterValues.size()){
                    for (int i = 0;i!=parameterNames.size();++i) {
                        if (parameterNames[i] == "Source Location" && isNum(parameterValues[i]))
                            source = parameterValues[i];
                        else if (parameterNames[i] == "Destination Location" && isNum(parameterValues[i]))
                            destination = parameterValues[i];
                        else return "Parameter name/value not recognised: " + parameterNames[i];
                    }
                }
                else return "Mismatch in number of parameter names and values";

                //Will be writing to operation list so wait for lock
                std::lock_guard<std::mutex> guard(m);
                //Check operations are valid and make sure correct parameters are used
                if (operation=="Transfer"){
                    if (source != "" && destination != ""){
                    message = "pick%" + source;
                    m_operationQ.push(message);
                    message = "place%" + destination;
                    }
                    else return "Missing source or destination parameter";
                }
                else if (operation == "Pick"){
                    if (source != "") message = "pick%" + source;
                    else return "Missing source parameter";
                }
                else if (operation == "Place"){
                    if (destination != "") message = "place%" + destination;
                    else return "Missing destination parameter";
                }
                else {
                    return "Operation not recognised: " + operation;
                }
                m_operationQ.push(message);
                return "";
            }
            else return  "Not yet initialised!";
        }

        std::string Abort() {
            std::lock_guard<std::mutex> guard(m);
            while (!m_operationQ.empty())m_operationQ.pop(); //remove everything from queue
            close(sock);
            m_connected = false;
            m_initialised = false;
            return "";
        }

    private:
        void doUpdate(){
            //Running all the time once connected - in separate thread
            //Handles the sending and recieving of operations/process IDs
            //Waits until current process is done, sends the next operation, recieves its process ID then removes it from the queue
            while (m_connected) {
                
                char buffer[1024] = {0};
                int status = getStatus();
                while (status == 0) {
                    //Sleep time should be increased as robot responds on the minute scale - don't spam status
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    status = getStatus();
                }
                if (status == 1) {
                    std::lock_guard<std::mutex> guard(m);
                    if (!m_operationQ.empty()){
                        send(sock,m_operationQ.front().c_str(),m_operationQ.front().length(),0);
                        std::cout<<"sending: "<<m_operationQ.front()<<"\n";
                        read(sock,buffer, 1024);
                        
                        std::stringstream ss(buffer);
                        ss>>m_processID;
                        std::cout<<"Reading: "<<m_processID<<"\n";
                        m_operationQ.pop();
                    }

                }
                else if (status<0) {
                    //If process failed, Abort is called - this might not be the intended behaviour depending on robot
                    std::cout<<"Process ID failed: "<<m_processID<<"\n";
                    Abort();
                    // return "Process ID failed: " + std::to_string(m_processID);
                }
            }
        }

    
        int getStatus() {
            //Only called by update thread
            //Sends status message for current process ID, returns an int based on response
            //When a process is finished - process ID is reset to 0 
            // ^ this prevents additional status calls if no more operations in list
            if (m_processID != 0) {
                char buffer[1024] = {0};
                std::string message{"status%"};
                message = message + std::to_string(m_processID);
                send(sock,message.c_str(),message.length(),0);
                read(sock,buffer,1024);
                
                std::string result(buffer);
                std::cout<<result<<"\n";
                if (result == "In Progress") {
                    return 0;
                }
                else if (result == "Finished Successfully"){
                    std::cout<<"Process ID finished: "<<m_processID<<"\n";
                    m_processID = 0; // 
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
            //When class is destroyed - make sure update loop is finished and join thread
            m_connected = false;
            if (update.joinable()) update.join(); //if never connected then thread won't have been started
        }

};

bool isNum(std::string test){
    std::stringstream ss(test);
    float fl;
    if (ss>>fl && ss.eof()) return true;
    else return false;
}


int main() {
    std::string ip{"127.0.0.1"};

    DeviceDriver driver;
    driver.OpenConnection(ip);
    std::cout<<driver.Initialize()<<"\n";
    // std::cout<<driver.ExecuteOperation("Transdfer", {"Destination Location", "Source Location"}, {"5", "12"})<<"\n";
    // std::cout<<driver.ExecuteOperation("Transfer", {"Source Location","Destination Location"}, {"12", "5"})<<"\n";
    std::cout<<driver.ExecuteOperation("Pick", {"Source Location","Destination Location"}, {"12k", "5"})<<"\n";

    
    // std::cout<<driver.ExecuteOperation("Transfer", {"Source Location","Destination Location"}, {"3", "5"})<<"\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(10000));

    driver.Abort();// while (true) {}; // Not the best way to keep driver alive
    // 
}