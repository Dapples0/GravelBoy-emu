#include <iostream>

#include "src/gb.h"


int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: ./GravelBoy path/to/rom\n";
        exit(1);
    }
    gb gb;

    gb.run((const char*)argv[1]);





    return 0;
}
