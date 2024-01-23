#include "acp.h"
#include "md5.h"

const char c64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

const int p[] = {3, 5, 7, 11, 13, 17, 19, 23, 29, 37, 41, 43, 47, 53};

int mode = 0;
int default_rounds = 7;

const int CACHE_SIZE = 128;
MapEntry key_cache[128] = { NULL };
int cache_current_size = 0;

char l64[128];

void acp_init() {
    for (int i=0; i<64; i++) {
        l64[c64[i]] = i;
    }
    for (int i = 0; i < CACHE_SIZE; i++) {
        key_cache[i].key = NULL;
        key_cache[i].weak_value = NULL;
        key_cache[i].strong_value = NULL;
    }
}

void filterChars(char *str) {
    int i, j;
    int len = strlen(str);
    for (i = j = 0; i < len; i++) {
        if (str[i] >= 'A' && str[i] <= 'Z' || str[i] >= 'a' && str[i] <= 'z' ||
             str[i] >= '0' && str[i] <= '9' || str[i] == '+' || str[i] == '/') {
            str[j++] = str[i];
        }
    }
    str[j] = '\0';
}

char* strint(const char* s) {
    // s must contain only base-64 characters!
    char* sc = (char*)malloc(strlen(s));
    strcpy(sc, s);
    filterChars(sc);
    char* r = (char*)malloc(strlen(sc));
    char* p = r;
    char n;
    while (*sc != '\0') {
        n = *(sc++);
        if (n > 128) {
            exit(1);
        }
        *(p++) = l64[n];
    }
    return r;
}

char* intstr(const char* i) {
    char* w = (char*)malloc(strlen(i)+1);
    char* p = w;
    char n;
    while (*i != -1) {
        n = *(i++);
        if (n > 128) {
            exit(1);
        }
        *(p++) = c64[n];
    }
    *(p++) = '\0';
    return w;
}

char* strreverse(char* s) {
    char* r = (char*)malloc(strlen(s));
    memcpy(r, s, strlen(s));
    int len = strlen(s);
    for (int i = 0; i < len / 2; i++) {
        char ch = r[i];
        r[i] = r[len-1-i];
        r[len-1-i] = ch;
    }
    *(r+strlen(s)) = '\0';
    return r;
}

char* toBase64(char* b, int binary) {
    char* res = (char*)malloc(binary == 1 ? 1e8 : (binary == 0 ? 1e5 : binary * 3 / 2));
    char* p = b;
    char* r = res;

    int padding;
    int end = binary && *p == EOF || !binary && *p == '\0';
    unsigned char bytes[3];
    long count;

    while (!end) {
        for (int i = 0; i < 3; i++) bytes[i] = 0;
        padding = 0;
        for (int i = 0; i < 3; i++) {
            end = binary == 1 && *p == EOF || binary == 0 && *p == '\0' || binary > 0 && binary == count;
            if (end) {
                padding = i > 0 ? 3-i : 0;
                break;
            }
            bytes[i] = (unsigned char) *(p++);
            count++;
        }
        unsigned int q = (bytes[0] << 16) | (bytes[1] << 8) | bytes[2];
        for (int j = 0; j < 4; j++) {
            *(r++) = c64[(q >> (18 - 6 * j)) & 0x3F];
        }
        end = binary == 1 && *p == EOF || !binary && *p == '\0' || binary > 0 && binary == count;
    }
    char* strend = r;
    for (int i = 0; i < padding; i++) {
        *(--r) = '=';
    }
    *(strend++) = '\0';
    return res;
}

char* toBase64s(char* b) {
    char* res = toBase64(b, 0);
    return res;
}

char* toBase64b(char* b) {
    char* res = toBase64(b, 1); 
    return res;
}

char* hextob64(char* s, int limit) {
    char* res = (char*)malloc(limit+1);
    char* r = res;
    int len = strlen(s);
    char* bytes = (char*)malloc(limit+1);
    char* b = bytes;
    char* p = s;
    unsigned char ch, byte, val;
    for (int i = 0; i < len; i += 2) {
        if (i >= limit*2) break;
        byte = 0;
        for (int j = 0; j < 2; j++) {
            ch = *(p++);
            if (ch >= 'A' && ch <= 'F') {
                val = ch - 'A' + 10;
            } else if (ch >= 'a' && ch <= 'f') {
                val = ch - 'a' + 10;
            } else if (ch >= '0' && ch <= '9') {
                val = ch - '0';
            }
            byte = byte * 16 + val;
        }
        *(b++) = byte;
    }
    *(b++) = -1;
    char* base64 = toBase64(bytes, limit);

    if (limit < strlen(base64)) {
        *(base64 + limit) = '\0';
    }

    return base64;
}

int isCached(const char* key, int type) {
    for (int i = 0; i < CACHE_SIZE; i++) {
        if (key_cache[i].key == NULL) continue;
        if (strcmp(key_cache[i].key, key) == 0) {
            return type == 0 && key_cache[i].weak_value != NULL || type == 1 && key_cache[i].strong_value != NULL;
        }
    }
    return 0;
}

char** getCachedValue(const char* key, int type) {
    for (int i = 0; i < CACHE_SIZE; i++) {
        if (key_cache[i].key == NULL) continue;
        if (strcmp(key_cache[i].key, key) == 0) {
            return type == 0 ? key_cache[i].weak_value : key_cache[i].strong_value;
        }
    }
    return NULL;
}

char** getPair(char** list, int index) {
    if (index >= cache_current_size) return NULL;
    int i = 0;
    while (i++ < index) list += 2;
    return list == NULL ? NULL : list;
}

char* getElement(char** list, int index) {
    int i = 0;
    while (i++ < index) list++;
    return list == NULL ? NULL : *list;
}

void putCachedValueForRound(const char* key, const char* value[], int type, int round) {
    for (int i = 0; i < CACHE_SIZE; i++) {
        if (key_cache[i].key != NULL && strcmp(key_cache[i].key, key) != 0) continue;
        key_cache[i].key = (char*) malloc(strlen(key));
        key_cache[i].key = strcpy(key_cache[i].key, key);

        const unsigned char MAX_ROUNDS = 20;
        
        if (type == 0) {
            if (key_cache[i].weak_value == NULL) {
                key_cache[i].weak_value = (char**) malloc(MAX_ROUNDS * 2 * sizeof(size_t));
                for (int j = 0; j < MAX_ROUNDS * 2; j++) {
                    key_cache[i].weak_value[j] = NULL;
                }
            }
            key_cache[i].weak_value[round * 2] = (char*) malloc(64);
            memcpy(key_cache[i].weak_value[round * 2], value[0], 64);
            key_cache[i].weak_value[round * 2 + 1] = (char*) malloc(strlen(value[1]));
            strcpy(key_cache[i].weak_value[round * 2 + 1], value[1]);
        } else {
            if (key_cache[i].strong_value == NULL) {
                key_cache[i].strong_value = (char**) malloc(MAX_ROUNDS * 2 * sizeof(size_t));
                for (int j = 0; j < MAX_ROUNDS * 2; j++) {
                    key_cache[i].strong_value[j] = NULL;
                }
            }
            key_cache[i].strong_value[round * 2] = (char*) malloc(128);
            memcpy(key_cache[i].strong_value[round * 2], value[0], 128);
            key_cache[i].strong_value[round * 2 + 1] = (char*) malloc(strlen(value[1]));
            strcpy(key_cache[i].strong_value[round * 2 + 1], value[1]);
        }
        cache_current_size++;
        break;
    }
}

void putCachedValue(const char* key, const char* value[], int type) {
    putCachedValueForRound(key, value, type, cache_current_size);
}

void prepareKeys(char* keys[], char* src, hashfc func, int rounds, int strong, int lengths[], int lsize) {
    char* ck_str = src;
    char* original_key = ck_str;
    //char* cur_key = strint(ck_str);
    char* str;
    for (int i = 0; i < rounds; i++) {
        char* cached_key = getElement(getPair(getCachedValue(original_key, strong), i), 0);
        if (cached_key != NULL) {
            keys[i] = cached_key;
            //cur_key = keys[i];
            //char* str = getElement(getPair(getCachedValue(original_key, strong), i), 1);
            //ck_str = (char*) malloc(strlen(str));
            //strcpy(ck_str, str);
            continue;
        }
        keys[i] = malloc(128);
        str = func(ck_str);
        strcat(str, func(strreverse(ck_str)));
        if (strong) {
            long len = strlen(str), q = len / 4;
            char* s = (char*)malloc(len + 1);
            long j;
            for (j = 0; j < q; j++) {
                s[j] = str[3*q + j];
            }
            for (j = q; j < 3*q; j++) {
                s[j] = str[j];
            }
            for (j = 3*q; j < len; j++) {
                s[j] = str[j - 3*q];
            }
            s[j] = '\0';
            strcat(str, func(s));

            for (j = 0; j < q; j++) {
                s[j] = str[2*q + j];
            }
            for (j = q; j < 2*q; j++) {
                s[j] = str[j];
            }
            for (j = 2*q; j < 3*q; j++) {
                s[j] = str[j - 2*q];
            }
            for (j = 3*q; j < len; j++) {
                s[j] = str[j];
            }
            s[j] = '\0';
            strcat(str, func(s));
            ck_str = hextob64(str, lengths[min(i, lsize-1)]);
        } else {
            int index = i == 0 ? 59 : 61;
            ck_str = str;
            *(ck_str + index) = '\0';
        }
        keys[i] = strint(ck_str);
        const char* pair[] = {keys[i], ck_str};
        //printf("%s\n", ck_str);
        putCachedValueForRound(original_key, pair, strong, i);
    }
    free(ck_str);
}


char* acraw(char* s, const char* k, char* out, hashfc func, int strong, char seed, long from, long step, int binary) {
    char* keys[15];
    int rounds = strong ? default_rounds : 2;
    
    char* ck_str = (char*)malloc(strlen(k));
    strcpy(ck_str, k);
    char* original_key = ck_str;
         
    int lengths[] = {47, 53, 59, 61, 67, 71, 73, 79, 83};

    prepareKeys(keys, ck_str, func, rounds, strong, lengths, sizeof(lengths) / sizeof(lengths[0])); 

    // isn't it thrilling?
    int len = strlen(s);
    int keylength;

    char* s0 = (char*)malloc(len);
    strcpy(s0, s);
    
    char* s1 = out;
    
    if (s1 == NULL) {
        s1 = (char*)malloc(len);
        strcpy(s1, s);
    }
         
    for (int i = 0; i < len; i++) {
        if (!binary) {
            char n = l64[s[i]];
            for (int j = 0; j < rounds; j++) {
                keylength = strong ? lengths[j] : (j == 0 ? 59 : 61);
                n ^= keys[j][(from+i*step) % keylength];
            }
            n ^= seed;
            s1[i] = c64[n];
            if (mode == 1 && i > 0) s1[i] = c64[l64[s1[i]] ^ l64[s1[i-1]]];
            if (mode == 2 && i > 0) s1[i] = c64[l64[s1[i]] ^ l64[s0[i-1]]];
        } else {
            for (int j = 0; j < rounds; j++) {
                keylength = strong ? lengths[j] : (j == 0 ? 59 : 61);
                s1[i] = ((unsigned char) s[i]) ^ keys[j][(from+i*step) % keylength];
            }
            s1[i] ^= seed;
            if (mode == 1 && i > 0) s1[i] = ((unsigned char) s1[i]) ^ ((unsigned char) s1[i-1]);
            if (mode == 2 && i > 0) s1[i] = ((unsigned char) s1[i]) ^ ((unsigned char) s0[i-1]);
        }
    }

    free(s0);

    return s1;
}

void acrawf(FILE* input, long size, const char* k, FILE* output, hashfc func, int strong, unsigned char seed, long from, long step) {
    char* keys[15];
    int rounds = strong ? default_rounds : 2;
         
    int lengths[] = {47, 53, 59, 61, 67, 71, 73, 79, 83};

    prepareKeys(keys, k, func, rounds, strong, lengths, sizeof(lengths) / sizeof(lengths[0])); 

    // isn't it thrilling?
    int keylength;
    unsigned char ch, ch1, last_p, last_c;
         
    for (int i = 0; i < size; i++) {
        fread(&ch, sizeof(char), 1, input);
        ch1 = ch;
        for (int j = 0; j < rounds; j++) {
            keylength = strong ? lengths[j] : (j == 0 ? 59 : 61);
            ch = (unsigned char) (ch ^ keys[j][(from+i*step) % keylength]);
        }
        ch = (unsigned char) (ch ^ seed);
        
        if (mode == 1 && i > 0) ch = ch ^ last_c;
        if (mode == 2 && i > 0) ch = ch ^ last_p;

        last_p = ch1;
        last_c = ch;
        fwrite(&ch, sizeof(char), 1, output);
    }
}

char* acraws(char* s, const char* k, char* out, hashfc func, int strong, char seed, long from, long step) {
    return acraw(s, k, out, func, strong, seed, from, step, 0);
}

char* acrawb(char* s, const char* k, char* out, hashfc func, int strong, char seed, long from, long step) {
    return acraw(s, k, out, func, strong, seed, from, step, 1);
}

char* acraws_basic(char* s, const char* k, char* out, hashfc func, int strong, char seed) {
    return acraws(s, k, out, func, strong, seed, 0L, 1L);
}

char* crypt(char* s, const char* k, char* out, hashfc func, int on, int weak, int noseed) {
    // get read, get set
    srand(time(0));
    char* key = (char*)malloc(strlen(k));
    strcpy(key, k);
    key = toBase64s((char*)key);
    int pos = strlen(key);
    while (*(key + pos - 1) == '=') {
        *(key + pos - 1) = '\0';
        pos--;
    }
    int len = (weak ? 3599 : 8965109) * strlen(k);
    // dump empty stuff
    if (strlen(k) == 0 || strlen(s) == 0 || func == NULL) return s;

    long from; int step;

    if (on) {
        if (!noseed && !weak) {
            from = rand() % len;
            int j = rand() % (sizeof(p) / sizeof(p[0]));
            while (strlen(k) % p[j] == 0 || p[j] % strlen(k) == 0) {
                j = rand() % sizeof(sizeof(p) / sizeof(p[0]));
            }
            step = p[j];
        } else {
            from = 0;
            step = 1;
        }
        char d = (!noseed) ? c64[rand() % 64] : s[strlen(s)-1];
        out = acraws(s, key, out, func, !weak, l64[d], from, step);
        strcat(out, " ");
        *(out+strlen(out)-1) = d;
        if (!noseed && !weak) {
            char* n = (char*)malloc(10);
            strcat(out, "#");
            strcat(out, itoa(from, n, 10));
            strcat(out, "#");
            strcat(out, itoa(step, n, 10));
        }
    } else {
        if (!weak) {
            int j = strlen(s)-1;
            char* d = strchr(s, '#') + 1;
            char* e = strrchr(s, '#') + 1;
            long from = 0;
            long step = 1;
            if (d != NULL) {
                char* from_str = (char*)malloc(e-d);
                strcpy(from_str, d);
                char* step_str = (char*)malloc(s + strlen(s) - e);
                strcpy(step_str, e);
                from = atoi(from_str);
                step = atoi(step_str);
            }
            *(d-1) = '\0';
            out = acraws(s, key, out, func, !weak, l64[s[d-s-2]], from, step);
            *(out+strlen(out)-1) = '\0';
        }
    }
    return out;
}

char* bcrypt(char* s, long size, const char* k, char* out, hashfc func, int on, int weak, int noseed) {
    // get read, get set
    srand(time(0));
    char* key = (char*)malloc(strlen(k));
    strcpy(key, k);
    key = toBase64s((char*)key);
    int pos = strlen(key);
    while (*(key + pos - 1) == '=') {
        *(key + pos - 1) = '\0';
        pos--;
    }
    int len = (weak ? 3599 : 8965109) * strlen(k);
    // dump empty stuff
    if (strlen(k) == 0 || size == 0L || sizeof(s) / sizeof(s[0]) == 0 || func == NULL) return s;

    unsigned int from, step;

    if (on) {
        if (!noseed && !weak) {
            from = rand() % len;
            int j = rand() % (sizeof(p) / sizeof(p[0]));
            while (strlen(k) % p[j] == 0 || p[j] % strlen(k) == 0) {
                j = rand() % sizeof(sizeof(p) / sizeof(p[0]));
            }
            step = p[j];
        } else {
            from = 0;
            step = 1;
        }
        char d = !noseed ? rand() % 256 : 0;
        out = acraw(s, key, out, func, !weak, d, from, step, 1);
        memcpy(out + size, &d, sizeof(char));
        if (!noseed && !weak) {
            memcpy(out + size + 1, &from, sizeof(int));
            memcpy(out + size + 5, &step, sizeof(int));
        }
    } else {
        if (!weak) {
            from = (unsigned int) *((int*)(s + size - sizeof(int) * 2));
            step = (unsigned int) *((int*)(s + size - sizeof(int)));
        } else {
            from = 0;
            step = 1;
        }
        char d = *(s + size - sizeof(int) * 2 - 1);
        *(s + size - sizeof(int) * 2 - 1) = '\0';
        out = acraw(s, key, out, func, !weak, d, from, step, 1);
    }
    return out;
}

void bcrypt_file(char* inputfile, char* outputfile, const char* k, hashfc func, int on, int weak, int noseed) {
    FILE *in = fopen(inputfile, "rb");
	if (!in) {
		printf("Input file \"%s\" not found.\n", inputfile);
        exit(1);
    }
    FILE *out = fopen(outputfile, "wb");

    long size;

    fseek(in, 0, SEEK_END);
    size = ftell(in);
    fseek(in, 0, SEEK_SET);

    // get read, get set
    srand(time(0));
    char* key = (char*)malloc(strlen(k));
    strcpy(key, k);
    key = toBase64s((char*)key);
    int pos = strlen(key);
    while (*(key + pos - 1) == '=') {
        *(key + pos - 1) = '\0';
        pos--;
    }
    int len = (weak ? 3599 : 8965109) * strlen(k);
    // dump empty stuff
    if (strlen(k) == 0 || size == 0L || func == NULL) return;

    unsigned int from, step;

    if (on) {
        if (!noseed && !weak) {
            from = rand() % len;
            int j = rand() % (sizeof(p) / sizeof(p[0]));
            while (strlen(k) % p[j] == 0 || p[j] % strlen(k) == 0) {
                j = rand() % sizeof(sizeof(p) / sizeof(p[0]));
            }
            step = p[j];
        } else {
            from = 0;
            step = 1;
        }
        unsigned char d = !noseed ? rand() % 256 : 0;
        acrawf(in, size, key, out, func, !weak, d, from, step);
        if (!noseed) fwrite(&d, sizeof(char), 1, out);
        if (!noseed && !weak) {
            fwrite(&from, sizeof(int), 1, out);
            fwrite(&step, sizeof(int), 1, out);
        }
    } else {
        unsigned char d = 0;
        if (!weak) {
            fseek(in, size - sizeof(int) * 2 - 1, SEEK_SET);
            fread(&d, sizeof(char), 1, in);
            fread(&from, sizeof(int), 1, in);
            fread(&step, sizeof(int), 1, in);
        } else {
            if (!noseed) {
                fseek(in, size - 1, SEEK_SET);
                fread(&d, sizeof(char), 1, in);
            }
            from = 0;
            step = 1;
        }
        fseek(in, 0, SEEK_SET);
        if (!noseed) size--;
        if (!weak) size -= sizeof(int) * 2;
        acrawf(in, size, key, out, func, !weak, d, from, step);
    }
    fclose(in);
    fclose(out);
}
