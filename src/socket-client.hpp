#ifndef SOCKET_CLIENT_HPP
#define SOCKET_CLIENT_HPP

#include <memory>
#include <sys/socket.h>

#include <opencv2/core.hpp>

class SocketClient {
 public:
  typedef std::unique_ptr<SocketClient> SocketClientPtr;
  typedef std::tuple<int, struct sockaddr*, socklen_t> SockInfo_t;

  static int CreateSocket(const char* serv_hostname, const int port,
                          SockInfo_t& sock_info);

  SocketClient(const SockInfo_t& client_info);
  ~SocketClient();
  int Connect();
  void SendImage(const cv::Mat& image);
  void SendImageDims(int rows, const int cols) const;

 private:
  const int socket_fdesc_;
  const struct sockaddr* sockaddr_;
  const socklen_t socklen_;
};

#endif
