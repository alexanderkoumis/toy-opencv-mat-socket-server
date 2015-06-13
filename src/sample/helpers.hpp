#ifndef HELPERS_HPP
#define HELPERS_HPP

#include <string>            // string
#include <vector>            // vector
#include <utility>           // pair
#include <tuple>             // tuple, make_tuple
#include <memory>            // unique_ptr

#include <opencv2/core.hpp>  // Mat

#include "socket-client.hpp" // SocketClient
#include "socket-server.hpp" // SocketServer

typedef std::pair<std::unique_ptr<SocketClient>,
                  std::unique_ptr<SocketServer>> ClientServer_t;

typedef std::tuple<std::string, int, std::string, std::string> Args_t;

void AssertCond(bool assert_cond, const char* err_msg);

Args_t ParseArgs(int argc, char** argv);

std::string CurrFileAncestorDir(const std::string& curr_file,
                                const std::string& ancestor_dir);

void LoadImages(const std::string dir, std::vector<cv::Mat>& images);

std::string Filename(const std::string& parent_dir, const std::string& prefix,
                     const int num, const std::string& ext);


#endif
