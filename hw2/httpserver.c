#include <arpa/inet.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <unistd.h>

#include "libhttp.h"
#include "wq.h"

/*
 * Global configuration variables.
 * You need to use these in your implementation of handle_files_request and
 * handle_proxy_request. Their values are set up in main() using the
 * command line arguments (already implemented for you).
 */
wq_t work_queue;
int num_threads;
int server_port;
char *server_files_directory;
char *server_proxy_hostname;
int server_proxy_port;


/*
 * Reads an HTTP request from stream (fd), and writes an HTTP response
 * containing:
 *
 *   1) If user requested an existing file, respond with the file
 *   2) If user requested a directory and index.html exists in the directory,
 *      send the index.html file.
 *   3) If user requested a directory and index.html doesn't exist, send a list
 *      of files in the directory with links to each.
 *   4) Send a 404 Not Found response.
 */
void handle_files_request(int fd) {

  /*
   * TODO: Your solution for Task 1 goes here! Feel free to delete/modify *
   * any existing code.
   */

  struct http_request *request = http_request_parse(fd);

  
  
  // get the final path the user requested.
  //printf("%s", "server_files_directory is ");
  //printf("%s", server_files_directory);
  char *path = malloc(strlen(server_files_directory) + strlen(request->path) + 1); 
  strcpy(path, server_files_directory);
  strcat(path, request->path);
  //printf("%s", "+++++++++++++++++++++++++++++++++++++++++++++ \n");
  //printf("%s", path);
   
  // check the given argument is a filename or a directory or 
  

  struct stat newstat;
  if (stat(path, &newstat) == -1){
    http_start_response(fd, 404);
    http_send_header(fd, "Content-Type", "text/html");
    http_end_headers(fd);
    http_send_string(fd, "");  
  } else{

  // Problem: the prob is that derectory and nonexist file will all fall into is_file = 0 
  
  // 1) If user requested an existing file, respond with the file
  // check whether exist 
  // set content-type header
  if(S_ISREG(newstat.st_mode)){
    //printf("%s", "I'm here in is_file=1 condition");
    http_start_response(fd, 200);
    
    
    int filesize;
    
    char read_buff[1024];
    //open the file as fd
    int sourcefd = open(path, O_RDONLY);
    filesize = lseek(sourcefd, (size_t)0, SEEK_END);
    lseek(sourcefd, (size_t)0, SEEK_SET);
    char fs[1024];
    sprintf(fs, "%d", filesize);
    http_send_header(fd, "Content-Type", http_get_mime_type(request->path));
    http_send_header(fd, "Content-Length", fs);
    http_end_headers(fd);
    while(read(sourcefd, read_buff, 1024 * sizeof(char))){
      http_send_data(fd, read_buff, 1024 * sizeof(char));
    }
  }else if(S_ISDIR(newstat.st_mode)){
    //printf("%s", "I'm here in is_file = 0 condition");
    //need to tell if it is the case that the file doesn't exist
    char *ultpath = malloc(strlen(path) + strlen("/index.html") + 1);
    //strcpy(ultpath, "/");
    strcpy(ultpath, path);
    strcat(ultpath, "/index.html");
    
    //printf("%s", ultpath);

    struct stat new2stat;
    int not_exist =

     stat(ultpath, &new2stat);
   
    if (!not_exist){
      http_start_response(fd, 200);

      //use lseek to get the length of the required file  
      
      int filesize;

      
      char read_buff[1024];
      //open the file as fd 
      int sourcefd = open(ultpath, O_RDONLY);

      filesize =lseek(sourcefd, (size_t)0, SEEK_END);
      lseek(sourcefd, (size_t)0, SEEK_SET);
      char fs[1024];
      sprintf(fs, "%d", filesize);

      http_send_header(fd, "Content-Length", fs);
      http_send_header(fd, "Content-Type", "text/html");
      http_end_headers(fd);

      while(read(sourcefd, read_buff, sizeof(read_buff))){
        http_send_data(fd, read_buff, sizeof(read_buff));
      }
    }else{
      // file doesn't exist or directory doesn't contatin .html
      // throw children and a link to the parent directory
      http_start_response(fd, 200);
      http_send_header(fd, "Content-Type", "text/html");
      http_end_headers(fd);

      // get the current directory adn print out all the entries


      
      struct dirent *rDir;
      DIR *oDir;
      oDir = opendir (path);
      while ((rDir = readdir(oDir)) != NULL) {
            //direference outside here 
            char *fname = rDir->d_name;
            char *link = malloc(strlen("<html><body><a href='/'>") + strlen("</a></body></html>") + 2*strlen(fname)+ 1);
            strcpy(link, "<html><body><a href='");
            strcat(link, fname);
            strcat(link, "'>");
            strcat(link, fname);
            strcat(link, "</a></body></html>");
            http_send_string(fd, link);
        }
      closedir(oDir);
      http_send_string(fd, "<html><body><a href='/'>Home</a></body></html>");

    }
  }else{
    //return 404 notfound 
    http_start_response(fd, 404);
    http_send_header(fd, "Content-Type", "text/html");
    http_end_headers(fd);
    http_send_string(fd, "");
  }



  
  // set content-type header
  /*
  char *ctheader = http_get_mime_type(request->path)
  http_send_header(fd, "Content-Type", ctheader);
  http_end_headers(fd);
  http_send_string(fd,
      "<center>"
      "<h1>Welcome to httpserver!</h1>"
      "<hr>"
      "<p>Nothing's here yet.</p>"
      "</center>");
  */
}
}


/*
 * Opens a connection to the proxy target (hostname=server_proxy_hostname and
 * port=server_proxy_port) and relays traffic to/from the stream fd and the
 * proxy target. HTTP requests from the client (fd) should be sent to the
 * proxy target, and HTTP responses from the proxy target should be sent to
 * the client (fd).
 *
 *   +--------+     +------------+     +--------------+
 *   | client | <-> | httpserver | <-> | proxy target |
 *   +--------+     +------------+     +--------------+
 */
void handle_proxy_request(int fd) {

  /*
  * The code below does a DNS lookup of server_proxy_hostname and 
  * opens a connection to it. Please do not modify.
  */

  struct sockaddr_in target_address;
  memset(&target_address, 0, sizeof(target_address));
  target_address.sin_family = AF_INET;
  target_address.sin_port = htons(server_proxy_port);

  struct hostent *target_dns_entry = gethostbyname2(server_proxy_hostname, AF_INET);

  int client_socket_fd = socket(PF_INET, SOCK_STREAM, 0);
  if (client_socket_fd == -1) {
    fprintf(stderr, "Failed to create a new socket: error %d: %s\n", errno, strerror(errno));
    exit(errno);
  }

  if (target_dns_entry == NULL) {
    fprintf(stderr, "Cannot find host: %s\n", server_proxy_hostname);
    exit(ENXIO);
  }

  char *dns_address = target_dns_entry->h_addr_list[0];

  memcpy(&target_address.sin_addr, dns_address, sizeof(target_address.sin_addr));
  int connection_status = connect(client_socket_fd, (struct sockaddr*) &target_address,
      sizeof(target_address));

  if (connection_status < 0) {
    /* Dummy request parsing, just to be compliant. */
    http_request_parse(fd);

    http_start_response(fd, 502);
    http_send_header(fd, "Content-Type", "text/html");
    http_end_headers(fd);
    http_send_string(fd, "<center><h1>502 Bad Gateway</h1><hr></center>");
    return;

  }

  /* 
  * TODO: Your solution for task 3 belongs here! 
  */
}
//This part is still part two!!!!!**********************************
void *handle_routine(void *request_hand){
  //lock acquire before check condition
  pthread_mutex_lock(&work_queue.wqlock);
  printf("%s", "current work queue size is");
  printf("%d", (wq_get_size(&work_queue)));
  // while condition not satisfied 
  while((wq_get_size(&work_queue)<=0)){
      pthread_cond_wait(&work_queue.cond_var, &work_queue.wqlock);
    }
  int request_fd = wq_pop(&work_queue);
  void (*request_handler)(int) = request_hand;
  request_handler(request_fd);
  pthread_mutex_unlock(&work_queue.wqlock);

}



 
 


void init_thread_pool(int num_threads, void (*request_handler)(int)) {
  /*
   * TODO: Part of your solution for Task 2 goes here!
   */

   //malloc thread pool
   int counter = num_threads;
   pthread_t *thread_pool = malloc(num_threads * sizeof(pthread_t));
   //create thread iteratively
   while (counter != 0){
    // create thread and call the handaler
    // working queue is a monitor of the threads?
    //int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                          //void *(*start_routine) (void *), void *arg);
    //int result;
    printf("%s", "creating thread *");
    printf("%s", thread_pool);
    thread_pool++;
    if ( pthread_create(thread_pool, NULL, &handle_routine, (void *) request_handler)){
      printf("%s", "thread_creation failed");
    }
    counter-=1;
   }

}


/*
 * Opens a TCP stream socket on all interfaces with port number PORTNO. Saves
 * the fd number of the server socket in *socket_number. For each accepted
 * connection, calls request_handler with the accepted fd number.
 */
void serve_forever(int *socket_number, void (*request_handler)(int)) {

  struct sockaddr_in server_address, client_address;
  size_t client_address_length = sizeof(client_address);
  int client_socket_number;

  *socket_number = socket(PF_INET, SOCK_STREAM, 0);
  if (*socket_number == -1) {
    perror("Failed to create a new socket");
    exit(errno);
  }

  int socket_option = 1;
  if (setsockopt(*socket_number, SOL_SOCKET, SO_REUSEADDR, &socket_option,
        sizeof(socket_option)) == -1) {
    perror("Failed to set socket options");
    exit(errno);
  }

  memset(&server_address, 0, sizeof(server_address));
  server_address.sin_family = AF_INET;
  server_address.sin_addr.s_addr = INADDR_ANY;
  server_address.sin_port = htons(server_port);

  if (bind(*socket_number, (struct sockaddr *) &server_address,
        sizeof(server_address)) == -1) {
    perror("Failed to bind on socket");
    exit(errno);
  }

  if (listen(*socket_number, 1024) == -1) {
    perror("Failed to listen on socket");
    exit(errno);
  }

  printf("Listening on port %d...\n", server_port);

  init_thread_pool(num_threads, request_handler);

  while (1) {
    client_socket_number = accept(*socket_number,
        (struct sockaddr *) &client_address,
        (socklen_t *) &client_address_length);
    if (client_socket_number < 0) {
      perror("Error accepting socket");
      continue;
    }

    printf("Accepted connection from %s on port %d\n",
        inet_ntoa(client_address.sin_addr),
        client_address.sin_port);

    // TODO: Change me?
    //***********************************Altered here*******************
    // enqueue to work queue 
    printf("%s", "num_threads is ");
    printf("%d", num_threads);
    if (num_threads < 1) {
      // ase in original code
      request_handler(client_socket_number);
      close(client_socket_number);
    } else {
      printf("%s", "in the else case num_thread>1");
      pthread_mutex_lock(&work_queue.wqlock);
      //critical section
      wq_push(&work_queue, client_socket_number);
      printf("%s", "current work queue size is");
      printf("%d", (wq_get_size(&work_queue)));
      pthread_cond_signal(&work_queue.cond_var);
      //critical section ends
      pthread_mutex_unlock(&work_queue.wqlock);
    }

    //***********************************Altered here***********************
    

    printf("Accepted connection from %s on port %d\n",
        inet_ntoa(client_address.sin_addr),
        client_address.sin_port);
  }

  shutdown(*socket_number, SHUT_RDWR);
  close(*socket_number);
}

int server_fd;
void signal_callback_handler(int signum) {
  printf("Caught signal %d: %s\n", signum, strsignal(signum));
  printf("Closing socket %d\n", server_fd);
  if (close(server_fd) < 0) perror("Failed to close server_fd (ignoring)\n");
  exit(0);
}

char *USAGE =
  "Usage: ./httpserver --files www_directory/ --port 8000 [--num-threads 5]\n"
  "       ./httpserver --proxy inst.eecs.berkeley.edu:80 --port 8000 [--num-threads 5]\n";

void exit_with_usage() {
  fprintf(stderr, "%s", USAGE);
  exit(EXIT_SUCCESS);
}

int main(int argc, char **argv) {
  signal(SIGINT, signal_callback_handler);

  /* Default settings */
  server_port = 8000;
  void (*request_handler)(int) = NULL;

  int i;
  for (i = 1; i < argc; i++) {
    if (strcmp("--files", argv[i]) == 0) {
      request_handler = handle_files_request;
      free(server_files_directory);
      server_files_directory = argv[++i];
      if (!server_files_directory) {
        fprintf(stderr, "Expected argument after --files\n");
        exit_with_usage();
      }
    } else if (strcmp("--proxy", argv[i]) == 0) {
      request_handler = handle_proxy_request;

      char *proxy_target = argv[++i];
      if (!proxy_target) {
        fprintf(stderr, "Expected argument after --proxy\n");
        exit_with_usage();
      }

      char *colon_pointer = strchr(proxy_target, ':');
      if (colon_pointer != NULL) {
        *colon_pointer = '\0';
        server_proxy_hostname = proxy_target;
        server_proxy_port = atoi(colon_pointer + 1);
      } else {
        server_proxy_hostname = proxy_target;
        server_proxy_port = 80;
      }
    } else if (strcmp("--port", argv[i]) == 0) {
      char *server_port_string = argv[++i];
      if (!server_port_string) {
        fprintf(stderr, "Expected argument after --port\n");
        exit_with_usage();
      }
      server_port = atoi(server_port_string);
    } else if (strcmp("--num-threads", argv[i]) == 0) {
      char *num_threads_str = argv[++i];
      if (!num_threads_str || (num_threads = atoi(num_threads_str)) < 1) {
        fprintf(stderr, "Expected positive integer after --num-threads\n");
        exit_with_usage();
      }
    } else if (strcmp("--help", argv[i]) == 0) {
      exit_with_usage();
    } else {
      fprintf(stderr, "Unrecognized option: %s\n", argv[i]);
      exit_with_usage();
    }
  }

  if (server_files_directory == NULL && server_proxy_hostname == NULL) {
    fprintf(stderr, "Please specify either \"--files [DIRECTORY]\" or \n"
                    "                      \"--proxy [HOSTNAME:PORT]\"\n");
    exit_with_usage();
  }

  serve_forever(&server_fd, request_handler);

  return EXIT_SUCCESS;
}
