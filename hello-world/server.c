#include <stdio.h>
#include <stdlib.h>
#include <uv.h>
#include <unistd.h>

#define PORT 1234
#define BACKLOG 10

void close_client(uv_handle_t *handle) {
    free(handle);
}

void on_write(uv_write_t *req, int status) {
    if (status < 0) {
        fprintf(stderr, "Write failed: %s\n", uv_strerror(status));
    }

    uv_close((uv_handle_t*)req->handle, close_client);
    free(req);
}

void new_connection(uv_stream_t *server, int status) {
    if (status < 0) {
        fprintf(stderr, "New connection error: %s\n", uv_strerror(status));
        return;
    }

    uv_tcp_t *client = malloc(sizeof(*client));
    memset(client, 0, sizeof(*client));

    uv_tcp_init(server->loop, client);

    if (uv_accept(server, (uv_stream_t*) client) == 0) {
        uv_write_t *req = malloc(sizeof(*req));
        memset(req, 0, sizeof(*req));

        char *s = "Hello, World!\n";
        uv_buf_t bufs[] = {uv_buf_init(s, (unsigned int)strlen(s))};

        uv_write((uv_write_t*)req, (uv_stream_t*)client, bufs, 1, on_write);
    }
    else {
        uv_close((uv_handle_t*)client, close_client);
    }

}

int main(void) {
    uv_loop_t *loop;

    loop = malloc(sizeof(uv_loop_t));
    uv_loop_init(loop);

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    uv_tcp_t server;
    uv_tcp_init(loop, &server);

    uv_ip4_addr("0.0.0.0", PORT, &addr);

    uv_tcp_bind(&server, (const struct sockaddr*)&addr, 0);
    int ret = uv_listen((uv_stream_t*) &server, BACKLOG, new_connection);
    if (ret) {
        fprintf(stderr, "Listen error %s\n", uv_strerror(ret));
        return 1;
    }

    uv_run(loop, UV_RUN_DEFAULT);

    uv_loop_close(loop);
    free(loop);
    return 0;
}
