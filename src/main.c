#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <locale.h>
#include <fcntl.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include "mapping.h"

size_t getFilesize(const char* filename) {
    struct stat st;
    stat(filename, &st);
    return st.st_size;
}

void fillToLowerMap(uint8_t* map) {
    uint8_t c = 'A';
    while(1) {
        map[c] = c+32;
        if(c++ == 'Z') break;
    }
    c = 'a';
    while(1) {
        map[c] = c;
        if(c++ == 'z') break;
    }
    map[0xE9] = 0xE9;
    map[0xFE] = 0xFE;
    map[0xF0] = 0xF0;
    map[0xE4] = 0xE4;
    map[0xF5] = 0xF5;
    map[0xF6] = 0xF6;
    map[0xFC] = 0xFC;
    map[0x21] = 0x21;
    map[0x27] = 0x27;
}

int fixInput(const uint8_t* input, int l, uint8_t* output, uint8_t* toLower) {
    uint8_t replaced = 0;

    //printf("len=%d\n", l);

    for(uint8_t i = 0; i < l; i++) {
        uint16_t wchar = input[i] << 8 | input[i+1];
        uint8_t res = encmap[wchar];

        if(res != 0) {
            //printf("putting %02x to %d\n", res, i-replaced);
            output[i-replaced] = toLower[res];
            replaced++;
            i++;
        } else {
            //printf("putting %02x to %d\n", input[i], i-replaced);
            output[i-replaced] = toLower[input[i]];
        }
    }

    return l - replaced;
}

void fillCharMap(const uint8_t* input, int len, uint8_t* charMap) {
    for(uint8_t i = 0; i < len; i++) {
        charMap[input[i]] += 1;
    }
}

uint8_t xor(const uint8_t* input, int len) {
    uint8_t xor = 0;
    for(uint8_t i = 0; i < len; i++) {
        xor ^= input[i];
    }
    return xor;
}

int main(int argc, char** argv) {
    struct timespec startT, endT;
    clock_gettime(CLOCK_MONOTONIC, &startT);

    char* dictionaryFileName = argv[1];
    char* searchWord = argv[2];
    size_t inputLength = strlen(searchWord);

    size_t filesize = getFilesize(dictionaryFileName);
    uint8_t* lex = mmap(NULL, filesize, PROT_READ, MAP_PRIVATE | MAP_POPULATE, open(dictionaryFileName, O_RDONLY, 0), 0);

    uint8_t fixed[inputLength];
    uint8_t inputCharMap[0xFF];
    memset(inputCharMap, 0, 255);

    uint8_t toLower[0xFF];
    fillToLowerMap(toLower);

    uint8_t currentXor = 0;
    int currentLength = 0;

    inputLength = fixInput(searchWord, inputLength, fixed, toLower);
    uint8_t inputXor = xor(fixed, inputLength);

    fillCharMap(fixed, inputLength, inputCharMap);

    char resultBuf[1000];
    int resp = 0;
    uint8_t ch = 0;

    uint8_t currentCharMap[0xFF];
    memset(currentCharMap, 0, 255);

    for(int i = 0; i < filesize; i++) {
        ch = *(lex)++;

        if(ch == '\r') {
            if(currentXor == inputXor && currentLength == inputLength) {
                //Might be the guy
                int count = 0;
                for(int j = 0; j < inputLength; j++) {
                    uint8_t testChar = toLower[*(lex-inputLength-1+j)];

                    if(inputCharMap[testChar] == currentCharMap[testChar]) count++;
                }

                if(count == inputLength) {
                    for(int j = 0; j < inputLength; j++) {
                        resultBuf[resp++] = *(lex-inputLength-1+j);
                    }
                    resultBuf[resp++] = ',';
                }
            }

            for(int j = 0; j < currentLength; j++) {
                uint8_t testChar = toLower[*(lex-currentLength-1+j)];
                currentCharMap[testChar] = 0;
            }

            currentXor = 0;
            currentLength = 0;
            i++;
            lex++;
            continue;

        } else {
            uint8_t lch = toLower[ch];
            currentCharMap[lch] += 1;
            currentXor ^= lch;
            currentLength++;
        }

    }

    if(resp != 0) {
        resultBuf[resp-1] = '\0';
    } else {
        resultBuf[0] = '\0';
    }


    clock_gettime(CLOCK_MONOTONIC, &endT);

    long nanos_used = (endT.tv_sec - startT.tv_sec) * 1000000000 + (endT.tv_nsec - startT.tv_nsec);
    printf("%.1f,%s\n", (float) nanos_used/1000, resultBuf);
}
