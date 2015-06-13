#include "helpers.hpp"

#include <tuple>

#include <opencv2/imgcodecs.hpp> // imread

void AssertCond(bool assert_cond, const char* err_msg) {
  static const char* err_fmt =
  "Error: %s\n"
  "Usage: ./mat_client_server <server hostname> <port> <in dir> <out dir>\n";

  if (!assert_cond) {
    printf(err_fmt, err_msg);
    exit(1);
  }
}

Args_t ParseArgs(int argc, char** argv) {
  AssertCond(argc == 5, "Wrong number of arguments");
  int port(atoi(argv[2]));
  return std::tie(argv[1], port, argv[3], argv[4]);
}

void LoadImages(const std::string pic_dir, std::vector<cv::Mat>& images) {

  std::ostringstream oss_cmd;

  // Get full path of image filename (sorted) printing script
  const std::string curr_file = std::string(__FILE__);
  const std::string curr_dir = curr_file.substr(0, curr_file.find_last_of("/"));
  const std::string image_name_script = curr_dir + "/get-image-names.py";

  // Create system process command
  oss_cmd << "python " << image_name_script << " " << pic_dir;
  std::string python_cmd(oss_cmd.str());

  // Execute command, use printed filename in imread
  FILE* file = popen(python_cmd.c_str(), "r");
  if (file) {
    constexpr std::size_t MAX_LINE_SZ = 1024;
    char line[MAX_LINE_SZ];
    while (fgets(line, MAX_LINE_SZ, file)) {
      std::string filename(pic_dir + "/" + line);
      filename = filename.substr(0, filename.length()-1); // remove /n
      const cv::Mat image = cv::imread(filename, -1);
      images.push_back(image);
    }
    pclose(file);
  }
}
