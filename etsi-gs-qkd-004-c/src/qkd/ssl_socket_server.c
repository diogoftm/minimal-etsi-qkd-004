#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "qkd/ssl_socket.h"
#include "qkd/interfaces.h"
#include "qkd/serializers.h"
#include "qkd/sizes.h"
#include "qkd/ssl_socket_server.h"
#include "memory_free_utils.h"


void qkd_servlet(SSL *client_ssl_socket, void *app_context_data, int *server_status) {
    /* Prints connection INFO */
    int client_socket = SSL_get_fd(client_ssl_socket);
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    getpeername(client_socket, (struct sockaddr *) &addr, &len);

    // Header unpack
    socket_data_t header_data = socket_ssl_recv(client_ssl_socket, sizeof(qkd_header_t));
    void *header_stream = header_data.data;
    qkd_header_t header = qkd_header_from_stream(&header_stream);
    if (header_data.data != NULL) {
        free(header_data.data);
        header_data.data = NULL;
    }

#ifdef QKD_LOG_INFO
    fprintf(stdout, "[INFO] Client connection on \"%s:%d\" using %s with cipher suit %s: ",
            inet_ntoa(addr.sin_addr), ntohs(addr.sin_port),
            SSL_get_version(client_ssl_socket), SSL_get_cipher (client_ssl_socket));
    switch (header.service) {
        case QKD_SERVICE_OPEN_CONNECT_REQUEST:
            fprintf(stdout, "OPEN_CONNECT");
            break;
        case QKD_SERVICE_GET_KEY_REQUEST:
            fprintf(stdout, "GET_KEY");
            break;
        case QKD_SERVICE_CLOSE_REQUEST:
            fprintf(stdout, "CLOSE");
            break;
    }
    fprintf(stdout, "\n");
#endif

    if (header.service == QKD_SERVICE_CLOSE_SERVER) {  // Close server
        *server_status = SERVER_FINISHED;
        return;
    }

    // Payload unpack
    void *payload = socket_ssl_recv(client_ssl_socket, header.payload_size).data;
    void *payload_stream = payload;

    qkd_header_t response_header = {QKD_004_VERSION, 0, 0};
    uint8_t *response_buf = NULL, *response_stream = NULL;
    size_t response_size = sizeof(qkd_header_t);

    if (header.service == QKD_SERVICE_OPEN_CONNECT_REQUEST) {
        response_header.service = QKD_SERVICE_OPEN_CONNECT_RESPONSE;
        qkd_open_connect_request_t oc_request = qkd_open_connect_request_from_stream(&payload_stream);
        qkd_open_connect_response_t oc_response = qkd_open_connect_struct(oc_request, app_context_data);

        response_header.payload_size = qkd_sizeof_open_connect_response(oc_response);
        response_size += response_header.payload_size;
        response_buf = malloc(response_size);
        response_stream = response_buf;

        qkd_header_to_stream(response_header, (stream_t) &response_stream);
        qkd_open_connect_response_to_stream(oc_response, (stream_t) &response_stream);

        free_qkd_open_connect_request(&oc_request);
        free_qkd_open_connect_response(&oc_response);
    } else if (header.service == QKD_SERVICE_GET_KEY_REQUEST) {
        response_header.service = QKD_SERVICE_GET_KEY_RESPONSE;
        qkd_get_key_request_t gk_request = qkd_get_key_request_from_stream(&payload_stream);
        qkd_get_key_response_t gk_response = qkd_get_key_struct(gk_request, app_context_data);


        response_header.payload_size = qkd_sizeof_get_key_response(gk_response);
        response_size += response_header.payload_size;
        response_buf = malloc(response_size);
        response_stream = response_buf;

        qkd_header_to_stream(response_header, (stream_t) &response_stream);
        qkd_get_key_response_to_stream(gk_response, (stream_t) &response_stream);

        free_qkd_get_key_request(&gk_request);
        free_qkd_get_key_response(&gk_response);
    } else if (header.service == QKD_SERVICE_CLOSE_REQUEST) {
        response_header.service = QKD_SERVICE_CLOSE_RESPONSE;
        qkd_close_request_t cl_request = qkd_close_request_from_stream(&payload_stream);
        qkd_close_response_t cl_response = qkd_close_struct(cl_request, app_context_data);

        response_header.payload_size = qkd_sizeof_close_response(cl_response);
        response_size += response_header.payload_size;
        response_buf = malloc(response_size);
        response_stream = response_buf;

        qkd_header_to_stream(response_header, (stream_t) &response_stream);
        qkd_close_response_to_stream(cl_response, (stream_t) &response_stream);
    } else {
        fprintf(stdout, "Error: qkd service unknown.\n");
    }
    if (payload != NULL) free(payload);


    // Send response
    socket_ssl_send(client_ssl_socket, (socket_data_t) {response_size, response_buf});
    if (response_buf != NULL) free(response_buf);
}

void qkd_socket_server_init(const char *host, uint16_t port, const char *cert_pem, const char *key_pem,
                            const char *ca_pem, int max_proto_version, void *server_context_data) {
    socket_server_init(host, port, cert_pem, key_pem, ca_pem, max_proto_version, qkd_servlet, server_context_data);
}
