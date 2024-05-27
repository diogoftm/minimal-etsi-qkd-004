#ifndef ETSI_GS_QKD_004_SSL_SOCKET_SERVER_H
#define ETSI_GS_QKD_004_SSL_SOCKET_SERVER_H

#ifdef __cplusplus__
extern "C" {
#endif

void qkd_socket_server_init(const char *host, uint16_t port, const char *cert_pem, const char *key_pem,
                            const char *ca_pem, int max_proto_version, void *server_context_data);

#ifdef __cplusplus__
}
#endif

#endif //ETSI_GS_QKD_004_SSL_SOCKET_SERVER_H
