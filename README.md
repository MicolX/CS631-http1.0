***** HTTP/1.0 server *****

Implementation of 'sws' command under BSD Unix system

Team members: Mingyao Xiong, Liam Brew

*** Updates ***
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
