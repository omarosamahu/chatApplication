

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
#include <cmath>

#define SOCKET_NAME          "/tmp/DemoSocket"
#define BUFFER_SIZE          128
#define MAX_CLIENT_SUPPORTED 32

int monitored_fd_set[MAX_CLIENT_SUPPORTED];
int client_result[MAX_CLIENT_SUPPORTED] = {0};

static void intialize_monitored_fd_set(){
    for(size_t i=0;i<MAX_CLIENT_SUPPORTED;++i){
        monitored_fd_set[i] = -1;
    }
}

static void add_monitored_fd_set(int fd){
    for(size_t i=0;i<MAX_CLIENT_SUPPORTED;++i){
        if(monitored_fd_set[i] != -1)
            continue;
        monitored_fd_set[i] = fd;
        break;
    }
}

static void remove_monitored_fd_set(int fd){
    for(size_t i=0;i<MAX_CLIENT_SUPPORTED;++i){
        if(monitored_fd_set[i] != fd)
            continue;
        monitored_fd_set[i] = -1;
        break;
    }
}

// This will clone all the file discriptors to the fd set
static void refresh_fd_set(fd_set * pFdSet){
    FD_ZERO(pFdSet); // Remove all the old file discriptors from the fd_set
    for(size_t i=0;i<MAX_CLIENT_SUPPORTED;++i){
        if(monitored_fd_set[i] != -1){
            FD_SET(monitored_fd_set[i],pFdSet);
        }
    }
}

static int getMaxFd(){
    int max = -1;
    for(size_t i=0;i<MAX_CLIENT_SUPPORTED;++i){
        max = std::max(max,monitored_fd_set[i]);
    }
    return max;
}

void server(){
    int ret=0;

    char buffer[BUFFER_SIZE];
    int item = 0;
    int result = 0;
    int data_socket;
    fd_set readFds;

    intialize_monitored_fd_set();
    // Delete all the instances for this socket
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
    //Add master file socket file discriptor
    add_monitored_fd_set(server_socket);
    for(;;){
        refresh_fd_set(&readFds);
        printf("Waiting on select()\n");
        select(getMaxFd() + 1,&readFds, NULL, NULL, NULL); // Blocking
        // Blocking will happen here because the server is waiting for a client to connect
        if(FD_ISSET(server_socket,&readFds)){
            data_socket = accept(server_socket,NULL,NULL); // Client handler will be established
            if(data_socket == -1){
                perror("accept");
                exit(EXIT_FAILURE);
            }
            printf("Connection accepted from the client\n");
            add_monitored_fd_set(data_socket);
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
