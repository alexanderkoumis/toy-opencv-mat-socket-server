#ifndef SOCKET_SERVER_HPP
#define SOCKET_SERVER_HPP

#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

#include <string>
#include <queue>
#include <memory>
#include <tuple>
#include <mutex>
#include <thread>

#include <opencv2/core.hpp>




class SocketServer {
 public:
  typedef std::unique_ptr<SocketServer> SocketServerPtr;
  typedef std::tuple<int, struct sockaddr_in, socklen_t> SockInfo_t;

  SocketServer(const int sock_fdesc);

  static int CreateSocket(const int port, SockInfo_t& sock_info);
  static int Accept(const SockInfo_t& sock_info);

  cv::Size2i ReceiveImageDims() const;

  void ReceiveImage(const cv::Size2i& image_sz, cv::Mat& image) const;
  void ReadErrors();

 private:
  std::mutex q_lock_;
  int sock_fdesc_;
};




#endif
