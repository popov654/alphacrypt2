#include "md5.h"
#include "acp.h"

char* readFileContent(char* filename) {
    FILE *in = fopen(filename, "rb");
    if (!in) {
        //printf("Input file \"%s\" not found.\n", filename);
        //exit(1);
        return NULL;
    }

    long size, pos;

    fseek(in, 0, SEEK_END);
    size = ftell(in);
    fseek(in, 0, SEEK_SET);

    if (size >= 1e8 * 5) {
        printf("File \"%s\" is too large", filename);
        exit(1);
    }

    char* content = (char*) malloc(size);
    fread(content, 1, size, in);
    fclose(in);

    return content;
}
 
int main(int argc, char* argv[])
{
    char *src = NULL, *input = NULL,
         *output = NULL, *key = NULL, *keyfilename = NULL;
    int on = 1;
    for (int i = 1; i < argc; i++) {
        if (i < argc-1 && (strcmp("-i", argv[i]) == 0 ||
                           strcmp("-in", argv[i]) == 0 ||
                           strcmp("--input", argv[i]) == 0)) {
            input = argv[++i];
        }
        else if (i < argc-1 && (strcmp("-o", argv[i]) == 0 ||
                           strcmp("-out", argv[i]) == 0 ||
                           strcmp("--output", argv[i]) == 0)) {
            output = argv[++i];
        }
        else if (i < argc-1 && (strcmp("-k", argv[i]) == 0 ||
                           strcmp("--key", argv[i]) == 0)) {
            key = argv[++i];
        }
        else if (i < argc-1 && (strcmp("-kf", argv[i]) == 0 ||
                           strcmp("--keyfile", argv[i]) == 0)) {
            keyfilename = argv[++i];
        }
        else if (strcmp("-d", argv[i]) == 0 || strcmp("--decode", argv[i]) == 0) {
            on = 0;
        }
    }

    if (!input && argc > 1 && argv[1][0] != '-') {
        input = argv[1];
    }

    if (!key && keyfilename) {
        key = readFileContent(keyfilename);
    }

    if (input && !key) {
        printf("Key parse error");
        return 1;
    }

    acp_init();

    if (!output && input) {
        if ((*(input) == '"' || *(input) == '\'') && *(input + strlen(input) - 1) == *(input)) {
            src = input+1;
            *(src + strlen(input)-2) = '\0';
        } else {
            src = readFileContent(input);
            if (src == NULL) {
                src = input;
            } else {
                output = (char*) malloc(strlen(input) + 5);
                strcpy(output, input);
                strcat(output, ".acp");
                acp_bcrypt_file(input, output, key, md5StringHash, on, 0, 0);
                return 0;
            }
        }
        char* encrypted = acp_crypt(src, key, NULL, md5StringHash, on, 0, 0);
        printf("%s", encrypted);
        return 0;
    }

    if (output && input) {
        acp_bcrypt_file(input, output, key, md5StringHash, on, 0, 0);
        return 0;
    }

    printf("Usage: acp.exe -i [input | inputfile] -k [key | keyfile] -o [output | outputfile]");

    return 0;
}
