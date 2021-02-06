#include<stdio.h>
#include<stdlib.h>
#include<string.h>

void RIVERTestOneInput(char *buffer) {
    LLVMFuzzerTestOneInput(buffer[0], buffer + 1);
}

int main(int argc, char* argv[]) {
    // Read binary input from stdin
    freopen(NULL, "rb", stdin);

    // Use a max input size of 4096 bytes
    const int MAX_INPUT_SIZE = 4096;

    char buffer[MAX_INPUT_SIZE];
    int len = 0;

    // While there is a new input
    while ((len = fread(buffer, sizeof(char), MAX_INPUT_SIZE, stdin)) != 0)
    {
        // Send the input to the buffer
        LLVMFuzzerTestOneInput(buffer, len);
        if (feof(stdin)) {
            break;
        }
    }
}
