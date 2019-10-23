/**
 * webserver.c -- A webserver written in C
 * 
 * Test with curl (if you don't have it, install it):
 * 
 *    curl -D - http://localhost:3490/
 *    curl -D - http://localhost:3490/d20
 *    curl -D - http://localhost:3490/date
 * 
 * You can also test the above URLs in your browser! They should work!
 * 
 * Posting Data:
 * 
 *    curl -D - -X POST -H 'Content-Type: text/plain' -d 'Hello, sample data!' http://localhost:3490/save
 * 
 * (Posting data is harder to test from a browser.)
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/file.h>
#include <fcntl.h>
#include "net.h"
#include "file.h"
#include "mime.h"
#include "cache.h"
#include <sys/epoll.h>
//#define __DEBUG__

#ifdef  __DEBUG__
#define DEBUG(format, ...) printf(format, ##__VA_ARGS__)
#else
#define DEBUG(format, ...)
#endif


#define PORT "3490"  // the port users will be connecting to

#define SERVER_FILES "./serverfiles"
#define SERVER_ROOT "./serverroot"


/**
 * Send an HTTP response
 *
 * header:       "HTTP/1.1 404 NOT FOUND" or "HTTP/1.1 200 OK", etc.
 * content_type: "text/plain", etc.
 * body:         the data to send.
 * 
 * Return the value from the send() function.
 */
int send_response(int fd, char *header, char *content_type, void *body, int content_length)
{
    const int max_response_size = 262144;
    char response[max_response_size];

    // Build HTTP response and store it in response

    ///////////////////
    // IMPLEMENT ME! //
    ///////////////////
    
    
    DEBUG("body is %s\n", body);

    int response_length = sprintf(response , "%s\r\nContent-Type: %s\r\nContent-Length: %d\r\nConnection: close\r\n\r\n%s\r\n",
                       header, content_type, content_length, body);
    /*
    int response_length = sprintf(response , "%s\r\nContent-Type: %s\r\nContent-Length: %d\r\n%s\r\n",
                       header, content_type, content_length, body);
    */
    // Send it all!
    int rv = send(fd, response, response_length, 0);

    if (rv < 0) {
        perror("send");
    }

    return rv;
}


/**
 * Send a /d20 endpoint response
 */
void get_d20(int fd)
{
    // Generate a random number between 1 and 20 inclusive
    
    ///////////////////
    // IMPLEMENT ME! //
    ///////////////////

    DEBUG("now in get_d20\n");

    int random_number = rand() % 20 + 1;

    DEBUG("random number is %d\n", random_number);


    char number[255];

    int number_length = sprintf(number, "%d", random_number);
    DEBUG("number(char) = %s\n", number);
    // Use send_response() to send it back as text/plain data
    send_response(fd, "HTTP 200 OK", "text/plain", number, number_length);
    ///////////////////
    // IMPLEMENT ME! //
    ///////////////////
}

/**
 * Send a 404 response
 */
void resp_404(int fd)
{
    char filepath[4096];
    struct file_data *filedata; 
    char *mime_type;

    // Fetch the 404.html file
    snprintf(filepath, sizeof filepath, "%s/404.html", SERVER_FILES);
    filedata = file_load(filepath);

    if (filedata == NULL) {
        // TODO: make this non-fatal
        fprintf(stderr, "cannot find system 404 file\n");
        exit(3);
    }

    mime_type = mime_type_get(filepath);

    send_response(fd, "HTTP/1.1 404 NOT FOUND", mime_type, filedata->data, filedata->size);

    file_free(filedata);
}

/**
 * Read and return a file from disk or cache
 */
void get_file(int fd, struct cache *cache, char *request_path)
{
    ///////////////////
    // IMPLEMENT ME! //
    ///////////////////


    (void)cache;

    struct file_data *filedata;
    char *mime_type;
    char filename[255];
    strcpy(filename, SERVER_ROOT);
    strcat(filename, request_path);
    filedata = file_load(filename);
    if (filedata == NULL) {
        resp_404(fd);
    }
    else {
        mime_type = mime_type_get(request_path);
        send_response(fd, "HTTP/1.1 200 OK", mime_type, filedata->data, filedata->size);
    }
    file_free(filedata);
}

/**
 * Search for the end of the HTTP header
 * 
 * "Newlines" in HTTP can be \r\n (carriage return followed by newline) or \n
 * (newline) or \r (carriage return).
 */
char *find_start_of_body(char *header)
{
    ///////////////////
    // IMPLEMENT ME! // (Stretch)
    ///////////////////

    (void)header;
    return NULL;
}

/**
 * Handle HTTP request and send response
 */
void handle_http_request(int fd, struct cache *cache)
{

    DEBUG("now in handle\n");

    (void)cache;

    const int request_buffer_size = 65536; // 64K
    char request[request_buffer_size];

    // Read request
    int bytes_recvd = recv(fd, request, request_buffer_size - 1, 0);

    if (bytes_recvd < 0) {
        perror("recv");
        return;
    }


    ///////////////////
    // IMPLEMENT ME! //
    ///////////////////
    char method[255];
    char path[255];
    sscanf(request, "%s %s", method, path);

    DEBUG("%s %s\n", method, path);

    DEBUG("111111111\n");


    if (strcmp(path, "/d20") == 0) {

        DEBUG("path = /d20\n");

        get_d20(fd);
    }
    else if (strcmp(method, "GET") == 0) {

        DEBUG("method is get\n");

        if (strcmp(path, "/favicon.ico") == 0) {
            return;
        }

        get_file(fd, cache, path);
    }
    else {

        DEBUG("path wrong\n");

        resp_404(fd);
    }
    // Read the first two components of the first line of the request 
 
    // If GET, handle the get endpoints

    //    Check if it's /d20 and handle that special case
    //    Otherwise serve the requested file by calling get_file()


    // (Stretch) If POST, handle the post request
}



#define MAX_EVENT_NUM 1024

void addfd(int epfd, int fd) {
    struct epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN;
    epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &event);
}

void delfd(int epfd, int fd) {
    struct epoll_event event;
    epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &event);
}

/**
 * Main
 */
int main(void)
{
    int newfd;  // listen on sock_fd, new connection on newfd
    int i;

    struct epoll_event events[MAX_EVENT_NUM];
    int epfd;
    epfd = epoll_create(5);

    struct sockaddr_storage their_addr; // connector's address information
    char s[INET6_ADDRSTRLEN];

    struct cache *cache = cache_create(10, 0);

    // Get a listening socket
    int listenfd = get_listener_socket(PORT);

    if (listenfd < 0) {
        fprintf(stderr, "webserver: fatal error getting listening socket\n");
        exit(1);
    }

    printf("webserver: waiting for connections on port %s...\n", PORT);

    addfd(epfd, listenfd);

    // This is the main loop that accepts incoming connections and
    // responds to the request. The main parent process
    // then goes back to waiting for new connections.
    
    while(1) {

        DEBUG("nfds = %d\n", nfds);

        int rv = epoll_wait(epfd, events, MAX_EVENT_NUM, -1);
        if (rv < 0) {
            perror("epoll");
        }
        else if (rv == 0) {
            printf("timeout\n");
        }
        else {

            for (i = 0; i < rv; i++) {
                int sockfd = events[i].data.fd;
                if (sockfd == listenfd) {
                    socklen_t sin_size = sizeof their_addr;

                    // Parent process will block on the accept() call until someone
                    // makes a new connection:
                    newfd = accept(listenfd, (struct sockaddr *)&their_addr, &sin_size);
                    if (newfd == -1) {
                        perror("accept");
                        continue;
                    }

                    // Print out a message that we got the connection
                    inet_ntop(their_addr.ss_family,
                        get_in_addr((struct sockaddr *)&their_addr),
                        s, sizeof s);
                    printf("server: got connection from %s\n", s);

                    addfd(epfd, newfd);

                }
                else if (events[i].events & EPOLLIN) {  //newfd
                    handle_http_request(sockfd, cache);
                    close(sockfd);
                    //delfd(epfd, sockfd);
                }
            }
        }
    }
    close(listenfd);
    // Unreachable code

    return 0;
}

