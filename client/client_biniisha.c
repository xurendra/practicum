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

//added to provided code
#define BUFFER_SIZE 8196
#define PORT 2000

//int main(void)
int main(int argc, char *argv[]){
   
  //Validate command-line arguments
  if (argc != 4) {
        fprintf(stderr, "Usage: %s WRITE local-file-path remote-file-path\n", argv[0]);
        return -1;
    }

{
  int socket_desc;
  struct sockaddr_in server_addr;
  //char server_message[2000], client_message[2000];
  char server_message[BUFFER_SIZE], client_message[BUFFER_SIZE];

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
  server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  //server_addr.sin_addr.s_addr = inet_addr("129.10.122.39"); //linux Khoury IP address
  //server_addr.sin_addr.s_addr = inet_addr("SERVER_IP_ADDRESS");  

  //added to provided code
 // Set up the WRITE command with paths:
    snprintf(client_message, sizeof(client_message), "WRITE %s %s", argv[2], argv[3]);
  
  // Send connection request to server:
  if(connect(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
    printf("Unable to connect\n");
    return -1;
  }
  printf("Connected with server successfully\n");
  
 //added to provided code 
  // Get input from the user:
//  printf("Enter message: ");
  gets(client_message);
  
  // Send the message to server:
  if(send(socket_desc, client_message, strlen(client_message), 0) < 0){
    printf("Unable to send message\n");
    return -1;
  }
  
  // Receive the server's response:
  if(recv(socket_desc, server_message, sizeof(server_message), 0) < 0){
    printf("Error while receiving server's msg\n");
    return -1;
  }
  
  printf("Server's response: %s\n",server_message);
  
  //if (strncmp(argv[1], "WRITE", 5) == 0) {
        // Open the local file for reading:
        FILE *file = fopen(argv[2], "r");
        if (file == NULL) {
            perror("Error opening local file");
            close(socket_desc); //added line          
            return -1;
        }

        // Send the command to the server:
        if (send(socket_desc, client_message, strlen(client_message), 0) < 0) {
            printf("Unable to send message\n");
            fclose(file);
            close(socket_desc);
            return -1;
        }

        // Send the file name to the server:
        if (send(socket_desc, argv[3], strlen(argv[3]), 0) < 0) {
            printf("Unable to send file name\n");
            fclose(file);
            close(socket_desc);
            return -1;
         }

        // Read and send the file content to the server:
        char buffer[BUFFER_SIZE];
        ssize_t bytes_read;
        while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
            if (send(socket_desc, buffer, bytes_read, 0) < 0) {
                perror("Error sending file data");
                fclose(file);
                close(socket_desc);
                return -1;
            }
        }

        fclose(file);
        printf("File sent to the server: %s\n", argv[3]);
    

  // Close the socket:
  close(socket_desc);
  
  return 0;
  }
}
