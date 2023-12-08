/*
 * server.c -- TCP Socket Server
 *
 * adapted from:
 *   https://www.educative.io/answers/how-to-implement-tcp-sockets-in-c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 2000
#define SERVER_IP_ADD "10.162.0.4"
#define BUFFER_SIZE 1024

void handle_write_command(int client_socket, char *buffer)
{
    char action[BUFFER_SIZE];
    char local_file_path[BUFFER_SIZE];
    char server_file_path[BUFFER_SIZE];

    // Parse the command
    sscanf(buffer, "%s %s %s", action, local_file_path, server_file_path);

    if (strcmp(action, "WRITE") == 0)
    {
        FILE *local_file = fopen(local_file_path, "rb");

        if (local_file == NULL)
        {
            perror("Error opening local file");
            return;
        }

        FILE *server_file = fopen(server_file_path, "wb");

        if (server_file == NULL)
        {
            perror("Error opening server file");
            fclose(local_file);
            return;
        }

        // Transfer file contents
        size_t bytesRead;
        while ((bytesRead = fread(buffer, 1, sizeof(buffer), local_file)) > 0)
        {
            fwrite(buffer, 1, bytesRead, server_file);
        }

        printf("File '%s' transferred from client to server as '%s'\n", local_file_path, server_file_path);

        fclose(local_file);
        fclose(server_file);
    }
    else
    {
        printf("Invalid command: %s\n", action);
    }
}

int main(void)
{
    int socket_desc, client_sock;
    socklen_t client_size;
    struct sockaddr_in server_addr, client_addr;
    char buffer[BUFFER_SIZE] = {0};

    // Clean buffers:
    // memset(server_message, '\0', sizeof(server_message));
    // memset(client_message, '\0', sizeof(client_message));

    // Create socket:
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_desc < 0)
    {
        printf("Error while creating socket\n");
        return -1;
    }
    printf("Socket created successfully\n");

    // Set port and IP:
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP_ADD);

    // Bind to the set port and IP:
    if (bind(socket_desc, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        printf("Couldn't bind to the port\n");
        return -1;
    }
    printf("Done with binding\n");

    // Listen for clients:
    if (listen(socket_desc, 1) < 0)
    {
        printf("Error while listening\n");
        return -1;
    }
    printf("\nListening for incoming connections.....\n");

    // Accept an incoming connection:
    client_size = sizeof(client_addr);
    client_sock = accept(socket_desc, (struct sockaddr *)&client_addr, &client_size);

    if (client_sock < 0)
    {
        printf("Can't accept\n");
        return -1;
    }
    printf("Client connected at IP: %s and port: %i\n",
           inet_ntoa(client_addr.sin_addr),
           ntohs(client_addr.sin_port));

    // Receiving data from client
    if (recv(client_sock, buffer, BUFFER_SIZE, 0) == -1)
    {
        perror("Error receiving data");
        exit(EXIT_FAILURE);
    }

    printf("Received data from client: %s\n", buffer);

    // Handle WRITE command
    handle_write_command(client_sock, buffer);

    // Closing the socket:
    close(client_sock);
    close(socket_desc);

    return 0;
}
