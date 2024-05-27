#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "qkd/models.h"
#include "qkd/ssl_socket_client.h"
#include "write_key_to_file.h"

void getKey(char keyType)
{
    char *cert_pem = "../ssl/127.0.0.1.pem", *key_pem = "../ssl/127.0.0.1.key";
    char *peer_ca_pem = NULL;
    char *host = "127.0.0.1";
    int port = 25575;

    int max_proto_version;
    max_proto_version = 0;

    qkd_client_context_data_t cfg = {host, port, cert_pem, key_pem, peer_ca_pem, max_proto_version};

    // --- OPEN_CONNECT ---
    char *source = "qkd//app1@aaaaaaaa-aaaa-aaaa-aaaa-aaaaaaaaaaaa";
    char *destination = "qkd//app2@bbbbbbbb-bbbb-bbbb-bbbb-bbbbbbbbbbbb";
    qkd_qos_t qos = {8192, 32, 32, 0, 0, 0, 0, "application/json"};
    qkd_open_connect_request_t oc_request = {source, destination, qos, {}};
    uuid_parse("00000000-0000-0000-0000-000000000000", oc_request.key_stream_id);
    qkd_open_connect_response_t oc_response = qkd_open_connect_struct(oc_request, &cfg);

    char *source_ = "qkd//app2@bbbbbbbb-bbbb-bbbb-bbbb-bbbbbbbbbbbb";
    char *destination_ = "qkd//app1@aaaaaaaa-aaaa-aaaa-aaaa-aaaaaaaaaaaa";
    qkd_qos_t qos_ = {8192, 32, 32, 0, 0, 0, 0, "application/json"};
    qkd_open_connect_request_t oc_request_ = {source_, destination_, qos_, {}};
    memcpy(oc_request_.key_stream_id, oc_response.key_stream_id, 16);
    qkd_open_connect_response_t oc_response_ = qkd_open_connect_struct(oc_request_, &cfg);

    for (int index = 0; index < 1; ++index)
    {
        // --- GET_KEY ---
        qkd_get_key_request_t gk_request = {{}, index, (qkd_metadata_t){1, (void *)&keyType}};
        uuid_copy(gk_request.key_stream_id, oc_response.key_stream_id);
        qkd_get_key_response_t gk_response = qkd_get_key_struct(gk_request, &cfg);
        write_key_to_file_bit_format(gk_response, "test.key");

        qkd_get_key_request_t gk_request_ = {{}, index, (qkd_metadata_t){1, (void *)&keyType}};
        uuid_copy(gk_request_.key_stream_id, oc_response_.key_stream_id);
        qkd_get_key_response_t gk_response_ = qkd_get_key_struct(gk_request_, &cfg);
        write_key_to_file_bit_format(gk_response_, "test_.key");
    }
    // --- CLOSE ---
    qkd_close_request_t cl_request = {.key_stream_id = {}};
    uuid_copy(cl_request.key_stream_id, oc_response.key_stream_id);
    qkd_close_response_t cl_response = qkd_close_struct(cl_request, &cfg);

    qkd_close_request_t cl_request_ = {.key_stream_id = {}};
    uuid_copy(cl_request_.key_stream_id, oc_response_.key_stream_id);
    qkd_close_response_t cl_response_ = qkd_close_struct(cl_request_, &cfg);
}

int main(int argc, char *argv[])
{

    //getKey(0); // Get a symmetric key for alice and bob
    getKey(1); // Get a oblivious key for alice and bob

    return 0;
}
