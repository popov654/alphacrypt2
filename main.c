#include "md5.h"
#include "acp.h"
#include "wchar.h"
#include "locale.h"

#ifdef _WIN32
#include "processenv.h"
#include "windows.h"
#include "shellapi.h"
#endif

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
 
int main(int argc, wchar_t* argv[])
{
    char *src = NULL, *input = NULL, *key = NULL, *keyfilename = NULL, *output = NULL;
    int on = 1;

    setlocale(LC_ALL, "en_US.utf8");
    
    #ifdef _WIN32
    LPWSTR cl = GetCommandLineW();
    int num_args;
    argv = CommandLineToArgvW(cl, &argc);
    #endif

    for (int i = 1; i < argc; i++) {
        if (i < argc-1 && (wcscmp(L"-i", argv[i]) == 0 ||
                           wcscmp(L"-in", argv[i]) == 0 ||
                           wcscmp(L"--input", argv[i]) == 0)) {
            int strlen = wcstombs(input, argv[i+1], wcslen(argv[i+1]));
            input = (char*) malloc(strlen + 1);
            wcstombs(input, argv[++i], strlen);
            input[strlen] = '\0';
        }
        else if (i < argc-1 && (wcscmp(L"-o", argv[i]) == 0 ||
                           wcscmp(L"-out", argv[i]) == 0 ||
                           wcscmp(L"--output", argv[i]) == 0)) {
            int strlen = wcstombs(output, argv[i+1], wcslen(argv[i+1]));
            output = (char*) malloc(strlen + 1);
            wcstombs(output, argv[++i], strlen);
        }
        else if (i < argc-1 && (wcscmp(L"-k", argv[i]) == 0 ||
                           wcscmp(L"--key", argv[i]) == 0)) {
            int strlen = wcstombs(key, argv[i+1], wcslen(argv[i+1]));        
            key = (char*) malloc(strlen + 1);
            wcstombs(key, argv[++i], strlen);
            key[strlen] = '\0';
        }
        else if (i < argc-1 && (wcscmp(L"-kf", argv[i]) == 0 ||
                           wcscmp(L"--keyfile", argv[i]) == 0)) {
            int strlen = wcstombs(keyfilename, argv[i+1], wcslen(argv[i+1]));
            keyfilename = (char*) malloc(strlen + 1);
            wcstombs(keyfilename, argv[++i], strlen);
        }
        else if (wcscmp(L"-d", argv[i]) == 0 || wcscmp(L"--decode", argv[i]) == 0) {
            on = 0;
        }
    }

    if (!input && argc > 1 && argv[1][0] != L'-') {
        int strlen = wcstombs(input, argv[1], wcslen(argv[1]));
        input = (char*) malloc(strlen + 1);
        wcstombs(input, argv[1], strlen);
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
        
        wchar_t* output_string;
        int len = mbstowcs(NULL, encrypted, 0);
        output_string = (wchar_t*) malloc(len * 4);
        mbstowcs(output_string, encrypted, len);
        output_string[len] = L'\0';
        printf_s("%ls", output_string);

        return 0;
    }

    if (output && input) {
        acp_bcrypt_file(input, output, key, md5StringHash, on, 0, 0);
        return 0;
    }

    printf("Usage: acp.exe -i [input | inputfile] -k [key | keyfile] -o [output | outputfile]");

    return 0;
}
