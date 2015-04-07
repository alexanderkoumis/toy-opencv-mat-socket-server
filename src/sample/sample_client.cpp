#include "socket_client.hpp"

#include <memory>  // unique_ptr
#include <cstdlib> // atoi

void GenerateImage(int rows, int cols, cv::Mat& image) {
  int lol = 0;
  image = cv::Mat::zeros(rows, cols, CV_8UC3);
  for (int row = 0; row < rows; ++row) {
    for (int col = 0; col < cols; ++col) {
      int idk = (lol % 23 == 0) ? rand() % 255 : 35;
      image.at<cv::Vec3b>(row, col)[0] = idk;
      image.at<cv::Vec3b>(row, col)[1] = idk;
      image.at<cv::Vec3b>(row, col)[2] = idk;
      lol += 7;
    }
  }
}

void AssertCond(bool assert_cond, const char* fail_msg) {
  if (!assert_cond) {
    printf("Error: %s\nUsage: ./pic-client <port> <rows> <cols>\n", fail_msg);
    exit(1);
  }
}

void ParseArgs(int argc, char** argv) {
  AssertCond(argc == 4, "Wrong number of arguments");
}

int main(int argc, char** argv) {
  ParseArgs(argc, argv);
  int port = atoi(argv[1]);
  int cols = atoi(argv[2]);
  int rows = atoi(argv[3]);
  const char hostname[] = "localhost";
  std::unique_ptr<SocketClient> client_ptr(new SocketClient(hostname, port));
  client_ptr->ConnectToServer();
  client_ptr->SendImageDims(cols, rows);
  while (1) {
    cv::Mat image;
    GenerateImage(cols, rows, image);
    client_ptr->SendImage(image);
  }
  return 0;
}
