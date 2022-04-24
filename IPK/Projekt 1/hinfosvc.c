#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>

#include "hinfosvc.h"

#define MAX_RESPONSE_LENGTH 100

#define MAX_HOST_LENGTH 50


char* generate_not_found()
{
    char* response = 
    "HTTP/1.1 404 Not found\n"
    "Connection: Closed\n"
    "\n";
    return response;

}

char* get_url(char* buffer)
{

    int n = strlen(buffer);
    int i = 0, j = 0;

    char* temp = (char*)malloc(sizeof(buffer) + 1);

    while (i < n && buffer[i] != ' ') {
        ++i;
    }

    ++i;
    j = 0;
    while (i < n && buffer[i] != ' ') {
            temp[j] = buffer[i];
        ++i, ++j;
    }
    temp[j] = '\0';

    return temp;
}

char* process_url(char* url)
{
    if (strcmp(url, "/hostname") == 0) {
            char hostname[MAX_HOST_LENGTH];
            hostname[MAX_HOST_LENGTH] = '\0';
            gethostname(hostname, MAX_HOST_LENGTH);

            char* header = generate_200_ok();

            char *response = malloc (sizeof (char) * MAX_RESPONSE_LENGTH);

            strcat(response, header);
            
            strcat(response, hostname);
            return response;
    }
    else if (strcmp(url, "/cpu-name") == 0) {

        FILE *fp;
        int status;
        char *path = malloc (sizeof (char) * MAX_HOST_LENGTH);
        char *response = malloc (sizeof (char) * MAX_RESPONSE_LENGTH);

        fp = popen("cat /proc/cpuinfo | grep 'model name' | head -n 1", "r");
        if (fp == NULL) {
            fprintf(stderr, "Unable to open stream");
        }   

        fgets(path, 1000, fp);

        status = pclose(fp);

        char* header = generate_200_ok();

        strcat(response, header);
        strcat(response, path);
        return response;
    }
    else if (strcmp(url, "/load") == 0) {
        FILE *fp;
        int status;
        char *path = malloc (sizeof (char) * MAX_HOST_LENGTH);
        char *response = malloc (sizeof (char) * 1000);

        fp = popen("grep 'cpu ' /proc/stat | awk '{usage=($2+$4)*100/($2+$4+$5)} END {print usage \"%\"}'", "r");
        if (fp == NULL) {
            fprintf(stderr, "Unable to open stream");
        }   

        fgets(path, 1000, fp);
        
        status = pclose(fp);

        char* header = generate_200_ok();

        strcat(response, header);
        strcat(response, path);
        return response;
    }
    else {
        return generate_not_found();
    }

}


int main(int argc, char const *argv[])
{
    int server_fd, new_socket;
    long valread;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    if (argc != 2) {
        fprintf(stderr, "Wrong number of arguments!\n");
        exit(1);
    }

    int port = atoi(argv[1]);

    if (port == 0) {
        fprintf(stderr, "Wrong number of arguments!\n");
        exit(1);
    }
    
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("In socket");
        exit(EXIT_FAILURE);
    }
    

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( port );
    
    memset(address.sin_zero, '\0', sizeof address.sin_zero);
    
    
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0)
    {
        perror("In bind");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 10) < 0)
    {
        perror("In listen");
        exit(EXIT_FAILURE);
    }
    while(1)
    {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
        {
            perror("In accept");
            exit(EXIT_FAILURE);
        }
        
        char buffer[30000] = {0};
        valread = read( new_socket , buffer, 30000);
        char* url = get_url(buffer);

        char* r = process_url(url);

        write(new_socket , r , strlen(r));;
        close(new_socket);
    }
    return 0;
}

char* generate_200_ok()
{
    char *header = 
    "HTTP/1.1 200 OK\n"
    "Content-Length: 88\n"
    "Content-Type: text/plain\n"
    "Connection: Closed\n"
    "\n";

    return header;
}