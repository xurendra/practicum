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
#include <sys/stat.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 2000
// #define SERVER_IP_ADD "10.162.0.4"
#define BUFFER_SIZE 1024

// sanitize filename to keep filename unchanged
void sanitize_filename(char *filename)
{
    int i, j = 0;
    for (i = 0; filename[i] != '\0'; i++)
    {
        if (isprint((unsigned char)filename[i]) || filename[i] == ' ')
        {
            filename[j++] = filename[i];
        }
    }
    filename[j] = '\0';
}

// create directory
void create_directory(const char *path)
{
    struct stat st;
    if (stat(path, &st) == -1)
    {
        // Directory doesn't exist, try to create it
        if (mkdir(path, 0777) == -1)
        {
            perror("Error creating directory");
            exit(EXIT_FAILURE);
        }
    }
    else if (!S_ISDIR(st.st_mode))
    {
        fprintf(stderr, "Error: '%s' is not a directory\n", path);
        exit(EXIT_FAILURE);
    }
}

void handle_write_command(int client_socket, char *command)
{
    char action[BUFFER_SIZE];
    char local_file_path[BUFFER_SIZE];
    char server_file_path[BUFFER_SIZE];
    //    char buffer[BUFFER_SIZE];

    // Parse the command
    sscanf(command, "%s %s %s", action, local_file_path, server_file_path);

    // handle write command
    if (strcmp(action, "WRITE") == 0)
    {

        // Get the current working directory
        char current_directory[BUFFER_SIZE];
        if (getcwd(current_directory, sizeof(current_directory)) == NULL)
        {
            perror("Error obtaining current working directory");
            exit(EXIT_FAILURE);
        }

        printf("path: %s\n", server_file_path);
        printf("Current Working Directory: %s\n", current_directory);

        // Concatenate the current working directory and the provided file path
        char full_server_path[BUFFER_SIZE];
        snprintf(full_server_path, sizeof(full_server_path), "%s/%s", current_directory, server_file_path);

        // check if directory is include in server_file_path
        char *last_slash = strrchr(full_server_path, '/');
        if (last_slash != NULL)
        {
            // extract directory path
            char directory_path[BUFFER_SIZE];
            strncpy(directory_path, full_server_path, last_slash - full_server_path);
            directory_path[last_slash - full_server_path] = '\0';

            // create the directory if it doesn't exist
            create_directory(directory_path);
        }

        // sanitize filename to remove non-printable charcters
        sanitize_filename(full_server_path);

        printf("Full server path %s\n", full_server_path);

        FILE *server_file = fopen(full_server_path, "wb");

        if (server_file == NULL)
        {
            perror("Error opening server file");
            fclose(server_file);
            return;
        }

        // Transfer file contents
        char buffer[BUFFER_SIZE];
        size_t bytesRead;
        while ((bytesRead = recv(client_socket, buffer, sizeof(buffer), 0)) > 0)
        {
            fwrite(buffer, 1, bytesRead, server_file);
        }

        printf("File '%s' transferred from client to server as '%s'\n", local_file_path, server_file_path);

        // fclose(local_file);
        fclose(server_file);
    }
    else
    {
        printf("Invalid command: %s\n", action);
    }
}

int main()
{
    int socket_desc, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_size = sizeof(client_addr);
    char buffer[BUFFER_SIZE];
    // char server_message[8196], client_message[8196];

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
    server_addr.sin_addr.s_addr = INADDR_ANY; // inet_addr(SERVER_IP_ADD);

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
    //  client_size = sizeof(client_addr);
    client_sock = accept(socket_desc, (struct sockaddr *)&client_addr, &client_size);

    if (client_sock < 0)
    {
        printf("Error acception connection\n");
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