#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>

#define PORT 8080

int main(int argc, char const *argv[])
{
    int server_fd, new_socket, bytes_read;
    struct sockaddr_in address;
    struct sockaddr_in clnt_address;
    int clnt_addrlen = sizeof(clnt_address);
    const int BUF_LEN = 256;
    char buffer[BUF_LEN];

    memset(buffer, 0, BUF_LEN);

    /* Create socket file descriptor */
    if ((server_fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(PORT);

    // Attach socket to the port 8080
    if (bind(server_fd, (struct sockaddr *) &address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    if ((new_socket = accept(server_fd, (struct sockaddr *) &clnt_address, (socklen_t*) &clnt_addrlen)) < 0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    /* Read network pipe data */
    while ((bytes_read = read(new_socket, buffer, BUF_LEN)) > 0) {
        printf("%s", buffer);
    }
    printf("\n");

    return 0;
}
