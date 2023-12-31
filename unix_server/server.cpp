

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

#define SOCKET_NAME   "/tmp/DemoSocket"
#define BUFFER_SIZE 128


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

    int network_socket = socket(AF_UNIX,SOCK_STREAM,0);
    // First parameter refers to socket domain ---> For the current one we use a network socket
    // Second parameter refers to socket type ---> For the current one we use a TCP Connection
    if(network_socket == -1){
        printf("Error with client socket\n");
        return;
    }

    // Destory all the sockets with this name
    unlink(SOCKET_NAME);
    // Now we need to connect
    // But before that we need to specify the address and the port we want to connect to
    sockaddr_un server_address;
    server_address.sun_family = AF_UNIX;

    memcpy(server_address.sun_path,SOCKET_NAME,sizeof(SOCKET_NAME));

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

void server(){
    int ret=0;

    char buffer[BUFFER_SIZE];
    int item = 0;
    int result = 0;
    int data_socket;
    unlink(SOCKET_NAME);


    int server_socket = socket(AF_UNIX,SOCK_STREAM,0);
    if(server_socket == -1){
        printf("Error with server socket\n");
        return;
    }

    sockaddr_un server_address;
    server_address.sun_family = AF_UNIX;
    memcpy(server_address.sun_path,SOCKET_NAME,sizeof(SOCKET_NAME));
    // strncpy(server_address.sun_path,SOCKET_NAME,sizeof(SOCKET_NAME)-1);

    ret = bind(server_socket,(const struct sockaddr*)(&server_address),sizeof(server_address)); // Number of bytes recieved by the sender
    if(ret == -1){
        perror("Error binding socket");
        exit(EXIT_FAILURE);
    }

    // We tell the OS to queue a number of clients with 4 and we going to handle
    // This number otherwise it will drop the extra clients the sends the data
    ret = listen(server_socket,10);
    if(ret == -1){
        perror("Error with listen function");
        exit(EXIT_FAILURE);
    }
    for(;;){
        // Blocking will happen here because the server is waiting for a client to connect
        data_socket = accept(server_socket,NULL,NULL); // Client handler will be established
        if(ret == -1){
            printf("Error Server socket for accept system call: %d", data_socket);
            return;
        }
        printf("Connection accepted from the client: %d\n",data_socket);
        for(;;){
            memset(buffer,0,sizeof(buffer));
            // Wait for next data packet
            ret = read(data_socket,buffer,sizeof(buffer));
            if(ret == -1){
                perror("read");
                exit(EXIT_FAILURE);
            }

            memcpy(&item,buffer,sizeof(int));
            if(item == 0) break;
            result += item;
        }
        memset(buffer,0,sizeof(buffer));
        sprintf(buffer,"Result %d",result);
        ret = write(data_socket,buffer,sizeof(buffer));
        if(ret == -1){
            perror("write");
            exit(EXIT_FAILURE);
        }
        // std::this_thread::sleep_for(std::chrono::milliseconds(500));
        close(data_socket);
    }

    if(server_socket != -1){
        close(server_socket);
    }

    unlink(SOCKET_NAME);
//    send(client_socket,result,sizeof(int),0);
}

int main(int argc, char *argv[])
{
    server();
    return 0;
}
