#include <memory> // unique_ptr
#include <sys/stat.h>

#include <opencv2/core.hpp> // Mat

#include "socket_server.hpp"


void ParseArgs(int argc, char** argv) {
  // Verify whether two arguments where given: "./server" and port number.
  if (argc != 2){
      perror("Wrong number of arguments!\nUsage: ./server <port>\n");
      exit(1);
  }
}

int main(int argc, char** argv) {
  ParseArgs(argc, argv);
  int port = atoi(argv[1]);
  std::unique_ptr<SocketServer> server_ptr(new SocketServer(port));
  server_ptr->ConnectToNetwork();
  server_ptr->ReceiveImageDims();
  while(1) {
    cv::Mat image;
    server_ptr->ReceiveImage(image);
    server_ptr->DisplayImage(image);
  }
  return 1; // Should not return
}
