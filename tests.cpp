//
// Created by Georg on 04.09.2023.
//
#include <gtest/gtest.h>
#include "cpu.h"

class Test1 : public testing::Test {
public:

    Memory mem;
    CPU cpu {mem, 9};

    void SetUp() override {
        cpu.reset();

    }
};

TEST_F(Test1, RunInlineProgram) {

    cpu.reset();

    mem[0xFFFC] = CPU::JSR;
    mem[0xFFFD] = 0x42;
    mem[0xFFFE] = 0x42;
    mem[0x4242] = CPU::LDA_IM;
    mem[0x4243] = 0x84;
    cpu.execute();
}

