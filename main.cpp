#include <iostream>
#include <stdio.h>
#include <zlib.h>
#include <cstring>
#include "ZlibApi.h"

int main(int argc, char *argv[]) {
    int ret;
    if (argc == 2 && strcmp(argv[1], "-i") == 0) {
        ret = doInflate(stdin, stdout);
        if (ret != Z_OK)
            checkZlibError(ret);
        return ret;
    }
    else if (argc == 2 && strcmp(argv[1], "-d") == 0) {
        ret = doDeflate(stdin, stdout, Z_DEFAULT_COMPRESSION);
        if (ret != Z_OK)
            checkZlibError(ret);
        return ret;
    }
    else {
        std::cerr << "Archiver usage: archiver [-d] | [-i] < file\n";
        return 1;
    }
}