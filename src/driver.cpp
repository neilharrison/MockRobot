#include <src/driver.h>

int main() {
    std::string ip{"127.0.0.1"};

    DeviceDriver driver;
    // driver.OpenConnection(ip);
    // std::cout<<driver.Initialize()<<"\n";
    // std::cout<<driver.ExecuteOperation("Transdfer", {"Destination Location", "Source Location"}, {"5", "12"})<<"\n";
    // std::cout<<driver.ExecuteOperation("Transfer", {"Source Location","Destination Location"}, {"12", "5"})<<"\n";
    // std::cout<<driver.ExecuteOperation("Pick", {"Source Location","Destination Location"}, {"12", "5"})<<"\n";
    
    std::string input;
    while (std::cin>>input) {
    
        if (input == "open") std::cout<<driver.OpenConnection(ip)<<"\n";
        else if (input == "init") std::cout<<driver.Initialize()<<"\n";
        else if (input == "transfer"){
            std::string source,dest;
            std::cin>>source>>dest;
            std::cout<<driver.ExecuteOperation("Transfer", {"Source Location","Destination Location"}, {source, dest})<<"\n";
        } 
        else if (input == "pick") {
            std::string source;
            std::cin>>source;
            std::cout<<driver.ExecuteOperation("Pick", {"Source Location"}, {source})<<"\n";
            }

        else if (input == "place") {
            std::string dest;
            std::cin>>dest;
            std::cout<<driver.ExecuteOperation("Place", {"Destination Location"}, {dest})<<"\n";
            }
        else if (input == "abort") std::cout<<driver.Abort()<<"\n";
    }
    std::cout<<driver.Abort()<<"\n";
}