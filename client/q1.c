/*
 * client.c -- TCP Socket Client
 * 
 * adapted from: 
 *   https://www.educative.io/answers/how-to-implement-tcp-sockets-in-c
 */

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 2000
#define SERVER_IP_ADD "10.162.0.4"
#define BUFFER_SIZE 1024

void write_file(int socket, char* client_file_path, char* server_file_path) {
    char command[BUFFER_SIZE];

    // Create WRITE command
    snprintf(command, BUFFER_SIZE, "WRITE %s %s", client_file_path, server_file_path);

    // Send data to server
    if (send(socket, command, strlen(command), 0) == -1) {
        printf("Error sending file");
        exit(EXIT_FAILURE);
    }

    printf("Sent WRITE command to server\n");
}

int main(void)
{
  int socket_desc;
  struct sockaddr_in server_addr;
  char server_message[2000], client_message[2000];
  
  // Clean buffers:
  memset(server_message,'\0',sizeof(server_message));
  memset(client_message,'\0',sizeof(client_message));
  
  // Create socket:
  socket_desc = socket(AF_INET, SOCK_STREAM, 0);
  
  if(socket_desc < 0){
    printf("Unable to create socket\n");
    return -1;
  }
  
  printf("Socket created successfully\n");
  
  // Set port and IP the same as server-side:
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(PORT);
  server_addr.sin_addr.s_addr = inet_addr(SERVER_IP_ADD);
  
  // Send connection request to server:
  if(connect(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
    printf("Unable to connect\n");
    return -1;
  }
  printf("Connected with server successfully\n");

  char* client_file_path = argv[2];
  char* server_file_path = (argc ==3) ? argv[3] : client_file_path;
  write_file(socket_desc, client_file_path, server_file_path);
  
  // Close the socket:
  close(socket_desc);
  
  return 0;
}
