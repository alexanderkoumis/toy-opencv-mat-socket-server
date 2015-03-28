// #include <cstring>
#include <cstdio>
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
int server_fdesc;
struct sockaddr_in server_addr_2;
struct hostent* server;

cv::Mat last_image;

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
   char buffer[256];
   server_fdesc = socket(AF_INET, SOCK_STREAM, 0);
   if (server_fdesc < 0){
      perror("ERROR opening socket");
      exit(1);
   }
   server = gethostbyname(hostname); // guess this is deprecated
   if (server == NULL) {
      printf("ERROR, no such host\n");
      exit(1);
   }
   memset((char*) &server_addr_2, 0, sizeof(server_addr_2));
   server_addr_2.sin_family = AF_INET;
   bcopy((char *)server->h_addr, (char *)&server_addr_2.sin_addr.s_addr, server->h_length);
   server_addr_2.sin_port = htons(server_port); 
   if (connect(server_fdesc, (const sockaddr*)&server_addr_2, sizeof(server_addr_2)) < 0) {
      perror("ERROR connecting!\n");
      exit(1);
   }
}

void GenerateImage(cv::Mat& image) {
  image = cv::Mat::zeros(height, width, CV_8UC3);
  for (int row = 0; row < height; ++row) {
    for (int col = 0; col < width; ++col) {
      image.at<cv::Vec3b>(row, col)[0] = rand() % 255;
      image.at<cv::Vec3b>(row, col)[1] = rand() % 255;
      image.at<cv::Vec3b>(row, col)[2] = rand() % 255;
    }
  }
}

void SendImage(cv::Mat& image) {
    image = image.reshape(0,1);
    int image_size = image.total() * image.elemSize();
    int bytes = send(server_fdesc, image.data, image_size, 0);
    printf("Sent %d-byte image to port %d\n", image_size, server_port);
}