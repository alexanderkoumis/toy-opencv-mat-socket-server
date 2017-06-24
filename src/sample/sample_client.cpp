#include "socket_client.hpp"

#include <memory> // unique_ptr
#include <cstdlib> // atoi

#include "opencv2/opencv.hpp"

void ParseArgs(int argc, char** argv) {
    if (argc != 4){
        printf("Wrong number of arguments!\nUsage: ./client <port> <rows> <cols>\n");
        exit(1);
    }
}

int main(int argc, char** argv) {
  ParseArgs(argc, argv);
  int port = atoi(argv[1]);
  int cols = atoi(argv[2]);                 // width
  int rows = atoi(argv[3]);                 // height
  
  cv::VideoCapture cap(0);                        /* open webcam */
  if(!cap.isOpened())
      return -1;

  cap.set(CV_CAP_PROP_FRAME_WIDTH, cols);      /* set width */
  cap.set(CV_CAP_PROP_FRAME_HEIGHT, rows);     /* set height */

  const char hostname[] = "localhost"; // change this to your hostname! 
  std::unique_ptr<SocketClient> client_ptr(new SocketClient(hostname, port));
  client_ptr->ConnectToServer();
  client_ptr->SendImageDims(cols, rows);
  while (1) {
    cv::Mat image;
    cap >> image;
    cv::imshow("Client",image);
    cv::waitKey(10);
    client_ptr->SendImage(image);
  }
  return 1; // Should not return
}
