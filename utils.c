#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "programmer.h"
#include "utils.h"

// `size` is number of chars in codeplug.
void codeplug_str(uint16_t *dest, int dest_n, char *src, int src_n) {
    memset(dest, 0, dest_n*2);
    for(int i = 0; i < dest_n; i++) {
        dest[i] = (i < src_n) ? src[i] : 0;
    }
}

void codeplug_str_print(uint16_t *str) {
    for(int i = 0; i < 16; i++) {
        if(str[i] == 0)
            break;
        putc(str[i], stdout);
    }
    putc('\n', stdout);
}

void codeplug_id(uint8_t *dest, uint32_t id) {
    dest[0] = (id >>  0) & 0xFF;
    dest[1] = (id >>  8) & 0xFF;
    dest[2] = (id >> 16) & 0xFF;
}

int id_to_int(uint8_t *id) {
    return (*(uint32_t *)id) & 0x00FFFFFF;
}

int codeplug_strcmp(uint16_t *str1, const char *str2) {
    for(int i = 0; i < 16; i++) {
        if(*str1 != *str2)
            return 0;
        str1++; str2++;
    }
    return 1;
}

int bcd_freq_to_int(uint32_t bcd) {
    int freq = 0;
    int place = 1;

    for(int i = 0; i < 2*sizeof(bcd); i++) {
        freq += (bcd & 0xF)*place;
        place *= 10;
        bcd >>= 4;
    }

    return freq*10;
}