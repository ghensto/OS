/* CSCI4061 F2016 Assignment 4
* login: schom120@umn.edu
* date: 12/12/16
* section: 7
* group: 65
* name: Abiola Adimi, Khoa Le, David Schommer
* id: adimi001@umn.edu, lexxx611@umn.edu, schom120@umn.edu */

1. How to compile and run the program: 
	- To compile the program, in the ternminal, run make on the provided Makefile inside folder named "SourceCode".
	- To run the program, type the following to the command line:
		+ If you want same number of dispatch, worker thread and queue size, type: 
			./web_server 9000 <directory path to testing folder> 100 100 100
		+ If you want different number of dispatch and worker thread and queue size, type: 
			./web_server 9000 <directory path to testing folder> 50 80 100
	- Open another terminal to query file:
		+ To query single file, type:
			wget http://127.0.0.1:9000/image/jpg/29.jpg
		+ To query multiple files, type:
			wget -i <path-to-urls>/urls -O results
	
	More convenient method to automagically test the server: 
	-Run ./RunProcesses.sh

2. How the program works:
	The program creates a number of dispatcher threads and worker threads which are specified by the argument inputs. The dispatcher threads will repeatedly receive incoming connection, and read the request from the connection to put into a queue for woker thread to work on later. The worker threads will look into the queue, pick up requests from it, and serve them back to the client. While doing that the worker threads also log these requests into a file called "web_server_log" for storing information.
	The log file is saved in the directory in which the server is run. 

3. Contributions: 

Abiola Adimi: mutexes in dispatch and worker threads, writing to log-file, queue management, determining content-type of files, writing data (html, jpg, etc.) to file, creating threads, debugging 

David Schommer: mutex locks in worker threads, find paths of requested files (in worker thread), writing data (html, jpg, etc.) to file, memory-management in worker threads, debugging and segmentation fault annihilation, README

Khoa Le: modify main() to include other inputs and error checking, debugging, README documentation.
