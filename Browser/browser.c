// CSci4061 F2016 Assignment 2
// date: 10/28/16
// name: Abiola Adimi, Khoa Le, David Schommer
// id: adimi001@umn.edu, lexxx611@umn.edu, schom120@umn.edu

#include "wrapper.h"
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <poll.h>
//#include <restart.h>

#define MAX_TAB 100

#define FIFO_MODES O_RDONLY

extern int errno;

/*
 * Name:                uri_entered_cb
 * Input arguments:     'entry'-address bar where the url was entered
 *                      'data'-auxiliary data sent along with the event
 * Output arguments:    none
 * Function:            When the user hits the enter after entering the url
 *                      in the address bar, 'activate' event is generated
 *                      for the Widget Entry, for which 'uri_entered_cb'
 *                      callback is called. Controller-tab captures this event
 *                      and sends the browsing request to the ROUTER (parent)
 *                      process.
 */
void uri_entered_cb(GtkWidget* entry, gpointer data) {
      int check_uri; 
	if(data == NULL) {
		return;
	}
	browser_window *b_window = (browser_window *)data;
	// This channel has pipes to communicate with ROUTER.
	comm_channel channel = b_window->channel;
	// Get the tab index where the URL is to be rendered
	int tab_index = query_tab_id_for_request(entry, data);
	if(tab_index <= 0) {
		fprintf(stderr, "Invalid tab index (%d).", tab_index);
		return;
	}
	// Get the URL.
	char* uri = get_entered_uri(entry);

	// Append your code here
	// ------------------------------------
	// * Prepare a NEW_URI_ENTERED packet to send to ROUTER (parent) process.
	child_req_to_parent request;
	request.type = NEW_URI_ENTERED;
	request.req.uri_req.render_in_tab = tab_index;
	
	if (strlen(uri) > 512){
	      perror("URL is too long; must be smaller than 512 characters");
	      return; 
	}
	
	
	strcpy(request.req.uri_req.uri, uri);
  //printf("%s\n",request.req.uri_req.uri);

	// * Send the url and tab index to ROUTER
	check_uri = write(channel.child_to_parent_fd[1], &request, sizeof(child_req_to_parent));
	
	if (check_uri == -1){
	      perror("Failed to write to the pipe in 'uri_entered_cb'\n");
	      return; 
	}
	
	return;
	// ------------------------------------
}

/*
 * Name:                create_new_tab_cb
 * Input arguments:     'button' - whose click generated this callback
 *                      'data' - auxillary data passed along for handling
 *                      this event.
 * Output arguments:    none
 * Function:            This is the callback function for the 'create_new_tab'
 *                      event which is generated when the user clicks the '+'
 *                      button in the controller-tab. The controller-tab
 *                      redirects the request to the ROUTER (parent) process
 *                      which then creates a new child process for creating
 *                      and managing this new tab.
 */
void create_new_tab_cb(GtkButton *button, gpointer data) {
      int check_create; 
	if(data == NULL) {
		return;
	}
	// This channel has pipes to communicate with ROUTER.
	comm_channel channel = ((browser_window*)data)->channel;
	// Append your code here.
	// ------------------------------------
 	// * Send a CREATE_TAB message to ROUTER (parent) process.
  	child_req_to_parent request;
	request.type = CREATE_TAB;
  	check_create = write(channel.child_to_parent_fd[1], &request, sizeof(child_req_to_parent));
	
	if (check_create == -1){
	      perror("Failed to write to the pipe in 'create_new_tab_cb'\n");
	      return;
	}
	// ------------------------------------
}

/*
 * Name:                url_rendering_process
 * Input arguments:     'tab_index': URL-RENDERING tab index
 *                      'channel': Includes pipes to communctaion with
 *                      Router process
 * Output arguments:    none
 * Function:            This function will make a URL-RENDRERING tab Note.
 *                      You need to use below functions to handle tab event.
 *                      1. process_all_gtk_events();
 *                      2. process_single_gtk_event();
*/
int url_rendering_process(int tab_index, comm_channel *channel) {
 
	// Don't forget to close pipe fds which are unused by this process
	close(channel->child_to_parent_fd[0]); 
	close(channel->parent_to_child_fd[1]); 
	
	browser_window * b_window = NULL;
	// Create url-rendering window
	create_browser(URL_RENDERING_TAB, tab_index, G_CALLBACK(create_new_tab_cb), G_CALLBACK(uri_entered_cb), &b_window, channel);
	child_req_to_parent req;
  
	while (1) {
		// Handle one gtk event, you don't need to change it nor understand what it does.
		process_single_gtk_event();
		// Poll message from ROUTER
		// It is unnecessary to poll requests unstoppably, that will consume too much CPU time
		// Sleep some time, e.g. 1 ms, and render CPU to other processes
		usleep(1000);
		
		int bytesread; 
		// Try to read data from ROUTER
		int readPipe = (channel->parent_to_child_fd)[0]; 
		bytesread = read(readPipe, &req, sizeof(child_req_to_parent));
		 
		// An error occured while reading
		if ((bytesread == -1) && errno != EAGAIN) {
		    perror("An error occured while URL-RENDERING-PROCESS polled for message from ROUTER.\n"); 
		}
		if (bytesread > 0) {
		    if (req.type == NEW_URI_ENTERED) {
		        render_web_page_in_tab(req.req.uri_req.uri, b_window);
		    }
		    else if (req.type == TAB_KILLED) {
		        process_all_gtk_events(); 
		        exit(0); 
		    }
		    else {
		        perror("Error: Unknown message type.\n"); 
		    }
		     
		}
   
 
		// Append your code here
		// Try to read data sent from ROUTER
		// If no data being read, go back to the while loop
		// Otherwise, check message type:
		//   * NEW_URI_ENTERED
		//     ** call render_web_page_in_tab(req.req.uri_req.uri, b_window);
           
		//   * TAB_KILLED
		//     ** call process_all_gtk_events() to process all gtk events and jump out of the loop
		//   * CREATE_TAB or unknown message type
		//     ** print an error message and ignore it
		// Handle read error, e.g. what if the ROUTER has quit unexpected?
	}
	return 0;
}
/*
 * Name:                controller_process
 * Input arguments:     'channel': Includes pipes to communctaion with
 *                      Router process
 * Output arguments:    none
 * Function:            This function will make a CONTROLLER window and
 *                      be blocked until the program terminates.
 */
int controller_process(comm_channel *channel) {
	// Do not need to change code in this function
	close(channel->child_to_parent_fd[0]);
	close(channel->parent_to_child_fd[1]);
	browser_window * b_window = NULL;
	// Create controler window
	create_browser(CONTROLLER_TAB, 0, G_CALLBACK(create_new_tab_cb), G_CALLBACK(uri_entered_cb), &b_window, channel);
	show_browser();
	return 0;
}

/*
 * Name:                router_process
 * Input arguments:     none
 * Output arguments:    none
 * Function:            This function implements the logic of ROUTER process.
 *                      It will first create the CONTROLLER process  and then
 *                      polling request messages from all ite child processes.
 *                      It does not need to call any gtk library function.
 */
int router_process() {
		
	int fdflags; // File descriptor flags (used for setting up non-blocking pipes)
	
	int numberOfTabs = 0; 
	int child_index = 0; // Next open spot in the channel array to place a communication channel
	comm_channel *channel[MAX_TAB]; // Communication channels between the router and the router's children 
	
	int tab_pid_array[MAX_TAB] = {0}; // You can use this array to save the pid
	                                  // of every child process that has been
					  // created. When a chile process receives
					  // a TAB_KILLED message, you can call waitpid()
					  // to check if the process is indeed completed.
					  // This prevents the child process to become
					  // Zombie process. Another usage of this array
					  // is for bookkeeping which tab index has been
					  // taken.
	// Append your code here
	// Prepare communication pipes with the CONTROLLER process
	
	// Create CONTROLLER channel (for CONTROLLER process)
	//... Create pipes        
	comm_channel controller_channel;
      channel[child_index] = &controller_channel; 
	if((pipe(channel[child_index]->parent_to_child_fd) == -1) || (pipe(channel[child_index]->child_to_parent_fd) == -1)) {
		perror("Failed to create pipes for controller.\n");
	}
	//... Setup for non-blocking read of messages from controller to parent 
	int read_channel = (channel[child_index]->child_to_parent_fd)[0]; 
	if ((fdflags = fcntl(read_channel, F_GETFL, 0)) == -1) {
		perror("Failed to get file status flags and access modes for controller.\n"); 
		return -1; 
	}
	fdflags |= O_NONBLOCK; 
	if (fcntl(read_channel, F_SETFL, fdflags) == -1) {
		perror("Failed to set file status flags and access modes for controller.\n"); 		
		return -1; 
	}
	
	//... Setup for non-blocking read of messages from router to controller 
	read_channel = (channel[child_index]->parent_to_child_fd)[0];
	if ((fdflags = fcntl(read_channel, F_GETFL, 0)) == -1) {
		perror("Failed to get file status flags and access modes for controller.\n"); 
		return -1; 
	}
	fdflags |= O_NONBLOCK; 
	if (fcntl(read_channel, F_SETFL, fdflags) == -1) {
		perror("Failed to set file status flags and access modes for controller.\n"); 		
		return -1; 
	}

	// Fork the CONTROLLER process
	// call controller_process() in the forked CONTROLLER process
	pid_t controller_pid;
	controller_pid = fork();
	if (controller_pid == -1) {
		perror("Failed to fork controller");
		return 1;
	}
	// Child process (controller)
	if (controller_pid == 0) {
		// Close unneeded pipes
        close(channel[0]->child_to_parent_fd[0]);
	    close(channel[0]->parent_to_child_fd[1]);
		// Run controller process
		controller_process(channel[child_index]);
		
	} else {
		// PARENT (router) CODE
		
		// After forking, we now add the controller (aka child process ID) to the beginning of the tab_pid_array for bookkeeping		
		tab_pid_array[0] = controller_pid; 
		
		
		int bytesread; 		
		child_req_to_parent message;
			
		while(1) {

			// for each channel, read!!
			for (int channel_index = 0; channel_index <= child_index; channel_index++) {
			
			    if (channel[channel_index] != NULL) {
			    
			        int readPipe = (channel[channel_index]->child_to_parent_fd)[0]; 
			        bytesread = read(readPipe, &message, sizeof(child_req_to_parent));
			
			        // An error occured while reading, end the program
			        if ((bytesread == -1) && errno != EAGAIN) {
				        perror("An error occured while polling.\n"); 
				        break; 
			        }
			        if (bytesread > 0) {
				        // CREATE_TAB message 
				        if (message.type == CREATE_TAB){
					        int new_tab_index = 0; 
					        // Find index to place tab 
					        for (int i = 1; i <= child_index; i++) {
					            if (channel[i] == NULL) {
					                new_tab_index = i;
					                break;  
					            }
					        }
					        if (new_tab_index == 0) {
					            child_index += 1; // Increment the child_index so that we don't overwrite other child processes
					            new_tab_index = child_index; 
					        } 
					    	
					        // Prepare communication pipes with the new URL-RENDERING process
					        //... Create (malloc) comm_channel and pipes 
					        comm_channel *tab_channel = malloc(sizeof (struct comm_channel));
					        if ((pipe(tab_channel->parent_to_child_fd) == -1) || (pipe(tab_channel->child_to_parent_fd) == -1)) {
					            perror("Failed to create pipes for new tab.\n"); 
					        }
					        //... Set up for parent non-blocking read from child  
					        int tab_read_channel = (tab_channel->child_to_parent_fd)[0]; 
					        if ((fdflags = fcntl(tab_read_channel, F_GETFL, 0)) == -1) {
					            perror("Failed to get file status flags and access modes for new tab.\n"); 
					            return -1; 
					        }
					        fdflags |= O_NONBLOCK; 
					        if (fcntl(tab_read_channel, F_SETFL, fdflags) == -1) {
					            perror("Failed to set file status flags and access modes for new tab.\n"); 
					        }
					        
					        //... Set up for child non-blocking read from parent
					        tab_read_channel = (tab_channel->parent_to_child_fd)[0]; 
					        if ((fdflags = fcntl(tab_read_channel, F_GETFL, 0)) == -1) {
					            perror("Failed to get file status flags and access modes for new tab.\n"); 
					            return -1; 
					        }
					        fdflags |= O_NONBLOCK; 
					        if (fcntl(tab_read_channel, F_SETFL, fdflags) == -1) {
					            perror("Failed to set file status flags and access modes for new tab.\n"); 
					        }
					        
					        //... Add comm_channel to channel array  
					        channel[new_tab_index] = tab_channel; 
					
					        if (numberOfTabs < MAX_TAB - 1){
					            
                                  int childpid = fork(); 
					              // Fork the URL-RENDERING process 
					              if (childpid == -1){
						              perror("\nFailed to create new child process for new tab.\n");
						              return -1;
					              }
					              // Child process (URL-RENDERING)
					              if (childpid == 0){
					                	url_rendering_process(new_tab_index, tab_channel); 
					              }  
					              // Add childpid to tab_pid_array 
					              tab_pid_array[child_index] = childpid;
					              numberOfTabs += 1; 
					        } 
					        else{
					            perror("Maximum number of tabs reached\n");
					        }
				        }
                
                        // NEW_URI_ENTERED message 
                        if(message.type == NEW_URI_ENTERED){
                          // Send message through pipe connecting the ROUTER to the corresponding URL-RENDERING process 
                          int tab_number = message.req.uri_req.render_in_tab; 
                          // Check if tab exists
                          if (channel[tab_number] == NULL || tab_number > child_index) {
                            fprintf(stderr, "Invalid tab number: Tab %d does not exist.\n", tab_number); 
                          } else {
                            int writePipe = (channel[tab_number]->parent_to_child_fd)[1];  
                            if (write(writePipe, &message, sizeof(child_req_to_parent)) == -1) {
                                perror("Failed to write message from ROUTER to URL-RENDERING.\n"); 
                            }  
                          }
                        }
                        
                        // TAB_KILLED message 
                        if(message.type == TAB_KILLED){
                            // CONTROLLER is killed
                            if (message.req.killed_req.tab_index == 0){
                                 for (int i = 1; i <= child_index; i++) { 
                                    if (channel[i] != NULL) {
                                        int writePipe = (channel[i]->parent_to_child_fd)[1]; 
                                        if (write(writePipe, &message, sizeof(child_req_to_parent)) == -1) {
                                            perror("Failed to write kill message from ROUTER to URL-RENDERING.\n"); 
                                        }
                                        // Wait for URL-RENDERING process to close 
                                        if (waitpid(tab_pid_array[i], NULL, 0) == -1) {
                                            perror("Failed to wait for child.\n"); 
                                        }   
                                    }
                                 }
                                 exit(0);   
                            } 
                            // A URL-RENDERING process is killed
                            else {
                                int tab_number = message.req.killed_req.tab_index;
                                int writePipe = (channel[tab_number]->parent_to_child_fd)[1]; 
                                if (write(writePipe, &message, sizeof(child_req_to_parent)) == -1) {
                                    perror("Failed to write kill message from ROUTER to URL-RENDERING.\n"); 
                                } 
                                // Wait for URL-RENDERING process to close 
                                if (waitpid(tab_pid_array[tab_number], NULL, 0) == -1) {
                                    perror("Failed to wait for child.\n"); 
                                }
                                
                                // Cleanup 
                                //... Close pipe fds for the url-rendering process we just closed 
                                close(channel[tab_number]->child_to_parent_fd[0]);
	                            close(channel[tab_number]->parent_to_child_fd[1]); 
                                
                                //... Remove child pid from bookkeeping array and communication channel array 
                                tab_pid_array[tab_number] = 1;
                                numberOfTabs -= 1; 
                                free(channel[tab_number]); 
                                channel[tab_number] = NULL;
                            } // End killing one URL-RENDERING process
                        } // End TAB_KILLED
			        } // End if bytes read is greater than 0
			    } // End if statement checking if a channel is NULL 
			} // End for loop (loop for every channel) 

			usleep(1000);  

		} // End while 

	} // End else (ROUTER process)


	//
	return 0;
}

int main() {
	return router_process();
}
