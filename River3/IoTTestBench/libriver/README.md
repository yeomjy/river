# IoT Simulation Infrastructure

This is a PoC that shows how we can simulate a network pipe.

This PoC uses a library implementation to connect to a server (`src/server.c`) and redirect the `stdout` to the server socket.

All that is required to redirect the stdout of a process to the server is to start the process with `LD_PRELOAD=path/to/libriver.so`

This PoC uses a client (`src/process-wrapper.c`) that tipically prints **"Hello"** to `stdout`.
Using the `LD_PRELOAD` loader option we load our custom library that does the redirect.
This approach has the advantage that no changes need to be made to the client source code.

To run the PoC, follow the example bellow:

```
# build the libriver.so library and client and server applications
make

# run the server in the background
# the server is currently listening on port 8080
./bin/server &

# start the client application
LD_PRELOAD=bin/libriver.so ./bin/process-wrapper
```

**Note:** If `patchelf` is installed on the build machine, `LD_PRELOAD` isn't necessary as we use `patchelf` at build time to specify `libriver.so` as a needed library.
