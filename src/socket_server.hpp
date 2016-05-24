#ifndef SOCKET_SERVER_HPP
#define SOCKET_SERVER_HPP

#include <string>
#include <sstream>
#include <cstdio>
#include <cstring>
#include <cctype> // isdigit
#include <sys/socket.h>
#include <netinet/in.h>

#include <opencv2/core.hpp>


class SocketServer {
public:
    SocketServer(int port);
    void ConnectToNetwork();
    void ReceiveImageDims();
    void ReceiveImage(cv::Mat& image);
    void DisplayImage(cv::Mat& image);
    int GetWidth();
    int GetHeight();

private:
    cv::Size2i image_dims_;
    struct sockaddr_in server_addr_;
    struct sockaddr_in client_addr_;
    socklen_t client_len_;
    size_t server_addr_size_;
    int port_;
    int sock_fdesc_init_;
    int sock_fdesc_conn_;
};

#endif
