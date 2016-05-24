# A simple application to stream OpenCV images

`socket_server.cpp` implements a TCP socket server listening for incoming OpenCV images (cv::Mat). `socket_client.cpp` provides a class that connects to a TCP socket and sends it images of configurable size.

## Dependencies

* OpenCV (3.0+)

## Running the samples

`src/sample_server.cpp` and `src/sample_client.cpp` demonstrate how to use the classes. Build the samples with cmake:
```bash
mkdir build
cd build
cmake ../src
```

Start the server, providing which port to listen on:
```bash
./sample_server 30000
```

On another terminal, start the client by specifying the port of the server's socket and dimensions of the image (width, height). 
```bash
./sample_client 30000 512 424
```

## Client-Server Protocol
The first items the server expects from a client is two integers, specifying the incoming images' width (cols) and height (rows). From the sample server:
```c++
server_ptr->ConnectToNetwork();
server_ptr->ReceiveImageDims();
while(1) {
  cv::Mat image;
  server_ptr->ReceiveImage(image);
  server_ptr->DisplayImage(image);
}
return 1;
```

After the server has received the dimensions, it displays the stream of images in an OpenCV window.
The sample client obeys this protocol by first sending the image dimensions, followed by a stream of images, until the program is killed.
```c++
client_ptr->ConnectToServer();
client_ptr->SendImageDims(cols, rows);
while (1) {
  cv::Mat image;
  ...
  client_ptr->SendImage(image);
}
return 1;
```

`THE SAMPLE CODE USES LOCALHOST FOR THE PURPOSE OF DEMONSTRATION AND TESTING. CHANGE hostname VARIABLE IN ORDER TO USE A PROPOR HOSTNAME IP ADDRESS.`

## Credits
This code builds upon `Toy OpenCV Socket Server` developed by *Alexander Koumis* and maintained at <https://github.com/alexanderkoumis/toy-opencv-mat-socket-server.git>

## LICENSE
This application is a free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License (LGPL v3) as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version. KinectFeatures is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License (LGPL v3). See <http://www.gnu.org/licenses/>.