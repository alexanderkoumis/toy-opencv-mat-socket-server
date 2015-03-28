// #include <cstring>
#include <cstdio>
#include <string>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>

#include "socket_common.hpp"

void ConnectToServer();
void GenerateImage(cv::Mat& image);
void SendImage(cv::Mat& image);

int pic_num = 0;
int socket_fdesc;
struct addrinfo* addr_resp;

int main(int argc, char** argv) {
  ConnectToServer();
  while (1) {
    cv::Mat image;
    GenerateImage(image);
    SendImage(image);
  }
  return 0;
}

void ConnectToServer() {
  struct addrinfo addr_hints;

  // Specify criteria for address structs to be returned by getAddrinfo
  memset(&addr_hints, 0, sizeof(addr_hints));
  addr_hints.ai_socktype = SOCK_STREAM;
  addr_hints.ai_family = AF_INET;

  // Populate addr_info_resp with address responses matching hints
  if (getaddrinfo(hostname, std::to_string(server_port).c_str(), &addr_hints,
                  &addr_resp) != 0) {
    perror("Couldn't connect to host!");
    exit(1);
  }

  // Create socket file descriptor for server
  socket_fdesc = socket(addr_resp->ai_family, addr_resp->ai_socktype,
                        addr_resp->ai_protocol);
  if (socket_fdesc == -1) {
    perror("Error opening socket");
    exit(1);
  }

  // Connect to server specified in address struct, assign process to server
  // file descriptor
  if (connect(socket_fdesc, addr_resp->ai_addr, addr_resp->ai_addrlen) == -1) {
    perror("Error connecting to address");
    exit(1);
  }
}

void GenerateImage(cv::Mat& image) {
  int salt(0);
  image = cv::Mat::zeros(height, width, CV_8UC3);
  for (int row = 0; row < height; ++row) {
    for (int col = 0; col < width; ++col) {
      int randy = (salt % 23 == 0) ? rand() % 255 : 0;
      image.at<cv::Vec3b>(row, col)[0] = randy;
      image.at<cv::Vec3b>(row, col)[1] = randy;
      image.at<cv::Vec3b>(row, col)[2] = randy;
      salt += 7;
    }
  }
}

void SendImage(cv::Mat& image) {
    image = image.reshape(0,1);
    int image_size = image.total() * image.elemSize();
    int bytes = send(socket_fdesc, image.data, image_size, 0);
    printf("Sent %d-byte image to port %d\n", image_size, server_port);
}
