#pragma once

#include <iostream>
#include <cassert>

#define CHUNK 16384

int doDeflate(FILE *source, FILE *dest, int level);
int doInflate(FILE* source, FILE *dest);
void checkZlibError(int ret);