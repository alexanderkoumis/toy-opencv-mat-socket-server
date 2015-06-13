#include "socket-server.hpp"

#include <signal.h>   // sigaction
#include <unistd.h>   // close
#include <sys/wait.h> // waitpid
#include <arpa/inet.h> // inet_ntop

#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>
#include <sstream>
#include <thread>


#include <opencv2/imgcodecs.hpp>

// Based on http://beej.us/guide/bgnet/examples/server.c

void SigChildHandler(int signal) {
  // Wait for process to terminate
  // -1:      match any process
  // NULL:    Don't store returned dead child status
  // WNOHANG: Don't block parent process
  while(waitpid(-1, NULL, WNOHANG) > 0);
}

SocketServer::SocketServer(const int sock_fdesc) :
    sock_fdesc_(sock_fdesc) {
  setbuf(stdout, NULL);
}

int SocketServer::CreateSocket(const int port, SockInfo_t& sock_info) {

  struct sigaction sig_action;
  sockaddr_in server_addr;
  sockaddr_in client_addr;
  socklen_t client_len;

  int sock_fdesc_init = 0;
  const int reuse_on = 1;

  // Set server address hints struct
  memset((char*)&server_addr, 0, sizeof(sockaddr_in));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(port);

  // Initialize Socket
  if ((sock_fdesc_init = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("server.socket\n");
    return -1;
  }

  // Set socket options
  // SOL_SOCKET:   Manipulate options at socket API level
  // SO_REUSEADDR: Option name - Reuse port if busy and in TIME_WAIT state (zombie),
  //               throw address-in-use error if in any other state
  // reuse_on:     Option value
  // sizeof(uint): Option length
  if (setsockopt(sock_fdesc_init, SOL_SOCKET, SO_REUSEADDR, &reuse_on,
                 sizeof(unsigned int)) == -1) {
    perror("server.setsockopt");
    exit(1);
  }

  // Bind socket to server address
  if (bind(sock_fdesc_init, (struct sockaddr*)&server_addr,
           sizeof(sockaddr_in)) == -1) {
    close(sock_fdesc_init);
    perror("server.bind");
    return -1;
  }

  // Reap dead processes
  sig_action.sa_handler = SigChildHandler;
  sigemptyset(&sig_action.sa_mask);
  sig_action.sa_flags = SA_RESTART;

  if (sigaction(SIGCHLD, &sig_action, NULL) == -1) {
    perror("server.sigaction");
    exit(1);
  }

  sock_info = std::make_tuple(sock_fdesc_init, client_addr, client_len);
  return 0;
}

int SocketServer::Accept(const SockInfo_t& sock_info) {
  int sock_fdesc_new = 0;

  const int sock_fdesc_init = std::get<0>(sock_info);
  struct sockaddr_in client_addr = std::get<1>(sock_info);
  socklen_t client_len = std::get<2>(sock_info);

  // Listen on socket file descriptor, set max pending connections
  if (listen(sock_fdesc_init, 5 == -1)) {
    perror("server.listen");
    return -1;
  }

  // Block process until connection with client has been established.
  // 'sock_fdesc_new' set as new file descriptor to be used for communication
  sock_fdesc_new = accept(sock_fdesc_init, (struct sockaddr *)&client_addr, &client_len);
  if (sock_fdesc_new == -1) {
    perror("server.accept");
    exit(1);
  }

  return sock_fdesc_new;
}

cv::Size2i SocketServer::ReceiveImageDims() const {

  ssize_t bytes_sent = 0;
  size_t dims_size = 0;

  int cols = 0;
  int rows = 0;

  size_t sizeof_dims = sizeof(int);

  if ((bytes_sent = recv(sock_fdesc_, (char*)&cols, sizeof_dims, 0)) == -1) {
    printf("ERROR!: recv failed\n"
           "sock_fdesc: %d\n"
           "image_size: %zu\n"
           "bytes_sent: %zu\n", sock_fdesc_, dims_size, bytes_sent);
    exit(1);
  }

  if ((bytes_sent = recv(sock_fdesc_, (char*)&rows, sizeof_dims, 0)) == -1) {
    printf("ERROR!: recv failed\n"
           "sock_fdesc: %d\n"
           "image_size: %zu\n"
           "bytes_sent: %zu\n", sock_fdesc_, dims_size, bytes_sent);
    exit(1);
  }

  return cv::Size2i(cols, rows);
}

void SocketServer::ReceiveImage(const cv::Size2i& image_dims, cv::Mat& image) const {

  int bytes = 0;
//  int image_ptr = 0;

  // Reset image
  image = cv::Mat::zeros(image_dims, CV_8UC3);

  // Get image size
  const size_t image_size = (image.total() * image.elemSize());

  // Allocate space for image buffer
  uchar sock_data[image_size];


  // Save image data to buffer
  size_t i = 0;

  for (i = 0; i < image_size; i += bytes) {
    bytes = recv(sock_fdesc_, sock_data + i, image_size - i, 0);
    if (bytes == -1) {
      printf("read image or somethin else idk\n");
      break;
    }
  }


  int image_ptr = 0;
  // Write image data to cv::Mat
  for (int r = 0;  r < image_dims.height; ++r) {
    for (int c = 0; c < image_dims.width; ++c) {
      image.at<cv::Vec3b>(c,r) = cv::Vec3b(sock_data[image_ptr+0],
                                           sock_data[image_ptr+1],
                                           sock_data[image_ptr+2]);
      image_ptr += 3;
      printf("[%d %d]\n", c, r);
    }
  }
}
