#ifndef _UTILS_H_
#define _UTILS_H_

#include <inttypes.h>


void codeplug_str(uint16_t *dest, int dest_n, char *src, int src_n);
void codeplug_str_print(uint16_t *str);
void codeplug_id(uint8_t *dest, uint32_t id);
int id_to_int(uint8_t *id);

int bcd_freq_to_int(uint32_t bcd);

#endif