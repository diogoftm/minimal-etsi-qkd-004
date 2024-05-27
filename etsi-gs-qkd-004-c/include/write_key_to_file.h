#ifndef WRITE_KEY_TO_FILE_H
#define WRITE_KEY_TO_FILE_H

#include "qkd/models.h"

#ifdef __cplusplus
extern "C" {
#endif

void write_key_to_file_bit_format(const qkd_get_key_response_t get_key_response, const char* file_name);

#ifdef __cplusplus
}
#endif

#endif