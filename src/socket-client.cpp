#include "socket-client.hpp"

#include <unistd.h> // close
#include <netdb.h>  // addrinfo, getaddrinfo, freeaddrinfo

#include <cstdio>
#include <iostream>
#include <string>
#include <tuple>

#include <opencv2/highgui.hpp>

SocketClient::SocketClient(const SockInfo_t& client_info) :
    socket_fdesc_(std::get<0>(client_info)),
    sockaddr_(std::get<1>(client_info)),
    socklen_(std::get<2>(client_info))
  {}

SocketClient::~SocketClient() {
  close(socket_fdesc_);
}

int SocketClient::CreateSocket(const char* serv_hostname, const int port,
                               SockInfo_t& sock_info) {
  struct addrinfo addrinfo_hints;
  struct addrinfo* addrinfo_resp;

  // Specify criteria for address structs to be returned by getAddrinfo
  memset(&addrinfo_hints, 0, sizeof(addrinfo_hints));
  addrinfo_hints.ai_socktype = SOCK_STREAM;
  addrinfo_hints.ai_family = AF_INET;

  // Populate addr_info_resp with address responses matching hints
  if (getaddrinfo(serv_hostname, std::to_string(port).c_str(),
                  &addrinfo_hints, &addrinfo_resp) != 0) {
    perror("Couldn't connect to host!");
    freeaddrinfo(addrinfo_resp);
    return -1;
  }

  // Create socket file descriptor for server
  const int sock_fdesc = socket(addrinfo_resp->ai_family,
                                addrinfo_resp->ai_socktype,
                                addrinfo_resp->ai_protocol);
  if (sock_fdesc == -1) {
    perror("Error opening socket");
    freeaddrinfo(addrinfo_resp);
    return -1;
  }

  freeaddrinfo(addrinfo_resp);
  sock_info = std::make_tuple(sock_fdesc, addrinfo_resp->ai_addr,
                              addrinfo_resp->ai_addrlen);

  return 0;

}

int SocketClient::Connect() {
  if (connect(socket_fdesc_, sockaddr_, socklen_) == -1) {
    perror("Error connecting to server address");
    return -1;
  }
  return 0;
}

void SocketClient::SendImage(const cv::Mat& image_2d) {

  cv::Mat image_1d = image_2d.reshape(0,1);
  size_t image_size = image_1d.total() * image_1d.elemSize();
  ssize_t bytes_sent_total = 0;
  while (bytes_sent_total < (ssize_t)image_size) {
    ssize_t bytes_sent_once = send(socket_fdesc_, image_1d.data, image_size, 0);
    if (bytes_sent_once == -1) {
      perror("client.send");
      exit(1);
    }
    else {
      bytes_sent_total += bytes_sent_once;
    }
  }
}

void SocketClient::SendImageDims(const int cols, const int rows) const {
  if (send(socket_fdesc_, (char*)&cols, sizeof(cols), 0) == -1 ||
      send(socket_fdesc_, (char*)&rows, sizeof(rows), 0) == -1) {
    perror("client.send");
    exit(1);
  }
}

