#include "helpers.hpp"

#include <opencv2/imgcodecs.hpp>

#include <functional> // function, bind
#include <future>     // future, async
#include <iostream>

int ClientServerConnect(const std::string hostname, const int port,
                        std::unique_ptr<SocketClient>& client_ptr,
                        std::unique_ptr<SocketServer>& server_ptr) {

  SocketClient::SockInfo_t client_info;
  SocketServer::SockInfo_t server_info;

  if (SocketServer::CreateSocket(port, server_info) == -1) {
    printf("Error creating server socket\n");
    return -1;
  }

  if (SocketClient::CreateSocket(hostname.c_str(), port, client_info) == -1) {
    printf("Error creating client socket\n");
    return -1;
  }

  client_ptr.reset(new SocketClient(client_info));

  // Blocks until client connects to server socket
  std::future<int> f_server_accept = std::async(std::launch::async, [&]() {
    return SocketServer::Accept(server_info);
  });

  // Blocks until client connects to server socket
  std::future<int> f_client_connect = std::async(std::launch::async, [&]() {
    return client_ptr->Connect();
  });

  // Wait until client connects to server socket
  f_server_accept.wait();
  f_client_connect.wait();

  // Initialize server with returned file descriptor
  const int server_sock_fdesc = f_server_accept.get();
  if (server_sock_fdesc == -1) {
    printf("Error with server socket file descriptor\n");
    return -1;
  }
  server_ptr.reset(new SocketServer(server_sock_fdesc));

  return 0;
}

void TransmitDims(const std::unique_ptr<SocketClient>& client_ptr,
                  const std::unique_ptr<SocketServer>& server_ptr,
                  const cv::Mat& image, cv::Size2i& image_dims) {

  std::future<cv::Size2i> s_recv_dims = std::async(std::launch::async, [&]() {
    return server_ptr->ReceiveImageDims();
  });

  std::future<void> c_send_dims = std::async(std::launch::async, [&]() {
    return client_ptr->SendImageDims(image.cols, image.rows);
  });

  c_send_dims.wait();
  s_recv_dims.wait();
  image_dims = s_recv_dims.get();
  printf("server.dims: [%dx%d]\n", image_dims.width, image_dims.height);
}

void LaunchServer(const std::unique_ptr<SocketServer>& server_ptr,
                  const cv::Size2i& image_dims,
                  const std::string& out_dir,
                  std::thread& send_thread) {
  static int pic_num = 0;
  std::packaged_task<void()> t_serv_init ([&](){
    while (1) {
      cv::Mat image;
      const std::string filename = Filename(out_dir, "pic", pic_num++, "jpg");
      server_ptr->ReceiveImage(image_dims, image);
      cv::imwrite(filename, image);
      printf("server.Run: Received image, wrote to %s\n", filename.c_str());
    }
  });
  send_thread = std::thread(std::move(t_serv_init));
}

void TransmitImage(const std::unique_ptr<SocketClient>& client_ptr,
                   const std::unique_ptr<SocketServer>& server_ptr,
                   const cv::Mat& image, cv::Size2i& image_dims,
                   const std::string out_dir,
                   std::thread& send_thread) {

  static std::once_flag once_dims;
  static std::once_flag once_serv;
  std::call_once(once_dims, TransmitDims, std::cref(client_ptr),
                                          std::cref(server_ptr),
                                          std::cref(image),
                                          std::ref(image_dims));
  std::call_once(once_serv, LaunchServer, std::cref(server_ptr),
                                          std::cref(image_dims),
                                          std::cref(out_dir),
                                          std::ref(send_thread));
  client_ptr->SendImage(image);
}

int main(int argc, char** argv) {

  setbuf(stdout, NULL);

  std::unique_ptr<SocketClient> client_ptr;
  std::unique_ptr<SocketServer> server_ptr;
  std::thread send_thread;

  std::vector<cv::Mat> images;
  cv::Size2i image_dims;

  std::string hostname;
  std::string in_dir;
  std::string out_dir;
  int port = 0;

  std::tie(hostname, port, in_dir, out_dir) = ParseArgs(argc, argv);

  LoadImages(in_dir, images);

  if (ClientServerConnect(hostname, port, client_ptr, server_ptr) == -1) {
    printf("Error creating socket client/server pair\n");
    return 1;
  }

  if (images.size() == 0) {
    printf("Error: Images not loaded properly\n");
    return 1;
  }

  printf("This many images: %zu\n", images.size());
  for (auto& image : images) {
    static int num = 0;
    printf("Sending image %d/%zu\n", num++, images.size());
    TransmitImage(client_ptr, server_ptr, image, image_dims, out_dir, send_thread);
  }

  if (send_thread.joinable()) {
    send_thread.join();
  }

  return 0;
}
