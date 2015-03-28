#include <string>
#include <cstdio>
#include <cstring>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>

#include "socket_common.hpp"

void ConnectToNetwork();
void ReceiveImage(cv::Mat& image);

static int sock_fdesc_init;
static int sock_fdesc_conn;
socklen_t client_len;
struct sockaddr_in server_addr;
struct sockaddr_in client_addr;

int main(int argc, char** argv) {
  ConnectToNetwork();
  int pic_count = 0;
  while(1) {
    cv::Mat image;
    std::string pic_filename = "../pics/pic_" + pic_count++;
    pic_filename += ".jpg";
    ReceiveImage(image);
    cv::imwrite(pic_filename, image);
  }
  return 0;
}

void ConnectToNetwork() {

  // Initialize local variables
  int bind_result(0);
  socklen_t client_addr_len(0);

  // Initialize Socket
  sock_fdesc_init = socket(AF_INET, SOCK_STREAM, 0);
  if (sock_fdesc_init == -1) {
    perror("Couldn't create socket!\n");
    exit(1);
  }
  
  // Zero out server address struct
  memset((char*)&server_addr, 0, sizeof(server_addr));

  // Set server address struct
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(server_port);

  // Assign server address to initial socket file descriptor
  if (bind(sock_fdesc_init, (struct sockaddr*)&server_addr,
      sizeof(server_addr)) == -1) {
    perror("ERROR! Couldn't bind initial socket file descriptor!");
    exit(1);
  }
  
  // Enable listening on initial socket file descriptor
  listen(sock_fdesc_init, 5);

  // Get size of client address for forthcoming 'accept' command
  client_addr_len = sizeof(client_addr);

  // Block process until connection with client has been established.
  // 'client_fdesc' set as new file descriptor to be used for communication
  sock_fdesc_conn = accept(sock_fdesc_init, (struct sockaddr*)&client_addr,
                           &client_addr_len);
  if (sock_fdesc_conn < 0) {
    perror("ERROR! Client couldn't connect!");
    exit(1);
  }
}

void ReceiveImage(cv::Mat& image) {
  // Declare local variables
  int bytes(0);
  int image_ptr(0);
  int image_size(0);
  uchar sock_data[image_size];

  // Reset image
  image = cv::Mat::zeros(height, width, CV_8UC3);
  
  // Get image size
  image_size = image.total() * image.elemSize();

  // Save image data to buffer
  for (int i = 0; i < image_size; i += bytes) {
    if ((bytes = recv(sock_fdesc_conn, sock_data + i, image_size - i, 0)) == -1) {
      printf("ERROR!: recv failed\n"
             "i: %d\n"
             "sock_fdesc: %d\n"
             "image_size: %d\n"
             "bytes: %d\n", i, sock_fdesc_conn, image_size, bytes);
      exit(1);
    }
  }

  // Write image data to cv::Mat
  for (int i = 0;  i < height; ++i) {
    for (int j = 0; j < width; ++j) {                                     
      image.at<cv::Vec3b>(i,j) = cv::Vec3b(sock_data[image_ptr+0],
                                           sock_data[image_ptr+1],
                                           sock_data[image_ptr+2]);
      image_ptr += 3;
    }
  }
}