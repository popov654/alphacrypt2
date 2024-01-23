#ifndef ACP_H
#define ACP_H

#ifndef MD5_H
#include <stdio.h>
#include <stdlib.h>
#endif

#include <stdint.h>
#include <string.h>
#include <time.h>

#ifndef min
#define min(a,b)    (((a) < (b)) ? (a) : (b))
#endif

#ifndef max
#define max(a,b)    (((a) > (b)) ? (a) : (b))
#endif

void acp_init();
void filterChars(char *str);
char* strint(const char* s);
char* intstr(const char* i);
char* strreverse(char* s);
char* toBase64(char* b, int binary);
char* toBase64s(char* b);
char* toBase64b(char* b);
char* hextob64(char* s, int limit);

typedef char* (*hashfc) (const char*);

typedef struct{
    char* key;
    char** weak_value;
    char** strong_value;
}MapEntry;

extern char l64[];

char* acraw(char* s, const char* k, char* out, hashfc f, int strong, char seed, long from, long step, int binary);
char* acraws(char* s, const char* k, char* out, hashfc f, int strong, char seed, long from, long step);
char* acrawb(char* s, const char* k, char* out, hashfc f, int strong, char seed, long from, long step);
char* acraws_basic(char* s, const char* k, char* out, hashfc f, int strong, char seed);

char* crypt(char* s, const char* k, char* out, hashfc func, int on, int weak, int noseed);
char* bcrypt(char* s, long size, const char* k, char* out, hashfc func, int on, int weak, int noseed);
void bcrypt_file(char* inputfile, char* outputfile, const char* k, hashfc func, int on, int weak, int noseed);

#endif