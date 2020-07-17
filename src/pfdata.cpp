#include "parflow/pfdata.hpp"

#include <stdio.h>

PFData::PFData() {

}
PFData::PFData(const char * filename) {
    _filename = filename;
}
int PFData::loadFile() {
    return 1;
}

void hello(void) {
    printf("Hello, World!\n");
}
