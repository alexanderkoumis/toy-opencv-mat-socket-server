#include <memory> // unique_ptr
#include <cstdio>
#include <string>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>

#include "socket_common.hpp"

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

class Client {
 public:
  Client(const char* hostname, int port);
  void ConnectToServer();
  void SendImage(cv::Mat& image);
 private:
  const char* hostname_;
  int port_;
  int pic_num_;
  int socket_fdesc_;
};

Client::Client(const char* hostname, int port) :
    hostname_ (hostname),
    port_(port),
    pic_num_(0),
    socket_fdesc_(0) {}

void Client::ConnectToServer() {
  struct addrinfo addrinfo_hints;
  struct addrinfo* addrinfo_resp;

  // Specify criteria for address structs to be returned by getAddrinfo
  memset(&addrinfo_hints, 0, sizeof(addrinfo_hints));
  addrinfo_hints.ai_socktype = SOCK_STREAM;
  addrinfo_hints.ai_family = AF_INET;

  // Populate addr_info_resp with address responses matching hints
  if (getaddrinfo(hostname_, std::to_string(port_).c_str(),
                  &addrinfo_hints, &addrinfo_resp) != 0) {
    perror("Couldn't connect to host!");
    exit(1);
  }

  // Create socket file descriptor for server
  socket_fdesc_ = socket(addrinfo_resp->ai_family, addrinfo_resp->ai_socktype,
                        addrinfo_resp->ai_protocol);
  if (socket_fdesc_ == -1) {
    perror("Error opening socket");
    exit(1);
  }

  // Connect to server specified in address struct, assign process to server
  // file descriptor
  if (connect(socket_fdesc_, addrinfo_resp->ai_addr,
              addrinfo_resp->ai_addrlen) == -1) {
    perror("Error connecting to address");
    exit(1);
  }

  free(addrinfo_resp);
}

void Client::SendImage(cv::Mat& image) {
    image = image.reshape(0,1);
    int image_size = image.total() * image.elemSize();
    int bytes = send(socket_fdesc_, image.data, image_size, 0);
    printf("Sent %d-byte image to port %d\n", image_size, port_);
}

void AssertCond(bool assert_cond, const char* fail_msg) {
  if (!assert_cond) {
    printf("Error: %s\nUsage: ./pic-client <port>\n", fail_msg);
    exit(1);
  }
}

void ParseArgs(int argc, char** argv) {
  AssertCond(argc == 2, "Wrong number of arguments");
}

int main(int argc, char** argv) {
  ParseArgs(argc, argv);
  int port = atoi(argv[1]);
  const char hostname[] = "localhost";
  std::unique_ptr<Client> client_ptr(new Client(hostname, port));
  client_ptr->ConnectToServer();
  while (1) {
    cv::Mat image;
    GenerateImage(image);
    client_ptr->SendImage(image);
  }
  return 0;
}
