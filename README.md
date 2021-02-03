***** HTTP/1.0 server *****

Implementation of 'sws' command under BSD Unix system

Team members: Mingyao Xiong, Liam Brew

*** Major updates after course ended ***
1. the server now takes requests asychronously
2. enable setting arbitary but valid port number and ip address
3. use chroot to prevent user from accessing outside of the root
4. make CGI 'really' works. But reminder: use absolute path when setting CGI directory

Notes about bulletpoint 3:
In order to use chroot and refer to the example code from the lecture, I have to raise the privilege to superuser
temporarily and lower it later on. Therefore, for the privilege to raise power, I have to change the ownership of 
the binary file to be root and set the file setuid. 

Notes about bulletpoint 4:
This part really dragged me for a few days. At first, I realized that I should have set the environment variable
$PATH to be the CGI directory, which was passed in as a parameter for -c option. However, even after I successfully
setenv, I still couldn't get the CGI program running. I found out later that's because of the chroot, the absolute 
path of CGI directory is invalid since the 'root' has changed. So did I get it done after fixing that? No and I was
frustrated. But I was sure that it has something to do with the 'root' change since I wrote a simple test code to run 
CGI program and it worked when without chroot. Finally, I realized that the failure of execution may be caused by 
the inaccessible shared library after chroot. So I put a statically compiled binary under the CGI directory and boom,
it worked! 

*** Updates ***
-- Week of Nov 30 to Dec 7 --
1. Team met initially after class to discuss and incorporate feedback
  - Decided to scrap our interpretation of the dual stack networking code as it was quickly becoming overburdened;
  pivoted to modifying the provided sample instead.
  - Likewise for using the daemon system call
2. Met later in the week to synchronize progress
  - response generation
  - reply sending
  - user directories
  - daemon server
3. Met 7 Dec to finalize project
  - added remaining logging functionality
  - worked more on CGIs
  - commenting, styling etc.

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
