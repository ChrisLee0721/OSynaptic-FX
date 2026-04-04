#ifndef OSFX_SOLIDITY_H
#define OSFX_SOLIDITY_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int osfx_b62_encode_i64(long long value, char* out, size_t out_len);
long long osfx_b62_decode_i64(const char* s, int* ok);

uint8_t osfx_crc8(const uint8_t* data, size_t len, uint16_t poly, uint8_t init);
uint16_t osfx_crc16_ccitt(const uint8_t* data, size_t len, uint16_t poly, uint16_t init);

#ifdef __cplusplus
}
#endif

#endif

