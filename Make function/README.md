CSci4061 F2016 Assignment 1
* login: schom120@umn.edu
* date: 10/05/16
* name: Abiola Adimi, Khoa Le, David Schommer
* id: 4980316 (adimi001@umn.edu), lexxx611@umn.edu, schom120@umn.edu */

1. How to compile the program:
	To comiple the program, run `make` on the provided Makefile. 

2. Who did what on the program: 

	Abiola Adimi: checked timestamp and displayed commands when you run makefile, testing

	Khoa Le: check_if_all_file_exist(), set up the B,n flag, test the program in different cases.

	David Schommer: navigate DAG, forking, exec call, waiting for child, README writeup

3. How to use the program from the shell (syntax):
	
	To build a program with 'makefile', navigate to the directory with your makefile and run 

		./make4061

	To build a specific target, run

		./make4061 <specificTarget>
		
	OPTIONS: 

		-f : Use 'file' as a makefile. 
		-n : Only display the commands to run; do not execute commands. 
		-B : Always recompile; do not check timestamps for target and input. 

