include<stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pthread.h>
#include <fcntl.h>
#include <ctype.h>

#define PORT 2000
#define MAX_CONNECTIONS 5
#define BUFFER_SIZE 1024

    pthread_mutex_t file_lock = PTHREAD_MUTEX_INITIALIZER;

void ensure_directory_exists(const char *path)
{
    struct stat st;
    if (stat(path, &st) == -1)
    {
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

void receive_file(int client_socket, char *server_file_path)
{
    FILE *server_file;

    char current_working_directory[BUFFER_SIZE];
    if (getcwd(current_working_directory, sizeof(current_working_directory)) == NULL)
    {
        perror("Error obtaining current working directory");
        exit(EXIT_FAILURE);
    }

    char full_server_file_path[BUFFER_SIZE];
    snprintf(full_server_file_path, sizeof(full_server_file_path), "%s/%s", current_working_directory, server_file_path);

    char directory_path[BUFFER_SIZE];
    strncpy(directory_path, full_server_file_path, strrchr(full_server_file_path, '/') - full_server_file_path);
    directory_path[strrchr(full_server_file_path, '/') - full_server_file_path] = '\0';

    ensure_directory_exists(directory_path);
    sanitize_filename(full_server_file_path);

    pthread_mutex_lock(&file_lock);

    server_file = fopen(full_server_file_path, "ab");

    if (server_file == NULL)
    {
        perror("Error opening server file");
        pthread_mutex_unlock(&file_lock);
        exit(EXIT_FAILURE);
    }

    char buffer[BUFFER_SIZE];
    size_t bytesRead;

    while ((bytesRead = recv(client_socket, buffer, sizeof(buffer), 0)) > 0)
    {
        fwrite(buffer, 1, bytesRead, server_file);
    }

    fclose(server_file);
    pthread_mutex_unlock(&file_lock);
}

void *handle_client(void *socket_ptr)
{
    int client_socket = *(int *)socket_ptr;
    free(socket_ptr); // Free the memory allocated for the socket file descriptor

    char buffer[BUFFER_SIZE];
    if (recv(client_socket, buffer, sizeof(buffer), 0) == -1)
    {
        perror("Error receiving command");
        close(client_socket);
        pthread_exit(NULL);
    }

    printf("Received command from client: %s\n", buffer);

    char action[BUFFER_SIZE];
    char local_file_path[BUFFER_SIZE];
    char server_file_path[BUFFER_SIZE];

    sscanf(buffer, "%s %s %s", action, local_file_path, server_file_path);

    if (strcmp(action, "WRITE") == 0)
    {
        receive_file(client_socket, server_file_path);
        printf("File received and saved on the server as '%s'\n", server_file_path);
    }
    else
    {
        printf("Invalid command: %s\n", action);
    }

    close(client_socket);
    pthread_exit(NULL);
}

int main()
{
    int server_socket, new_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size = sizeof(client_addr);

    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("Error binding socket");
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, MAX_CONNECTIONS) == -1)
    {
        perror("Error listening");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    while (1)
    {
        if ((new_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_size)) == -1)
        {
            perror("Error accepting connection");
            continue;
        }

        printf("Connection accepted from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        // Create a new thread to handle the client
        pthread_t thread;
        int *socket_ptr = malloc(sizeof(int));
        *socket_ptr = new_socket;

        if (pthread_create(&thread, NULL, handle_client, (void *)socket_ptr) != 0)
        {
            perror("Error creating thread");
            close(new_socket);
            free(socket_ptr);
        }

        // Detach the thread to allow its resources to be automatically released when it exits
        pthread_detach(thread);
    }

    close(server_socket);

    return 0;
}