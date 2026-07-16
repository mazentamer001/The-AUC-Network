# The AUC Network

## Project Description

The AUC Network is a client-server application developed in C++ to provide a centralized platform for students at The American University in Cairo (AUC). It enables students to communicate, collaborate, and exchange resources through a single integrated platform.

The application includes several core features such as user authentication, public and private chat, discussion forums, a marketplace, file sharing, and user profiles. It is built using Qt for the graphical user interface, Boost.Asio for networking, CMake for project configuration, and Google Test for unit testing.


## Building and Running

> **Important:** Before building, configure the server IP address in the client source code.

```bash
cd The-AUC-Network

cmake -B build
cmake --build build

# Start the server
./build/server/Server

# In a separate terminal, start the client
./build/client/TestGUI
```
