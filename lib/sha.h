#ifdef SHA_IMPLEMENTATION
#ifndef SHA_H
#define SHA_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char base64_to_char(int x)
{
    if (x < 26) return x + 'A';
    if (x < 52) return x - 26 + 'a';
    if (x < 62) return x - 52 + '0';
    if (x == 62) return '+';
    return '/';
}

static int hex_to_int(char c)
{
    if ('0' <= c && c <= '9') return c - '0';
    if ('a' <= c && c <= 'f') return c - 'a' + 10;
    return 0;
}

static int leftrotate(int x, int amt)
{
    return (x<<amt)+((x>>(32-amt))&((1<<amt)-1));
}

static char* hex_to_base64(char* m, int len)
{
    char* res;
    int res_length, i, j, k, bit, idx, num;
    res_length = ((len-1)/3+1)*2;
    res = malloc((res_length+1) * sizeof(char));
    res[res_length] = '\0';
    for (bit = i = 0; i < res_length; i++) {
        num = 0;
        for (j = 0; j < 6; j++) {
            idx = bit>>2;
            if (idx >= len) break;
            k = hex_to_int(m[idx]);
            num |= (k>>(3-(bit&3))&1)<<(5-j);
            bit++;
        }
        res[i] = base64_to_char(num);
    }
    switch ((len<<1)%3) {
        case 1:
            res[res_length-2] = '=';
        case 2:
            res[res_length-1] = '=';
    }
    return res;
}

static char* sha1_utf8(char* message, int message_length)
{
    unsigned long long ml;
    unsigned char* m;
    char* res;
    unsigned int w[80];
    unsigned int bit, num_bits, idx;
    unsigned int h0, h1, h2, h3, h4;
    unsigned int a, b, c, d, e, f, k, tmp;
    int i, j, len, mod;

    ml = message_length<<3;
    mod = (-64-1-ml)%512;
    num_bits = ml + 1 + mod + 64;
    len = num_bits>>3;
    m = calloc(len+1, sizeof(char));
    for (i = 0; i < message_length; i++)
        m[i] = message[i];
    bit = message_length<<3;
    idx = bit>>3;
    m[idx] |= 1<<(7-(bit&7));
    bit++;
    for (i = 0; i < 8; i++)
        m[len-i-1] = (ml>>(i<<3))&0xFF;

    h0 = 0x67452301;
    h1 = 0xEFCDAB89;
    h2 = 0x98BADCFE;
    h3 = 0x10325476;
    h4 = 0xC3D2E1F0;
    for (i = 0; i < len; i+=64, m+=64) {
        for (j = 0; j < 64; j+=4)
            w[j>>2] = (m[j]<<24)+(m[j+1]<<16)+(m[j+2]<<8)+m[j+3];
        for (j = 16; j < 80; j++)
            w[j] = leftrotate(w[j-3]^w[j-8]^w[j-14]^w[j-16], 1);
        a = h0;
        b = h1;
        c = h2;
        d = h3;
        e = h4;
        for (j = 0; j < 80; j++) {
            if (j < 20) {
                f = (b&c)|((~b)&d);
                k = 0x5A827999;
            } else if (j < 40) {
                f = b^c^d;
                k = 0x6ED9EBA1;
            } else if (j < 60) {
                f = (b&c)|(b&d)|(c&d);
                k = 0x8F1BBCDC;
            } else {
                f = b^c^d;
                k = 0xCA62C1D6;
            }
            tmp = leftrotate(a, 5) + f + e + k + w[j];
            e = d;
            d = c;
            c = leftrotate(b, 30);
            b = a;
            a = tmp;
        }
        h0 = h0 + a;
        h1 = h1 + b;
        h2 = h2 + c;
        h3 = h3 + d;
        h4 = h4 + e;
    }
    free(m-len);
    res = malloc(41 * sizeof(char));
    sprintf(res,"%08x%08x%08x%08x%08x",h0,h1,h2,h3,h4);
    return res;
}

#endif
#endif
