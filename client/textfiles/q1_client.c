/*
 * client.c -- TCP Socket Client
 *
 * adapted from:
 *   https://www.educative.io/answers/how-to-implement-tcp-sockets-in-c
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 2000
#define SERVER_IP_ADD "10.162.0.4"
#define BUFFER_SIZE 1024

void send_write_command(int socket, char *client_file_path, char *server_file_path)
{
    char command[BUFFER_SIZE];

    // Create WRITE command
    snprintf(command, BUFFER_SIZE, "WRITE %s %s ", client_file_path, server_file_path);

    // Send data to server
    if (send(socket, command, strlen(command), 0) == -1)
    {
        printf("Error sending file");
        exit(EXIT_FAILURE);
    }
    printf("Sent WRITE command to server with local file path: %s and server file path: %s\n", client_file_path, server_file_path);
}

void send_file(int client_socket, char *client_file_path)
{
    FILE *client_file = fopen(client_file_path, "rb");

    if (client_file == NULL)
    {
        perror("Error opening client file");
        exit(EXIT_FAILURE);
    }

    char buffer[BUFFER_SIZE];
    size_t bytesRead;

    while ((bytesRead = fread(buffer, 1, sizeof(buffer), client_file)) > 0)
    {
        if (send(client_socket, buffer, bytesRead, 0) == -1)
        {
            perror("Error sending file data");
            fclose(client_file);
            exit(EXIT_FAILURE);
        }
    }
    fclose(client_file);
}

int main(int argc, char *argv[])
{
    if (argc < 3 || argc > 4)
    {
        fprintf(stderr, "Usage: %s WRITE client_file_path [server_file_path]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    int socket_desc;
    struct sockaddr_in server_addr;
    // char server_message[2000], client_message[2000];

    // Clean buffers:
    // memset(server_message,'\0',sizeof(server_message));
    // memset(client_message,'\0',sizeof(client_message));

    // Create socket:
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_desc < 0)
    {
        printf("Unable to create socket\n");
        return -1;
    }

    printf("Socket created successfully\n");

    // Set port and IP the same as server-side:
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP_ADD);

    // Send connection request to server:
    if (connect(socket_desc, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        printf("Unable to connect\n");
        return -1;
    }
    printf("Connected with server successfully\n");

    char *client_file_path = argv[2];
    char *server_file_path = (argv[3] != NULL) ? argv[3] : client_file_path;

    printf("Sent client path: %s, server path %s\n", client_file_path, server_file_path);

    send_write_command(socket_desc, client_file_path, server_file_path);

    send_file(socket_desc, client_file_path);

    // Close the socket:
    close(socket_desc);

    return 0;
}