#include "socket_client.hpp"

#include <memory>  // unique_ptr
#include <cstdlib> // atoi

const int kTestImageRows = 1920;
const int kTestImageCols = 1080;

void GenerateImage(cv::Mat& image) {
  int lol = 0;
  image = cv::Mat::zeros(kTestImageRows, kTestImageCols, CV_8UC3);
  for (int row = 0; row < kTestImageRows; ++row) {
    for (int col = 0; col < kTestImageCols; ++col) {
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
  std::unique_ptr<SocketClient> client_ptr(new SocketClient(hostname, port));
  client_ptr->ConnectToServer();
  client_ptr->SendImageDims(kTestImageRows, kTestImageCols);
  while (1) {
    cv::Mat image;
    GenerateImage(image);
    client_ptr->SendImage(image);
  }
  return 0;
}
