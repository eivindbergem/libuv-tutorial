// Copyright 2015 Eivind Alexander Bergem <eivind.bergem@gmail.com>
//
// Simple libuv hello world TCP server
//
// libuv tutorial is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// libuv tutorial is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with libuv tutorial.  If not, see <http://www.gnu.org/licenses/>.

#include <stdio.h>
#include <stdlib.h>
#include <uv.h>

#define PORT 1234
#define BACKLOG 10

// Close client
void close_client(uv_handle_t *handle) {
    // Free client handle after connection has been closed
    free(handle);
}

// Write callback
void on_write(uv_write_t *req, int status) {
    // Check status
    if (status < 0) {
        fprintf(stderr, "Write failed: %s\n", uv_strerror(status));
    }

    // Close client hanlde
    uv_close((uv_handle_t*)req->handle, close_client);

    // Free request handle
    free(req);
}

// Callback for new connections
void new_connection(uv_stream_t *server, int status) {
    // Check status code, anything under 0 means an error.
    if (status < 0) {
        fprintf(stderr, "New connection error: %s\n", uv_strerror(status));
        return;
    }

    // Create handle for client
    uv_tcp_t *client = malloc(sizeof(*client));
    memset(client, 0, sizeof(*client));
    uv_tcp_init(server->loop, client);

    // Accept new connection
    if (uv_accept(server, (uv_stream_t*) client) == 0) {
        // Create write request handle
        uv_write_t *req = malloc(sizeof(*req));
        memset(req, 0, sizeof(*req));

        // Add a buffer with hello world
        char *s = "Hello, World!\n";
        uv_buf_t bufs[] = {uv_buf_init(s, (unsigned int)strlen(s))};

        // Write and call on_write callback when finished
        int ret = uv_write((uv_write_t*)req, (uv_stream_t*)client, bufs, 1, on_write);

        if (ret < 0) {
            fprintf(stderr, "Write error: %s\n", uv_strerror(ret));
            uv_close((uv_handle_t*)client, close_client);
        }
    }
    else {
        // Accept failed, closing client handle
        uv_close((uv_handle_t*)client, close_client);
    }

}

int main(void) {
    // Initialize the loop
    uv_loop_t *loop;
    loop = malloc(sizeof(uv_loop_t));
    uv_loop_init(loop);

    // Create sockaddr struct
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));

    // Convert ipv4 address and port into sockaddr struct
    uv_ip4_addr("0.0.0.0", PORT, &addr);

    // Set up tcp handle
    uv_tcp_t server;
    uv_tcp_init(loop, &server);

    // Bind to socket
    uv_tcp_bind(&server, (const struct sockaddr*)&addr, 0);

    // Listen on socket, run new_connection() on every new connection
    int ret = uv_listen((uv_stream_t*) &server, BACKLOG, new_connection);
    if (ret) {
        fprintf(stderr, "Listen error: %s\n", uv_strerror(ret));
        return 1;
    }

    // Start the loop
    uv_run(loop, UV_RUN_DEFAULT);

    // Close loop and shutdown
    uv_loop_close(loop);
    free(loop);
    return 0;
}
