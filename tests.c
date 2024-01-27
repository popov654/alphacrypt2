#include <assert.h>
#include "md5.h"
#include "acp.h"

void testMD5() {
    const char* input = "Hello";
    char* hash = md5StringHash(input);
    printf("MD5('Hello') = \"%s\"\n", hash);
    assert(strcmp("8b1a9953c4611296a827abf8c47804d7", hash) == 0);
}

void testBase64() {
    char* str = toBase64s("Hello");
    printf("Base64('Hello') = \"%s\"\n", str);
    assert(strcmp("SGVsbG8=", str) == 0);
}

void testSimpleEncryption() {
    acp_init();
    printf("%s\n", acraws_basic("Hello", "d3bkvc5z", NULL, md5StringHash, 0, l64['C']));
    printf("%s\n", acraws_basic("FSnMk", "d3bkvc5z", NULL, md5StringHash, 0, l64['C']));
}

void testStrongEncryption() {
    acp_init();
    char* encrypted = acp_crypt("Hello", "ifofooL3", NULL, md5StringHash, 1, 0, 0);
    printf("%s\n", encrypted);
    char* decrypted = acp_crypt(encrypted, "ifofooL3", NULL, md5StringHash, 0, 0, 0);
    printf("%s\n", decrypted);
}

void testStrongEncryptionBinary() {
    acp_init();
    char* data = (char*)malloc(5);
    data[0] = 'H';
    data[1] = 'e';
    data[2] = 'l';
    data[3] = 'l';
    data[4] = 'o';
    char* result;
    char* encrypted = acp_bcrypt(data, 5, "d3bkvc5z", NULL, md5StringHash, 1, 0, 0);
    for (int i = 0; i < 6; i++) {
        printf("%u ", (unsigned char) encrypted[i]);
    }
    printf("Offset: %u ", *((unsigned int*) (encrypted + 6)));
    printf("Step: %u ", *((unsigned int*) (encrypted + 6 + sizeof(int))));
    printf("\n");
    char* decrypted = acp_bcrypt(encrypted, 5 + 1 + sizeof(int) * 2, "d3bkvc5z", NULL, md5StringHash, 0, 0, 0);
    printf("%s\n", decrypted);
}

int filesAreEqual(char* _file1, char* _file2, int probs) {
    FILE *file1 = fopen(_file1, "rb");
	FILE *file2 = fopen(_file2, "rb");
	if (!file1) {
		printf("Input file \"%s\" not found.\n", file1);
        exit(1);
    }
    if (!file2) {
		printf("Input file \"%s\" not found.\n", file2);
        exit(1);
    }

    long size;
    unsigned char segment = 100;

    fseek(file1, 0, SEEK_END);
    size = ftell(file1);
    fseek(file1, 0, SEEK_SET);

    fseek(file2, 0, SEEK_END);
    if (ftell(file2) != size) {
        return 0;
    };
    fseek(file2, 0, SEEK_SET);

    long pos = 0, skip = probs > 1 ? (size - segment) / (probs - 1) - segment : (size > segment ? size - segment : 1);
    while (pos < size) {
        for (int i = 0; i < segment; i++) {
            unsigned char c1, c2;
            int res1 = fread(&c1, 1, 1, file1);
            int res2 = fread(&c2, 1, 1, file2);
            if (c1 != c2 || c1 == EOF) {
                fclose(file1);
                fclose(file2);
                return c1 == EOF;
            }
        }
        fseek(file1, skip, SEEK_CUR);
        fseek(file2, skip, SEEK_CUR);
        pos += segment + skip;
    }

    fclose(file1);
    fclose(file2);

    return 1;
}

void testBinaryFileEncryption() {
    acp_bcrypt_file("pic.png", "pic.png.acp", "d3bkvc5z", md5StringHash, 1, 0, 0);
    acp_bcrypt_file("pic.png.acp", "pic2.png", "d3bkvc5z", md5StringHash, 0, 0, 0);
    assert(filesAreEqual("pic.png", "pic2.png", 3) == 1);
}

 
int main(int argc, char* argv[])
{
    testBase64();
    testMD5();
    testSimpleEncryption();
    testStrongEncryption();
    testStrongEncryptionBinary();
    testBinaryFileEncryption();
    return 0;
}
