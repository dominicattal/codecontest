#ifdef SHA_IMPLEMENTATION
#ifndef SHA_H
#define SHA_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int char_to_base64(char c)
{
    if ('A' <= c && c <= 'Z') return c - 'A';
    if ('a' <= c && c <= 'z') return c - 'a' + 26;
    if ('0' <= c && c <= '9') return c - '0' + 52;
    if (c == '+' || c == '-') return 62;
    return 0;
}

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

static char* utf8_to_base64(char* m, int len)
{
    char* res;
    int res_length, i, j, bit, idx, num;
    res_length = ((len-1) / 3 + 1) * 4;
    res = malloc((res_length+1) * sizeof(char));
    res[res_length] = '\0';
    for (bit = i = 0; i < res_length; i++) {
        num = 0;
        for (j = 0; j < 6; j++) {
            idx = bit>>3;
            if (idx >= len) break;
            num |= (m[idx]>>(7-bit&7)&1)<<(5-j);
            bit++;
        }
        res[i] = base64_to_char(num);
    }
    switch (len%3) {
        case 1:
            res[res_length-2] = '=';
        case 2:
            res[res_length-1] = '=';
    }
    return res;
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
            num |= (k>>(3-bit&3)&1)<<(5-j);
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

static char* base64_to_utf8(char* m, int len)
{
    char* res;
    int res_length, i, j, bit, idx, num;
    if (m[len-1] != '=')
        res_length = (len>>2)*3;
    else if (m[len-2] != '=')
        res_length = ((len>>2)-1)*3+2;
    else
        res_length = ((len>>2)-1)*3+1;
    res = malloc((res_length+1) * sizeof(char));
    res[res_length] = '\0';
    for (bit = i = 0; i < res_length; i++) {
        for (j = 0; j < 8; j++) {
            idx = bit/6;
            num = char_to_base64(m[idx]);
            res[i] |= ((num>>(5-bit%6))&1)<<(7-j);
            bit++;
        }
    }
    return res;
}

static char* sha1_utf8(char* message, int message_length)
{
    unsigned long long ml;
    unsigned char* m;
    char* res;
    unsigned int w[80];
    unsigned int append_one;
    unsigned int i, j, bit, n, num_bits, len, num, idx, res_len;
    unsigned int h0, h1, h2, h3, h4;
    unsigned int a, b, c, d, e, f, k, tmp;
    int mod;

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

static char* base64_concat(char* m1, int n1, char* m2, int n2)
{
    char* res;
    int res_length, num_bits, num_bits1, num_bits2;
    int i, j, bit, num, cur, idx, res_idx;
    num_bits1 = (n1>>2)*3;
    num_bits1 -= m1[n1-1] == '=';
    num_bits1 -= m1[n1-2] == '=';
    num_bits1 *= 8;
    num_bits2 = (n2>>2)*3;
    num_bits2 -= m2[n2-1] == '=';
    num_bits2 -= m2[n2-2] == '=';
    num_bits2 *= 8;
    num_bits = num_bits1+num_bits2;
    res_length = (num_bits+5)/6 + ((num_bits>>2)%3);
    res = malloc((res_length+1) * sizeof(char));
    res[res_length] = '\0';
    cur = res_idx = 0;
    j = 6;
    for (i = 0; i < num_bits1; i++) {
        idx = i/6;
        num = char_to_base64(m1[idx]);
        cur = (cur<<1)|((num>>(5-(i%6)))&1);
        if (--j == 0) {
            res[res_idx++] = base64_to_char(cur);
            cur = 0;
            j = 6;
        }
    }
    for (i = 0; i < num_bits2; i++) {
        idx = i/6;
        num = char_to_base64(m2[idx]);
        cur = (cur<<1)|((num>>(5-(i%6)))&1);
        if (--j == 0) {
            res[res_idx++] = base64_to_char(cur);
            cur = 0;
            j = 6;
        }
    }
    if (num_bits%24 == 16) {
        res[res_idx++] = base64_to_char(cur<<2);
        res[res_idx++] = '=';
    } else if (num_bits%24 == 8) {
        res[res_idx++] = base64_to_char(cur<<4);
        res[res_idx++] = '=';
        res[res_idx++] = '=';
    }
    return res;
}

#endif
#endif
