#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>


#define PORT 8080

void *handle_client(void* arg);

int main(int argc, char* argv[])
{
    int server_fd;
    struct sockaddr_in server_addr;
    //create server socket
    if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket Failed");
        exit(EXIT_FAILURE);

    }
    //config socket
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    //bind socket to port
    if(bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Bind Failed");
        exit(EXIT_FAILURE);
    }

    //listen for connections
    if(listen(server_fd, 10) < 0)
    {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }
    while(1)
    {
        //client info
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int *client_fd = malloc(sizeof(int));

        //accept client connection
        if((*client_fd = accept(server_fd, struct sockaddr *)&client_addr, &client_addr_len)) < 0)
        {
            perror("accept failed");
            continue;
        }

        pthread_t thread_id;
        pthread_create(&thread_id, NULL, handle_client, (void *)client_fd);
        pthread_detach(thread_id);
    }
    return 0;
}


void *handle_client(void* arg)
{
    int client_fd = *((int *)arg);
    char *buffer = (char *)malloc(BUFFER_SIZE * sizeof(char));

    // receive requst data from client and store into buffer
    ssize_t bytes_received = recv(client_fd, buffer, BUFFER_SIZE, 0);
    if(bytes_received > 0)
    {
        // check if request is GET
        regex_t regex;
        regcomp(&regex, "^GET /([^ ]*) HTTP/1", REG_EXTENDED);
        regmatch_t matches[2];

        if(regexec(&regex, buffer, 2, matches, 0) == 0)
        {
            // extract filename from request and decode URL
            buffer[matches[1].rm_eo] = '\0';
            const char *url_encoded_file_name = buffer + matches[1].rm_so;
            char *file_name = url_decode(url_encoded_file_name);

            // get filename
            char file_ext[32];
            strcpy(file_ext, get_file_extension(file_name));

            // build HTTP response
            char *response = (char *)malloc(BUFFER_SIZE * 2 * sizeof(char));
            size_t repsponse_len;
            build_http_response(file_name, file_ext, response, &repsponse_len);

            // send HTTP response to client
            send(clident_fd, response, response_len, 0);

            free(response);
            free(file_name);
        }
        regfree(&regex);
    }
}
