Device Driver for MockRobot

A device driver to interface between a UI and MockRobot

Included is a c++ project containing a header file containing the driver code (driver.h) 
and a basic usage example of the driver (driver.cpp)
The cpp file contains a basic text based user interface for testing 
- this doesn't replicate required the User interface but generates function calls as the UI would do

Also included is a python file that imitates the MockRobot (fakeMockRobot_multi.py)
This creates a TCP server on local host at port 10330 (1000 was restricted for testing)
The server accepts identical commands to MockRobot and outputs randomised processIDs and status calls 