***** HTTP/1.0 server *****

Implementation of 'sws' command under BSD Unix system

Team members: Mingyao Xiong, Liam Brew

*** Updates ***

--- Nov.23.2020 ---
1. merge response.c into the project
  - the server can now parse the request correctly and talks back, but the response message is 
    not correct yet, It's very likely that the dir path is not set up properly. Additonal to
	that, the chroot() should be called to lock the root directory. These will be solved in the
	following updates.

--- Nov.19.2020 ---
1. combined separate branches into master
2. settled on final snapshot work breakdown structure
  - mxiong will finish his ongoing response work
  - lbrew will finish his ongoing connectivity functionality

--- Nov.12.2020 ---
1. parser.c extracts the information from a legit HTTP/1.0 message.
2. team met with eachother to discuss project status:
  - lbrew added functionality related to options parsing, daemonization and basic socket connections
  - mxiong added functionality related to request validation and reading

--- Nov.4.2020 ---
1. team met and discussed the project and settled on an initial work breakdown structure
  - mxiong will handle request parsing and interpretation
  - lbrew will handle initial conenctivity functionality
