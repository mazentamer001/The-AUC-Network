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
