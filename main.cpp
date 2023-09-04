#include <iostream>
#include "cpu.h"

int main() {
    Memory mem;
    CPU cpu(mem, 9);
    cpu.reset();

    mem[0xFFFC] = CPU::JSR;
    mem[0xFFFD] = 0x42;
    mem[0xFFFE] = 0x42;
    mem[0x4242] = CPU::LDA_IM;
    mem[0x4243] = 0x84;
    cpu.execute();
    return 0;
}
