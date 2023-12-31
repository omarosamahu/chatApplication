#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>

#include <netinet/in.h> // To store address information

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include <thread>
#include <fstream>

#include <string>

#define SOCK_NAME "/tmp/DemoSocket"
#define BUFEER_SIZE 128

void TCP_client(){
    std::this_thread::sleep_for(std::chrono::milliseconds(800));
    int network_socket = socket(AF_INET,SOCK_STREAM,0);
    // First parameter refers to socket domain ---> For the current one we use a network socket
    // Second parameter refers to socket type ---> For the current one we use a TCP Connection
    if(network_socket == -1){
        printf("Error with client socket\n");
        return;
    }

    // Now we need to connect
    // But before that we need to specify the address and the port we want to connect to
    sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(8001);
    server_address.sin_addr.s_addr = INADDR_ANY; // inet_addr(_ip.c_str());

    int connection_result = connect(network_socket,(sockaddr*)(&server_address),sizeof(server_address));
    if(connection_result == -1){
        printf("Something is wrong with the connection\n");
    }

    // Now we are ready to receive
    char server_response[256];
    recv(network_socket,&server_response,sizeof(server_response),0);

    printf("Client Data received from server %s\n",server_response);

    if(network_socket != -1){
        close(network_socket);
    }
}

void UNX_client(){
    // std::this_thread::sleep_for(std::chrono::milliseconds(800));
    char buffer[BUFEER_SIZE];
    int network_socket = socket(AF_UNIX,SOCK_STREAM,0);
    // First parameter refers to socket domain ---> For the current one we use a network socket
    // Second parameter refers to socket type ---> For the current one we use a TCP Connection
    if(network_socket == -1){
        printf("Error with client socket\n");
        return;
    }

    // Destory all the sockets with this name
    // unlink(SOCK_NAME);
    // Now we need to connect
    // But before that we need to specify the address and the port we want to connect to
    sockaddr_un server_address;
    server_address.sun_family = AF_UNIX;

    memcpy(server_address.sun_path,SOCK_NAME,sizeof(SOCK_NAME));
    // strncpy(server_address.sun_path,SOCK_NAME,sizeof(SOCK_NAME)-1);

    int connection_result = connect(network_socket,(sockaddr*)(&server_address),sizeof(server_address));
    if(connection_result == -1){
        perror("Error connecting to the server");
        exit(EXIT_FAILURE);
    }

    int i=1;
    int ret;
    while (i != 0)
    {
        printf("Enter the number to send to server :\n");
        scanf("%d",&i);

        // ret = write(network_socket,&i,sizeof(int));
        ret = send(network_socket,&i,sizeof(int),0);
        if(ret == -1){
            perror("Write");
            break;
        }
        printf("Number of bytes send= %d, data sent= %d\n",ret,i);
    }

    memset(buffer,0,BUFEER_SIZE);
/*
    ret = read(network_socket,buffer,sizeof(buffer));
    if(ret == -1){
        perror("read");
        exit(EXIT_FAILURE);
    }
    printf("Recieved data from server: %s\n",buffer);
*/


    

    // Now we are ready to receive
    char server_response[256];
    recv(network_socket,&buffer,sizeof(buffer),0);

    printf("Client Data received from server %s\n",buffer);


    if(network_socket != -1){
        close(network_socket);
    }
}
int main(int argc, char *argv[])
{
    UNX_client();
    return 0;
}
