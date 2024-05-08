#include <stdio.h>
#include <signal.h>
#include "rc522_controller.h"


void intHandler(int dummy) {
    cleanup();
    exit(0);
}

void print_array(uint8_t *arr, uint8_t size) {
    for (size_t i = 0; i < size; i++)
    {
        printf("arr[%u] = %u; ", i, arr[i]);
    }
    printf("\n");
}

void print_as_char(uint8_t *arr, uint8_t size) {
    for (size_t i = 0; i < size; i++)
    {
        printf("%c", arr[i]);
    }
    printf("|\n");
}

rc522_status read_data_from_card(uint8_t *key, uint8_t key_size, uint8_t **buffer, uint8_t *buffer_size) {

    uint8_t block_addres[3] = {8, 9, 10};

    while(1) {
        rc522_status status;
        uint8_t *req_res, *uid, *select_tag_res, *auth_res, *block_data;
        uint8_t res_size = 0, res_size_bits = 0;
        
        
        status = req_a_picc(&req_res, &res_size, &res_size_bits);
        if(status != RC522_OK) return status;

        res_size = 0, res_size_bits = 0;
        status = anticollision(&uid, &res_size, &res_size_bits);
        if(status != RC522_OK) return status;

        printf("id: %llu\n", convert_uid_to_number(uid));

        res_size = 0, res_size_bits = 0;
        status = select_tag(&select_tag_res, &res_size, &res_size_bits, uid);
        if(status != RC522_OK) return status;

        res_size = 0, res_size_bits = 0;
        status = authenticate(&auth_res, &res_size, &res_size_bits, PICC_AUTHENT1A, 11, key, 6, uid);
        if(status != RC522_OK) return status;

        *buffer_size = 0;
        *buffer = malloc(16*3);
        for (uint8_t i = 0; i < 3; i++)
        {
            uint8_t *block_data;
            uint8_t block_res_size = 0, block_res_size_bits = 0;
            uint8_t *buffer_pointer = (*buffer) + (i * 16);

            block_res_size = 0, block_res_size_bits = 0;
            status = read_block(&block_data, &block_res_size, &block_res_size_bits, block_addres[i]);

            if(status != RC522_OK) {
                *buffer_size = 0;
                free(*buffer);
                free(block_data);
                free(req_res);
                free(uid);
                free(select_tag_res);
                stop_authentication();
                return status;
            }

            memcpy(buffer_pointer, block_data, block_res_size);
            *buffer_size += block_res_size;
            free(block_data);
        }
        free(req_res);
        free(uid);
        free(select_tag_res);
        stop_authentication();
        return RC522_OK;
        
    }
}

rc522_status write_data_from_card(uint8_t *key, uint8_t key_size, uint8_t *buffer, uint8_t buffer_size) {

    uint8_t block_addres[3] = {8, 9, 10};

    while(1) {
        rc522_status status;
        uint8_t *req_res, *uid, *select_tag_res, *auth_res, *block_data;
        uint8_t res_size = 0, res_size_bits = 0;
        
        
        status = req_a_picc(&req_res, &res_size, &res_size_bits);
        if(status != RC522_OK) return status;

        res_size = 0, res_size_bits = 0;
        status = anticollision(&uid, &res_size, &res_size_bits);
        if(status != RC522_OK) return status;

        printf("id: %llu\n", convert_uid_to_number(uid));

        res_size = 0, res_size_bits = 0;
        status = select_tag(&select_tag_res, &res_size, &res_size_bits, uid);
        if(status != RC522_OK) return status;

        res_size = 0, res_size_bits = 0;
        status = authenticate(&auth_res, &res_size, &res_size_bits, PICC_AUTHENT1A, 11, key, 6, uid);
        if(status != RC522_OK) return status;

        uint8_t max_data_access = buffer_size / 16;
        uint8_t complete_data = max_data_access > 3;
        max_data_access = complete_data ? 3 : max_data_access;
        for (uint8_t i = 0; i <= max_data_access; i++)
        {
            uint8_t *block_data;
            uint8_t block_res_size = 0, block_res_size_bits = 0;
            uint8_t *buffer_pointer = buffer + i*16;

            block_res_size = 0, block_res_size_bits = 0;
            if(i == max_data_access && !complete_data) {
                status = write_block(buffer_pointer, buffer_size % 16, &block_data, &block_res_size, &block_res_size_bits, block_addres[i]);
            }
            else {
                status = write_block(buffer_pointer, 16, &block_data, &block_res_size, &block_res_size_bits, block_addres[i]);
            }

            if(status != RC522_OK) {
                free(block_data);
                free(req_res);
                free(uid);
                free(select_tag_res);
                stop_authentication();
                return status;
            }

            free(block_data);
        }
        free(req_res);
        free(uid);
        free(select_tag_res);
        stop_authentication();
        return RC522_OK;
        
    }
}

int main(int argc, char const *argv[])
{
    uint8_t *data, data_size;
    uint8_t key[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    uint8_t send_data[26] = {'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a'};
    //printf("Realizando o setup do SPI\n");
    signal(SIGINT, intHandler);
    spi_setup();
    init();

    uint8_t temp = 0;
    while(write_data_from_card(key, 6, send_data, 26) != RC522_OK){}
    print_array(data, data_size);
    cleanup();
    printf("\n");
    //calculate_crc(data, 5, result);
    //self_test();
    printf("Fim\n");
    return 0;
}
