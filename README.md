# Toy OpenCV Socket Server

`socket_server.cpp` implements a server listening on a TCP socket for incoming OpenCV images (datatype cv::Mat), saving them to disk once received. `socket_client.cpp` provides a class that connects to a TCP socket and sends it images of configurable size.

## Building samples

`src/sample_server.cpp` and `src/sample_client.cpp` demonstrate how to use these classes in another program. Build the samples with cmake:
```
mkdir build
cd build
cmake ../src
```
## Running the samples

Start the server, providing which port to listen on, followed by the path to save images to:
```
./sample_server 5005 ~/Desktop/socket_images
```

Start the client, specifying the port of the server's socket, and dimensions of the generated images (cols, rows). Careful! This can fill up your hard drive fast.
```
./sample_client 5005 1920 1080
```