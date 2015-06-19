#include <stdio.h>

FILE* getFP();

int main(int argc, char** argv) {
  if (getFP() == NULL) {
    return 1;
  }
  return 0;
}

