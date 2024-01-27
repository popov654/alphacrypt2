#include "md5.h"
#include "acp.h"
#include "wchar.h"
#include "locale.h"

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

int file_exists(const char *filename)
{
    FILE *file;
    file = fopen(filename, "r");
    if (file) {
        fclose(file);
        return 1;
    }
    return 0;
}

int ask_for_overwrite(const char* output) {
    if (file_exists(output)) {
        printf("The file with such name already exists. Do you want to overwrite it? (y/N) ");
        char c;
        scanf("%c", &c);
        if (c != 'y' && c != 'Y') {
            return 0;
        }
    }
    return 1;
}
 
int main(int argc, char* argv[])
{
    char *src = NULL, *input = NULL, *key = NULL, *keyfilename = NULL, *output = NULL;
    int on = 1;

    setlocale(LC_ALL, "en_US.utf8");

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
            if (!file_exists(input)) {
                src = input;
            } else {
                if (on) {
                    output = (char*) malloc(strlen(input) + 5);
                    strcpy(output, input);
                    strcat(output, ".acp");
                } else {
                    output = (char*) malloc(strlen(input) - 3);
                    strncpy(output, input, strlen(input) - 4);
                    output[strlen(input) - 4] = '\0';
                }
                if (ask_for_overwrite(output)) {
                    acp_bcrypt_file(input, output, key, md5StringHash, on, 0, 0);
                }
                return 0;
            }
        }
        char* encrypted = acp_crypt(src, key, NULL, md5StringHash, on, 0, 0);
        
        wchar_t* output_string;
        int len = mbstowcs(NULL, encrypted, 0);
        output_string = (wchar_t*) malloc(len * 4);
        mbstowcs(output_string, encrypted, len);
        output_string[len] = L'\0';
        printf_s("%ls", output_string);

        return 0;
    }

    if (output && input) {
        if (ask_for_overwrite(output)) {
            acp_bcrypt_file(input, output, key, md5StringHash, on, 0, 0);
        }
        return 0;
    }

    printf("Usage: acp.exe -i [input | inputfile] -k [key | keyfile] -o [output | outputfile]");

    return 0;
}
