#include "socket_server.hpp"

#include <unistd.h> // close
#include <sys/types.h>
#include <sys/stat.h>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>

static const int MAXPENDING = 5; // Maximum outstanding connection requests

SocketServer::SocketServer(int port) :
    image_dims_(cv::Size2i(0, 0)),
    client_len_(0),
    server_addr_size_(sizeof(server_addr_)),
    port_(port),
    sock_fdesc_init_(0),
    sock_fdesc_conn_(0) { client_len_ = server_addr_size_; }

void SocketServer::ConnectToNetwork() {
    // Initialize Socket
    sock_fdesc_init_ = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fdesc_init_ == -1) {
        close(sock_fdesc_init_);
        perror("Couldn't create socket!\n");
        exit(1);
    }

    // Zero out server address struct
    memset((char*)&server_addr_, 0, server_addr_size_);

    // Set server address struct
    server_addr_.sin_family = AF_INET;
    server_addr_.sin_addr.s_addr = INADDR_ANY;
    server_addr_.sin_port = htons(port_);

    // Assign server address to initial socket file descriptor
    if (bind(sock_fdesc_init_, (struct sockaddr*)&server_addr_, server_addr_size_) == -1) {
        perror("Couldn't bind initial socket file descriptor!");
        printf("Trying again after killing existing process on port %d...\n", port_);
        close(sock_fdesc_init_);
        
        if (bind(sock_fdesc_init_, (struct sockaddr*)&server_addr_, server_addr_size_) == -1) {
            perror("Couldn't bind initial socket file descriptor after retry!");
            exit(1);
        }
        
        printf("Successful bind to port %d after killing previous process\n", port_);
    }

    // Enable listening on initial socket file descriptor
    // Mark the socket so it will listen for incoming connections
    if (listen(sock_fdesc_init_, MAXPENDING) < 0){
        perror("listen() failed");
        exit(1);
    }

    // Block process until connection with client has been established.
    // 'client_fdesc' set as new file descriptor to be used for communication
    sock_fdesc_conn_ = accept(sock_fdesc_init_, (struct sockaddr*)&client_addr_, &client_len_);
    if (sock_fdesc_conn_ == -1) {
        perror("ERROR! Client couldn't connect!");
        exit(1);
    }
}

void SocketServer::ReceiveImageDims() {
    ssize_t bytes_sent = 0;
    size_t dims_size = 0;

    int cols = 0;
    int rows = 0;

    size_t sizeof_dims = sizeof(image_dims_.height);

    if ((bytes_sent = recv(sock_fdesc_conn_, (char*)&cols, sizeof_dims, 0)) == -1) {
        printf("ERROR!: recv failed\n"
                "sock_fdesc: %d\n"
                "image_size: %zu\n"
                "bytes_sent: %zu\n", sock_fdesc_conn_, dims_size, bytes_sent);
        exit(1);
    }
    
    if ((bytes_sent = recv(sock_fdesc_conn_, (char*)&rows, sizeof_dims, 0)) == -1) {
        printf("ERROR!: recv failed\n"
                "sock_fdesc: %d\n"
                "image_size: %zu\n"
                "bytes_sent: %zu\n", sock_fdesc_conn_, dims_size, bytes_sent);
        exit(1);
    }
    image_dims_ = cv::Size2i(cols, rows);
    printf("Image dimensions: [%dx%d]\n", cols, rows);
}

void SocketServer::ReceiveImage(cv::Mat& image) {

    int num_bytes = 0;
    int image_ptr = 0;
    int image_size = 0;

    // Reset image
    image = cv::Mat::zeros(image_dims_, CV_8UC3);

    // Get image size
    image_size = image.total() * image.elemSize();

    // Allocate space for image buffer
    uchar sock_data[image_size];

    // Save image data to buffer
    std::cout << "Receiving new frame... --> ";
    for (int i = 0; i < image_size; i += num_bytes) {
        num_bytes = recv(sock_fdesc_conn_, sock_data + i, image_size - i, 0);
        if (num_bytes == -1) {
            printf("ERROR!: recv failed\n"
                    "i: %d\n"
                    "sock_fdesc: %d\n"
                    "image_size: %d\n"
                    "num_bytes: %d\n", i, sock_fdesc_conn_, image_size, num_bytes);
            exit(1);
        }
    }
    printf("%d Bytes received!\n", num_bytes);

    std::vector<uchar> vectordata(sock_data, sock_data + sizeof sock_data / sizeof sock_data[0]); // Converting array to STL vector.
    cv::Mat data_mat(vectordata, true);
    image = data_mat.reshape(3,image_dims_.height);     // reshape to 3 channels and "height" rows.
}

void SocketServer::DisplayImage(cv::Mat& image) {
    std::cout << "Displaying new frame..." << std::endl;
    cv::imshow("ServerSide", image);
    cv::waitKey(30);
}

int SocketServer::GetWidth() {
    return image_dims_.width;
}

int SocketServer::GetHeight() {
    return image_dims_.height;
}
