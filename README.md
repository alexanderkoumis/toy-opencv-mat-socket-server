# A simple application to send and receive OpenCV images

`socket_server.cpp` implements a TCP socket server listening for incoming OpenCV images (cv::Mat). `socket_client.cpp` provides a class that connects to a TCP socket and sends it images of configurable size.

## Running the samples

`src/sample_server.cpp` and `src/sample_client.cpp` demonstrate how to use these classes in another program. Build the samples with cmake:
```bash
mkdir build
cd build
cmake ../src
```

Start the server, providing which port to listen on, followed by the path to save images to:
```bash
./sample_server 5005 ~/Desktop/socket_images
```

Start the client, specifying the port of the server's socket, and dimensions of the generated images (cols, rows). Careful! This can fill up your hard drive fast.
```bash
./sample_client 5005 1920 1080
```

## Client-Server Protocol
The first items the server expects from a client is two integers, specifying the incoming images' columns (width) and rows (height). From the sample server:
```c++
server_ptr->ConnectToNetwork();
server_ptr->ReceiveImageDims();
while(1) {
  cv::Mat image;
  server_ptr->ReceiveImage(image);
  server_ptr->WriteImage(image);
}
return 1;
```

After the server has received the dimensions, it saves received images to the user-supplied path.
The sample client obeys this protocol by first sending the image dimensions, followed by a stream of images, until the program is killed.
```c++
client_ptr->ConnectToServer();
client_ptr->SendImageDims(cols, rows);
while (1) {
  cv::Mat image;
  GenerateImage(cols, rows, image);
  client_ptr->SendImage(image);
}
return 1;
```
