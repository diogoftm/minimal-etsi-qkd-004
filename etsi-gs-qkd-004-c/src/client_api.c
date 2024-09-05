#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "qkd/models.h"
#include "qkd/ssl_socket_client.h"
#include "write_key_to_file.h"

char* openConnect(char* certPem, char* keyPem, char* host, int port, char* myUuid, char* peerUuid,
                                        char* ksid, int keyLength)
{
    qkd_client_context_data_t cfg = {host, port, certPem, keyPem, NULL, 0};
    qkd_qos_t qos = {keyLength, 32, 32, 0, 0, 0, 0, "application/json"};
    qkd_open_connect_request_t oc_request = {myUuid, peerUuid, qos, {}};

    if (ksid == NULL) {
        ksid = strdup("00000000-0000-0000-0000-000000000000");
        if (ksid == NULL) {
            return NULL;
        }
    }

    if (uuid_parse(ksid, oc_request.key_stream_id) != 0) {
        free(ksid);
        return NULL;
    }

    qkd_open_connect_response_t oc_response = qkd_open_connect_struct(oc_request, &cfg);
    if (&oc_response == NULL) {
        free(ksid);
        return NULL;
    }

    char* result_key_stream_id = ksid;
    if (oc_response.key_stream_id != NULL){
        result_key_stream_id = malloc(UUID_STR_LEN + 1);
        uuid_unparse(oc_response.key_stream_id, result_key_stream_id);
    }

    return result_key_stream_id;
}

qkd_get_key_response_t getKey(char* certPem, char* keyPem, char* host, int port, char keyType, int index, char* ksid)
{
    qkd_client_context_data_t cfg = {host, port, certPem, keyPem, NULL, 0};
    qkd_key_info_t key_info = {qkd_key_type_symmetric, qkd_key_type_role_default};
    
    qkd_get_key_request_t gk_request = {{}, index, (qkd_metadata_t){sizeof(qkd_key_info_t), (void*)&key_info}};

    if (uuid_parse(ksid, gk_request.key_stream_id) != 0) {
        qkd_get_key_response_t error_response = {};
        return error_response;
    }

    qkd_get_key_response_t gk_response = qkd_get_key_struct(gk_request, &cfg);
    if (&gk_response == NULL) {
        qkd_get_key_response_t error_response = {};
        return error_response;
    }

    return gk_response;
}

qkd_close_response_t close(char* certPem, char* keyPem, char* host, int port, char* ksid)
{
    qkd_client_context_data_t cfg = {host, port, certPem, keyPem, NULL, 0};
    qkd_close_request_t cl_request = {.key_stream_id = {}};
    uuid_parse(ksid, cl_request.key_stream_id);
    qkd_close_response_t cl_response = qkd_close_struct(cl_request, &cfg);
    return cl_response;
}