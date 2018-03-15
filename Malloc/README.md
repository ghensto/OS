/* CSCI4061 F2016 Assignment 3
* login: schom120@umn.edu
* date: 11/21/16
* section: 7
* name: Abiola Adimi, Khoa Le, David Schommer
* id: adimi001@umn.edu, lexxx611@umn.edu, schom120@umn.edu */

1. How to compile the program
    -run make on the provided Makefile inside folder named "src".


2. How to use the program from the shell
    *Optional way:
        -modify number of messages you want to send inside Runprocesses.sh
        -run ./Runprocesses.sh

    *Normal way:
        -open two terminals and go to the directory containing all the files
        -First run ./packet_sender <num of messages>
        -Second run ./packet_sender <num of messages>


3. What the program does
    -The program sends messages from packet_sender process and receives them in
    packet_receiver process
    -The program also includes a memory manager that is thread/signal safe


4. Work Distribution

Abiola Adimi: Worked on packet_sender and error checking for partB
    -Create a packet_queue_msg for the current packet.
    -send this packet_queue_msg to the receiver. Handle any error appropriately.
    -send SIGIO to the receiver if message sending was successful.
    -Create a message queue
    -read the receiver pid from the queue and store it for future use
    -set up alarm handler and mask all signals within it
    -setup alarm timer

Khoa Le
    -Implement memory manager using stack data structure
    -Allocate memory chunk for application
    -Create the main_malloc and main_mm for comparing the speed
    -Fix minor bugs in packet_sender

David Schommer:
    -memory manager testing
    -implement packet_handler() (for receiver)
    -implement assemble_message() (for receiver)
    -send pid from receiver to sender
    -setup message queue in receiver
    -setup SIGIO handler (for receiver)
