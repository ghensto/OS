/* csci4061 S2016 Assignment 4
* section: one_digit_number
* date: mm/dd/yy
* names: Name of each member of the team (for partners)
* UMN Internet ID, Student ID (xxxxxxxx, 4444444), (for partners)
*/

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include "util.h"

#define MAX_THREADS 100
#define MAX_QUEUE_SIZE 100
#define MAX_REQUEST_LENGTH 1024

static pthread_mutex_t queue_access = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t queue_full = PTHREAD_COND_INITIALIZER;
static pthread_cond_t queue_empty = PTHREAD_COND_INITIALIZER;


static int in = 0;
static int out = 0;
static int count = 0;
static int queue_size = 0;
static char root_directory[1024];
static int login = 0;

//Structure for a single request.
typedef struct request
{
        int             m_socket;
        char    m_szRequest[MAX_REQUEST_LENGTH];
} request_t;

static request_t queue[MAX_QUEUE_SIZE];
static char *log_queue[1000];


// Inserts requests into the queue
void * dispatch(void * arg)
{
        int fd, req; //file descriptors and request
        char filename[MAX_REQUEST_LENGTH];
        request_t new_request;

        while(1) {

            fd = accept_connection();
            if(fd < 0){
                perror("Failed to accept connection.\n");
                continue;
            }

            req = get_request(fd, filename);
            if(req != 0){
                  perror("Failed to receive request.\n");
                  continue;
            }

            // Initialiazes new request
            new_request.m_socket = fd;
            strncpy(new_request.m_szRequest, filename, MAX_REQUEST_LENGTH);

            // Insert request in queue

            pthread_mutex_lock(&queue_access);
            // If the queue is full, we wait
	        while (count == queue_size) {
	            pthread_cond_wait(&queue_empty, &queue_access);
            }

            // Add the new request to the queue
		    queue[in] = new_request;
		    in = (in + 1) % queue_size;
		    count++;
		    pthread_cond_signal(&queue_full); // Wake up other threads
		    pthread_mutex_unlock(&queue_access); // Give the key back
        }

        return NULL;
}

// Process a request
void * worker(void * arg) {

    request_t new_request;
    int number_of_requests_handled = 0;
    int id = (*(int*)arg);
    while(1) {
        // Removes request from the queue
        pthread_mutex_lock(&queue_access);

        // Wait until there is something in the queue
		while (count <= 0) {
		    pthread_cond_wait(&queue_full, &queue_access);
        }
        
        // Gets req to be processed
		new_request.m_socket = queue[out].m_socket;
		strncpy(new_request.m_szRequest, queue[out].m_szRequest, MAX_REQUEST_LENGTH);;

        // Increment the number of requests handled
        number_of_requests_handled += 1;
        // Remove the request from the queue
		out = (out + 1) % queue_size;
		count--;


        // Get content type path based on file type
        char content_type[1024];
        if(strstr(new_request.m_szRequest, ".jpg") != NULL) {
            strcpy(content_type,"image/jpeg");
        }
        else if(strstr(new_request.m_szRequest, ".htm") != NULL || strstr(new_request.m_szRequest,".html") != NULL) {
            strcpy(content_type,"text/html");
        }
        else if(strstr(new_request.m_szRequest, ".gif") != NULL) {
            strcpy(content_type,"image/gif");
        }
        else {
            strcpy(content_type,"text/plain");
        }

        // Get path to requested file
        size_t root_str_length = strlen(root_directory);
        size_t req_str_length = strlen(new_request.m_szRequest);
        char *path_to_file = (char *) malloc(root_str_length + req_str_length + 1); // path to the requested file
        memcpy(path_to_file, root_directory, root_str_length);
        memcpy(path_to_file + root_str_length, new_request.m_szRequest, req_str_length + 1);


        // Open web_server_log for logging
        char server_log_filename[] = "web_server_log";
        size_t server_log_filename_length = strlen(server_log_filename);
        char *path_to_log_file = (char *) malloc(root_str_length + server_log_filename_length + 1); // absolute path to the server log
        memcpy(path_to_log_file, root_directory, root_str_length);
        memcpy(path_to_log_file + root_str_length, server_log_filename, server_log_filename_length + 1);

        // Get file and read contents into buffer 'buf'
        FILE *requested_file_fd;
        int nbytes = 0;
        char *buf;
        char output[1024];

        // If the file does not exist
        if ((requested_file_fd = fopen(path_to_file, "rb")) == NULL) {
            // Return error message to request
            if (return_error(new_request.m_socket, new_request.m_szRequest) != 0){
		        perror("Failed to return error in worker.\n");
		        snprintf(output, sizeof(output), "[%d][%d][%d][%s][%s]\n",id,number_of_requests_handled,new_request.m_socket,new_request.m_szRequest,"error");
                
                // Add to log file
		        log_queue[login] = strdup(output);
		        login++;

                // return_error failed; give back the mutex lock
                pthread_cond_signal(&queue_empty);
        		pthread_mutex_unlock(&queue_access);
                continue;
		    }
		    
            // File not found; write the error and give back the mutex lock
            snprintf(output, sizeof(output), "[%d][%d][%d][%s][%s]\n",id,number_of_requests_handled,new_request.m_socket,new_request.m_szRequest,"ERROR 404: Not Found.");
            
            // Add to log file
            log_queue[login] = strdup(output);
		    login++;
            
            pthread_cond_signal(&queue_empty);
            pthread_mutex_unlock(&queue_access);
            continue;
            
        } else {
            // Getting a file was successful
            // Read contents of the file into a buffer
            // ... Determine size of file (in bytes)
            fseek(requested_file_fd,0,SEEK_END);
            nbytes = ftell(requested_file_fd);
            
            // ... Go back to the start of the file
            fseek(requested_file_fd,0,SEEK_SET);
            
            // ... Copy file contents to buffer
            buf = (char *)malloc(nbytes);
            fread(buf,nbytes,1,requested_file_fd);
            fclose(requested_file_fd);
            
            // Returns the contents of a file to the requesting client
            if (return_result(new_request.m_socket, content_type, buf, nbytes) != 0){
        	    if (return_error(new_request.m_socket, new_request.m_szRequest) != 0){
        	        perror("Failed to return error in worker.\n");
        	        snprintf(output, sizeof(output), "[%d][%d][%d][%s][%s]\n",id,number_of_requests_handled,new_request.m_socket,new_request.m_szRequest,"error");
                    
                    // Add to log file
        	        log_queue[login] = strdup(output);
		            login++;

                    // return_error failed; give back the mutex lock
                    pthread_cond_signal(&queue_empty);
            		pthread_mutex_unlock(&queue_access);
        	        continue;
        	    }
        	} // End: return_result
        } // End: reading contents of file and returning the data to the request

        snprintf(output, sizeof(output), "[%d][%d][%d][%s][%d]\n",id,number_of_requests_handled,new_request.m_socket,new_request.m_szRequest,nbytes);
        
        // Add to log file
        log_queue[login] = strdup(output);
		login++;

        free(path_to_file);
        free(buf);

		pthread_cond_signal(&queue_empty);
		pthread_mutex_unlock(&queue_access);

    }
        return NULL;
}

int main(int argc, char **argv)
{

        //Error check first.
        if(argc != 6 && argc != 7)
        {
                printf("usage: %s port path num_dispatcher num_workers queue_length [cache_size]\n", argv[0]);
                return -1;
        }

        printf("Server running.....\n");

        int port = atoi(argv[1]);
        init(port);

        // Convert relative path of server root to absolute path
        char relative_path[1024];
        strcpy(relative_path, argv[2]);
        char *ptr = realpath(relative_path, root_directory);
        if (ptr == NULL) {
            perror("Failed to get absolute path from relative path.");
            exit(1);
        }

        // Keep track of threads
        pthread_t dispatch_thread[MAX_THREADS], worker_thread[MAX_THREADS];

        // Create file to write log to
        int log;
        log = creat("web_server_log",0666);

        // Get number of dispatch threads
        int num_dispatch = atoi(argv[3]);
        if (num_dispatch <= 0 || num_dispatch > MAX_THREADS) {
            fprintf(stderr,"Invalid number of dispatch threads.\n");
            exit(-1);
        }

        // Get number of worker threads
        int num_worker = atoi(argv[4]);
        if (num_worker <= 0 || num_worker > MAX_THREADS) {
            fprintf(stderr,"Invalid number of worker threads.\n");
            exit(-1);
        }

        // Get queue size
        queue_size = atoi(argv[5]);
        if (queue_size <= 0 || queue_size > MAX_QUEUE_SIZE) {
            fprintf(stderr,"Invalid queue size.\n");
            exit(-1);
        }

        printf("Queue size is: %d", queue_size);

        // Create dispather and worker threads with thread number as parameter
        for(int i = 0; i < num_dispatch; i++){
            if(pthread_create(&dispatch_thread[i], NULL, dispatch, &i) != 0){
                fprintf(stderr,"Error creating dispatcher thread\n");
		        exit(1);
	        }
        }

	    for(int i = 0; i < num_worker; i++){
            if(pthread_create(&worker_thread[i], NULL, worker, &i) != 0){
                fprintf(stderr,"Error creating worker thread\n");
		        exit(1);
	        }
        }
        
        // Record events
        FILE *log_file;
        char buffer[1024];
        while(1){

            log_file = fopen("web_server_log", "a+");
            if(log_file == NULL) {
                perror("Error opening file.\n");
            }
            for(int i = 0; i < 1000; i++){
                if(log_queue[i] != NULL){
                    pthread_mutex_lock(&queue_access);
                    strncpy(buffer,log_queue[i],1024);
                    fprintf(log_file,"%s\n",buffer);
                    log_queue[i] = NULL;
                    pthread_mutex_unlock(&queue_access);
                }
            }

            fclose(log_file);

        }
        return 0;
}
