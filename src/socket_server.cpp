#include <memory>
#include <string>
#include <sstream>
#include <cstdio>
#include <cstring>
#include <cctype> // isdigit
#include <sys/types.h> 
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>

#include "socket_common.hpp"

class Server {
 public:
  Server(int port, std::string out_path);
  void ConnectToNetwork();
  void ReceiveImage(cv::Mat& image);
  void WriteImage(cv::Mat& image);

 private:
  struct sockaddr_in server_addr_;
  struct sockaddr_in client_addr_;
  std::string pic_filename_;
  std::string out_path_;
  socklen_t client_len_;
  size_t server_addr_size_;
  int port_;
  int pic_count_;
  int sock_fdesc_init_;
  int sock_fdesc_conn_;
};

Server::Server(int port, std::string out_path) :
    out_path_(out_path),
    client_len_(0),
    server_addr_size_(sizeof(server_addr_)),
    port_(port),
    pic_count_(0),
    sock_fdesc_init_(0),
    sock_fdesc_conn_(0) {
  client_len_ = server_addr_size_; // idk
}

void Server::ConnectToNetwork() {

  // Initialize Socket
  sock_fdesc_init_ = socket(AF_INET, SOCK_STREAM, 0);
  if (sock_fdesc_init_ == -1) {
    perror("Couldn't create socket!\n");
    exit(1);
  }
  
  // Zero out server address struct
  memset((char*)&server_addr_, 0, server_addr_size_);

  // Set server address struct
  server_addr_.sin_family = AF_INET;
  server_addr_.sin_addr.s_addr = INADDR_ANY;
  server_addr_.sin_port = htons(port_);

  // Assign server address to initial socket file descriptor
  if (bind(sock_fdesc_init_, (struct sockaddr*)&server_addr_,
      server_addr_size_) == -1) {
    perror("ERROR! Couldn't bind initial socket file descriptor!");
    exit(1);
  }
  
  // Enable listening on initial socket file descriptor
  listen(sock_fdesc_init_, 5);

  // Block process until connection with client has been established.
  // 'client_fdesc' set as new file descriptor to be used for communication
  sock_fdesc_conn_ = accept(sock_fdesc_init_, (struct sockaddr*)&client_addr_,
                            &client_len_);
  if (sock_fdesc_conn_ == -1) {
    perror("ERROR! Client couldn't connect!");
    exit(1);
  }
}

void Server::ReceiveImage(cv::Mat& image) {

  int bytes(0);
  int image_ptr(0);
  int image_size(0);

  // Reset image
  image = cv::Mat::zeros(height, width, CV_8UC3);
  
  // Get image size
  image_size = image.total() * image.elemSize();

  // Allocate space for image buffer
  uchar sock_data[image_size];

  // Save image data to buffer
  for (int i = 0; i < image_size; i += bytes) {
    if ((bytes = recv(sock_fdesc_conn_, sock_data + i, image_size - i, 0)) == -1) {
      printf("ERROR!: recv failed\n"
             "i: %d\n"
             "sock_fdesc: %d\n"
             "image_size: %d\n"
             "bytes: %d\n", i, sock_fdesc_conn_, image_size, bytes);
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
  std::ostringstream oss;
  oss << out_path_ << "/pic_" << std::to_string(pic_count_++) << ".jpg";
  pic_filename_ = oss.str();
}

void Server::WriteImage(cv::Mat& image) {
  cv::imwrite(pic_filename_, image);
}

void AssertCond(bool assert_cond, const char* fail_msg) {
  if (!assert_cond) {
    printf("Error: %s\nUsage: ./pic-server <port> <out_path>\n", fail_msg);
    exit(1);
  }
}

bool DirExists(const char* path) {
  struct stat info;
  if (stat(path, &info) != 0) {
    perror("Can't access path");
    return 0;
  }
  else if(info.st_mode & S_IFDIR) {
    return 1;
  }
  else return 0;
}

void ParseArgs(int argc, char** argv) {
  AssertCond(argc == 3, "Wrong number of arguments");
  AssertCond(DirExists(argv[2]), "Supplied directory does not exist");
}

int main(int argc, char** argv) {
  ParseArgs(argc, argv);
  std::unique_ptr<Server> server_ptr(new Server(atoi(argv[1]), argv[2]));
  server_ptr->ConnectToNetwork();
  while(1) {
    cv::Mat image;
    server_ptr->ReceiveImage(image);
    server_ptr->WriteImage(image);
  }
  return 0;
}
