#include "helpers.hpp"

#include <opencv2/imgcodecs.hpp>

#include <functional> // function, bind
#include <future>     // future, async
#include <iostream>

std::once_flag once_dims;
std::once_flag once_serv;
std::unique_ptr<SocketClient> client_ptr;
std::unique_ptr<SocketServer> server_ptr;

std::mutex send_mutex;
std::condition_variable send_cond;
std::thread serv_thread;

bool server_ready(false);

int ClientServerConnect(const std::string serv_hostname, const int port) {

  SocketClient::SockInfo_t client_info;
  SocketServer::SockInfo_t server_info;

  if (SocketServer::CreateSocket(port, server_info) == -1) {
    printf("Error creating server socket\n");
    return -1;
  }

  if (SocketClient::CreateSocket(serv_hostname.c_str(), port, client_info) == -1) {
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

void TransmitDims(const cv::Mat& image, cv::Size2i& image_dims) {

  std::future<cv::Size2i> f_server_recv_dims = std::async(std::launch::async, [&]() {
    return server_ptr->ReceiveImageDims();
  });

  std::future<void> f_client_send_dims = std::async(std::launch::async, [&]() {
    return client_ptr->SendImageDims(image.cols, image.rows);
  });

  f_client_send_dims.wait();
  f_server_recv_dims.wait();
  image_dims = f_server_recv_dims.get();
  printf("server.dims: [%dx%d]\n", image_dims.width, image_dims.height);
}

void LaunchServer(const std::unique_ptr<SocketServer>& server_ptr,
                  const cv::Size2i& image_dims,
                  const std::string& out_dir) {

  const std::string outfile_base = out_dir + "/pic";
  static int pic_num = 0;

  std::packaged_task<void()> t_serv_init ([&](){
    cv::Mat image;
    printf("server.Run: Waiting for image\n");
    server_ptr->ReceiveImage(image_dims, image);
    std::string outfile_full = outfile_base + std::to_string(pic_num++) + ".png";
    std::cout << "Writing image to: " << outfile_full << std::endl;
    cv::imwrite(outfile_full, image);
    printf("server.Run: Received image\n");
  });


 std::future<void> f_serv_init = t_serv_init.get_future();
 std::thread(std::move(t_serv_init)).detach();
}

void TransmitImage(const cv::Mat& image, cv::Size2i& image_dims, const std::string out_dir) {

  std::call_once(once_dims, TransmitDims, std::cref(image), std::ref(image_dims));
  std::call_once(once_serv, LaunchServer, std::cref(server_ptr), std::cref(image_dims), std::ref(out_dir));

  client_ptr->SendImage(image);

}

int main(int argc, char** argv) {

  setbuf(stdout, NULL);

  std::vector<cv::Mat> images;
  cv::Size2i image_dims;

  std::string hostname;
  std::string in_dir;
  std::string out_dir;
  int port = 0;

  std::tie(hostname, port, in_dir, out_dir) = ParseArgs(argc, argv);

  LoadImages(in_dir, images);



  if (ClientServerConnect(hostname, port) == -1) {
    printf("Error creating socket client/server pair\n");
    return 1;
  }

  if (images.size() == 0) {
    printf("Error: Images not loaded properly\n");
    return 1;
  }

  for (auto& image : images) {
    TransmitImage(image, image_dims, out_dir);
  }


  printf("hihihihi\n");
  serv_thread.join();
  return 0;
}
