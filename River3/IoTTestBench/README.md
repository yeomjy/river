# IoT Simulation Infrastructure

This is a PoC that shows how we can simulate a network pipe.

This PoC uses a client (`src/process-wrapper.c`) and a server (`src/server.c`).

The client accepts as CLI arguments the command to run, executes the command and sends the output to the server.

To run the PoC, follow the example bellow:

```
# build the client and server applications
make

# run the server in the background
# the server is currently listening on port 8080
./bin/server &

# start the client application with the command to execute (Ex. ls -l)
./bin/process-wrapper ls -l
```

