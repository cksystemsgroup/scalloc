#include <stdio.h>

static FILE* fp = fopen("/home/mlippautz/.vimrc", "r");

FILE* getFP() { return fp; }

