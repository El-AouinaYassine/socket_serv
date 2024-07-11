#include <stdio.h>
#include <string.h>
#include <sys/sendfile.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/socket.h>

int main() {
    // Create a socket file descriptor with IPv4 and SOCK_STREAM (TCP)
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("Socket creation failed");
        return 1;
    }

    // Address struct
    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = htons(8080),    // htons to convert port number to network byte order
        .sin_addr.s_addr = INADDR_ANY  // Use any available address
    };

    // Associate the socket with the address created
    if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("Bind failed");
        close(sockfd);
        return 1;
    }

    // Listen for incoming connections with a backlog queue size of 10
    if (listen(sockfd, 10) == -1) {
        perror("Listen failed");
        close(sockfd);
        return 1;
    }

    // Accept an incoming connection
    int acpt = accept(sockfd, NULL, NULL);
    if (acpt == -1) {
        perror("Accept failed");
        close(sockfd);
        return 1;
    }

    // Buffer to receive data
    char buffer[256] = {0};
    if (recv(acpt, buffer, sizeof(buffer), 0) == -1) {
        perror("Receive failed");
        close(acpt);
        close(sockfd);
        return 1;
    }

    // Parse the received data to extract the filename
    char *f = buffer + 5;
    *strchr(f, ' ') = 0;

    // Open the file in read-only mode
    int open_fd = open(f, O_RDONLY);
    if (open_fd == -1) {
        perror("File open failed");
        close(acpt);
        close(sockfd);
        return 1;
    }

    // Get the file size
    off_t file_size = lseek(open_fd, 0, SEEK_END);
    lseek(open_fd, 0, SEEK_SET);  // Reset the file offset to the beginning

    // Send the file
    if (sendfile(acpt, open_fd, NULL, file_size) == -1) {
        perror("Sendfile failed");
        close(open_fd);
        close(acpt);
        close(sockfd);
        return 1;
    }

    // Close file and sockets
    close(open_fd);
    close(acpt);
    close(sockfd);

    return 0;
}
