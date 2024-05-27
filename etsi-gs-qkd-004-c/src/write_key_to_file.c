#include "write_key_to_file.h"

#include <stdio.h>

int number_of_digits(uint32_t n) {
    if (n == 0)
        return 1;
    
    int digits = 0;
    while(n!= 0) {
        n /= 10;
        ++digits;
    }
     
    return digits;
}

void write_key_to_file_bit_format(const qkd_get_key_response_t get_key_response, const char* file_name) {
    FILE *key_file = fopen(file_name, "a");
    char sindex[11];

    uint32_t key_byte;

    //Write index
    fputs("Index: ", key_file);
    sprintf(sindex, "%d", get_key_response.index);
    fwrite(sindex, sizeof(char), number_of_digits(get_key_response.index), key_file);

    //Write key
    fputs("\nKey: ", key_file);
    for(key_byte = 0; key_byte < get_key_response.key_buffer.size; ++key_byte){
        uint8_t byte = ((uint8_t*) get_key_response.key_buffer.data)[key_byte];
        for (int bit = 7; bit >= 0; --bit) {
            (byte & (1 << bit)) ? fputc('1', key_file) : fputc('0', key_file);
        }
    }
    fputs("\n\n", key_file);

    fclose(key_file);
}