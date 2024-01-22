SERVER

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <fcntl.h>

#define SERVER_PORT 8080
#define FILE_TO_SEND "example.txt"
#define MAX_BUFFER_SIZE 1024

void send_file(int sockfd)
{
    int file_fd = open(FILE_TO_SEND, O_RDONLY);
    if (file_fd == -1)
    {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    struct stat file_stat;
    if (fstat(file_fd, &file_stat) == -1)
    {
        perror("Error getting file information");
        close(file_fd);
        exit(EXIT_FAILURE);
    }

    dprintf(sockfd, "HTTP/1.1 200 OK\r\nContent-Length: %ld\r\n\r\n", file_stat.st_size);

    char buffer[MAX_BUFFER_SIZE];
    ssize_t bytesRead;

    while ((bytesRead = read(file_fd, buffer, sizeof(buffer))) > 0)
    {
        send(sockfd, buffer, bytesRead, 0);
    }

    close(file_fd);
}

int main()
{
    int sockfd, confd;
    struct sockaddr_in servaddr, cliaddr;
    socklen_t addr_len = sizeof(cliaddr);

    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(SERVER_PORT);

    // Bind
    if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1)
    {
        perror("Error binding");
        exit(EXIT_FAILURE);
    }

    // Listen
    if (listen(sockfd, 1) == -1)
    {
        perror("Error listening");
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port %d...\n", SERVER_PORT);

    // Accept
    confd = accept(sockfd, (struct sockaddr *)&cliaddr, &addr_len);
    if (confd == -1)
    {
        perror("Error accepting connection");
        exit(EXIT_FAILURE);
    }

    // Send file over HTTP
    send_file(confd);

    // Close sockets
    close(confd);
    close(sockfd);

    return 0;
}

