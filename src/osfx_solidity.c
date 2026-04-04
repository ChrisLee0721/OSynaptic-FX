#include "../include/osfx_solidity.h"

static const char OSFX_B62[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

int osfx_b62_encode_i64(long long value, char* out, size_t out_len) {
    unsigned long long n;
    char buf[96];
    size_t idx = 0;
    size_t w = 0;

    if (!out || out_len < 2) {
        return 0;
    }
    if (value == 0) {
        out[0] = '0';
        out[1] = '\0';
        return 1;
    }

    n = (value < 0) ? (unsigned long long)(-value) : (unsigned long long)value;
    while (n > 0 && idx < sizeof(buf) - 1) {
        buf[idx++] = OSFX_B62[n % 62ULL];
        n /= 62ULL;
    }
    if (idx == 0) {
        return 0;
    }
    if (idx + ((value < 0) ? 1 : 0) + 1 > out_len) {
        return 0;
    }
    if (value < 0) {
        out[w++] = '-';
    }
    while (idx > 0) {
        out[w++] = buf[--idx];
    }
    out[w] = '\0';
    return 1;
}

long long osfx_b62_decode_i64(const char* s, int* ok) {
    unsigned long long val = 0;
    const char* p = s;
    int neg = 0;

    if (ok) {
        *ok = 0;
    }
    if (!s || !*s) {
        return 0;
    }
    if (*p == '-') {
        neg = 1;
        p++;
        if (!*p) {
            return 0;
        }
    }
    while (*p) {
        unsigned char c = (unsigned char)*p;
        int d = -1;
        if (c >= '0' && c <= '9') {
            d = (int)(c - '0');
        } else if (c >= 'a' && c <= 'z') {
            d = 10 + (int)(c - 'a');
        } else if (c >= 'A' && c <= 'Z') {
            d = 36 + (int)(c - 'A');
        } else {
            return 0;
        }
        val = val * 62ULL + (unsigned long long)d;
        p++;
    }
    if (ok) {
        *ok = 1;
    }
    return neg ? -(long long)val : (long long)val;
}

uint8_t osfx_crc8(const uint8_t* data, size_t len, uint16_t poly, uint8_t init) {
    uint8_t crc = init;
    size_t i;
    int j;
    if (!data || len == 0) {
        return crc;
    }
    for (i = 0; i < len; ++i) {
        crc ^= data[i];
        for (j = 0; j < 8; ++j) {
            if (crc & 0x80U) {
                crc = (uint8_t)(((crc << 1) ^ (uint8_t)poly) & 0xFFU);
            } else {
                crc = (uint8_t)((crc << 1) & 0xFFU);
            }
        }
    }
    return crc;
}

uint16_t osfx_crc16_ccitt(const uint8_t* data, size_t len, uint16_t poly, uint16_t init) {
    uint16_t crc = init;
    size_t i;
    int j;
    if (!data || len == 0) {
        return crc;
    }
    for (i = 0; i < len; ++i) {
        crc ^= (uint16_t)(data[i] << 8);
        for (j = 0; j < 8; ++j) {
            if (crc & 0x8000U) {
                crc = (uint16_t)(((crc << 1) ^ poly) & 0xFFFFU);
            } else {
                crc = (uint16_t)((crc << 1) & 0xFFFFU);
            }
        }
    }
    return crc;
}

