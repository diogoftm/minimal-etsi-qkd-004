#include "qkd/copy.h"
#include "qkd/interfaces.h"
#include "qkd/models.h"
#include "qkd/ssl_socket_server.h"
#include "qkd/sizes.h"
#include "memory_free_utils.h"

#ifdef RAND_MAX
#undef RAND_MAX
#define RAND_MAX = 255
#endif

typedef struct qkd_ksid_info
{
    uuid_t key_stream_id;
    qkd_qos_t qos;
    qkd_uri_t source;
    qkd_uri_t destination;
    int current_index;
    struct qkd_ksid_info *ksid_info_next;
} qkd_ksid_info_t;

typedef struct server_context_data_example
{
    int key_rate;
    float qber;
    char *example_db_name;
    uuid_t node_uuid;
    qkd_ksid_info_t *ksid_info_linked_list;
} server_context_data_example_t;

uint32_t simple_hash(const char *str)
{
    uint32_t hash = 0;
    while (*str)
    {
        hash = (hash << 5) + hash + (uint8_t)(*str);
        str++;
    }
    return hash;
}

qkd_key_buffer_t gen_pseudorandom_key(qkd_uri_t source, qkd_uri_t destination, size_t key_size, uint32_t index)
{
    uint32_t seed = 0;
    seed ^= simple_hash(source);
    seed ^= simple_hash(destination);
    seed ^= index;

    qkd_key_buffer_t key = {key_size, malloc(key_size)};
    if (key.data == NULL)
    {
        key.size = 0;
        return key;
    }

    srand(seed);

    for (size_t byte = 0; byte < key_size; ++byte)
    {
        ((uint8_t *)key.data)[byte] = rand() % 256;
    }
    return key;
}

qkd_key_buffer_t gen_oblivious_pseudorandom_key(qkd_uri_t source, qkd_uri_t destination, size_t key_size, uint32_t index, uint8_t oblivious_key_type)
{
    uint32_t seed = 0;
    seed ^= simple_hash(source);
    seed ^= simple_hash(destination);
    seed ^= index;

    qkd_key_buffer_t key = {key_size, malloc(key_size)};
    if (key.data == NULL)
    {
        key.size = 0;
        return key;
    }

    srand(seed);

    for (size_t byte = 0; byte < key_size; ++byte)
    {
        ((uint8_t *)key.data)[byte] = rand() % 256;
    }

    if (oblivious_key_type == 2)
    {
        for (size_t i = 0; i < key_size; i++)
        {
            if (i % 2 == 0)
                ((uint8_t *)key.data)[i] = ((uint8_t *)key.data)[i] | 0xAA;
            else
                ((uint8_t *)key.data)[i] = (((uint8_t *)key.data)[i] >> 1) & 0x55;
        }
    }

    return key;
}

void qkd_open_connect(qkd_uri_t source, qkd_uri_t destination, qkd_qos_t *qos, uuid_t *key_stream_id,
                      qkd_status_t *status, void *app_context_data)
{
    *qos = *qos;
    *status = qkd_status_code_successful;

    srand(time(NULL));
    server_context_data_example_t *config = app_context_data;
    qkd_ksid_info_t *ksid_info_list = config->ksid_info_linked_list;

    if (uuid_compare(*key_stream_id, (uuid_t){}) == 0) // If KSID is 00000000-0000-0000-0000-000000000000
    {
        while (ksid_info_list != NULL)
        {
            if (strcmp(ksid_info_list->source, source) == 0 && strcmp(ksid_info_list->destination, destination) == 0)
            {
                uuid_copy(*key_stream_id, ksid_info_list->key_stream_id);
                return;
            }
            ksid_info_list = ksid_info_list->ksid_info_next;
        }

        // A real KMS shall block here until the other App makes the same request but
        // since there's no SDN Agent in this simulation setup, the request is answered right away

        uuid_generate_random(*key_stream_id);
        qkd_ksid_info_t *new_ksid_info = (qkd_ksid_info_t *)calloc(1, sizeof(qkd_ksid_info_t));
        uuid_copy(new_ksid_info->key_stream_id, *key_stream_id);
        qkd_copy_qos(&new_ksid_info->qos, qos);
        qkd_copy_uri(&new_ksid_info->source, &source);
        qkd_copy_uri(&new_ksid_info->destination, &destination);
        new_ksid_info->current_index = 0;
        new_ksid_info->ksid_info_next = NULL;

        if (ksid_info_list == NULL)
            config->ksid_info_linked_list = new_ksid_info; // Insert as the first element of the linked list
        else
            ksid_info_list->ksid_info_next = new_ksid_info; // Insert at the end of the linked list
    }
}

void qkd_get_key(uuid_t key_stream_id, uint32_t *index, qkd_key_buffer_t *key_buffer, qkd_metadata_t *metadata,
                 qkd_status_t *status, void *app_context_data)
{
    // Use or modify app_context_data
    server_context_data_example_t *config = app_context_data;

    char device_info[SSL3_RT_MAX_PLAIN_LENGTH];

    qkd_ksid_info_t *ksid_info_list = config->ksid_info_linked_list;
    while (ksid_info_list != NULL)
    {
        if (uuid_compare(key_stream_id, ksid_info_list->key_stream_id) == 0)
            break;
        ksid_info_list = ksid_info_list->ksid_info_next;
    }

    if (ksid_info_list == NULL)
    { // KSID not found
        *status = qkd_status_code_gk_peer_not_connected;
        return;
    }

    size_t key_size = ksid_info_list->qos.key_chunk_size;

    qkd_key_buffer_t key_data;
    qkd_key_info_t *meta_value = (qkd_key_info_t *)metadata->data;
    printf("%s %s %i\n", ksid_info_list->source, ksid_info_list->destination, *index);
    if (meta_value->key_type == qkd_key_type_symmetric)
    {
        key_data = gen_pseudorandom_key(ksid_info_list->source, ksid_info_list->destination, key_size,
                                        *index);
    }
    else
    {
        if (meta_value->key_role == 16)
        {
            key_data = gen_oblivious_pseudorandom_key(ksid_info_list->source, ksid_info_list->destination, key_size,
                                                      *index, 1);
        }
        else
            key_data = gen_oblivious_pseudorandom_key(ksid_info_list->source, ksid_info_list->destination, key_size,
                                                      *index, 2);
    }
    sprintf(device_info, "{\"device_info\": [\"key_rate\": %d, \"qber\": %f]}", config->key_rate, config->qber);
    size_t data_size = qkd_sizeof_str(device_info);
    *metadata = (qkd_metadata_t){data_size, malloc(data_size)};
    memcpy((*metadata).data, device_info, data_size);
    *key_buffer = key_data;
    ksid_info_list->current_index++;
    *status = qkd_status_code_successful;
}

void qkd_close(uuid_t key_stream_id, qkd_status_t *status, void *app_context_data)
{
    // Use or modify app_context_data
    server_context_data_example_t *config = app_context_data;
    qkd_ksid_info_t *ksid_info_list = config->ksid_info_linked_list, *ksid_info_prev = NULL;

    // ...

    *status = qkd_status_code_successful;
}

int main(int argc, char *argv[])
{
    /**
     * Note: before implementing the main remember implementing open_connect, get_key and qkd_close is Mandatory
     */

    // Certificate sending is MANDATORY for server side
    char *cert_pem = "../ssl/127.0.0.1.pem", *key_pem = "../ssl/127.0.0.1.key";

    /* Activate/deactivate client cert verification */
    char *peer_ca_pem;
    //    peer_ca_pem = "../ssl/root_CA.pem";    // Client verification enabled
    peer_ca_pem = NULL; // Client verification disabled

    // Optional (modify server_context_data_example_t to fit your preferences or set it to NULL)
    server_context_data_example_t config = {32768, 0.49f, "database_file.db", {}, NULL};
    uuid_generate_random(config.node_uuid);

    int max_proto_version;
    //    max_proto_version = TLS1_2_VERSION;
    max_proto_version = 0;

    qkd_socket_server_init("127.0.0.1", 25575, cert_pem, key_pem, peer_ca_pem, max_proto_version, &config);
    return 0;
}
