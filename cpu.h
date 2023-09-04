//
// Created by Georg on 04.09.2023.
//

// https://www.obelisk.me.uk/6502/
// https://www.youtube.com/watch?v=qJgsuQoy9bc

#ifndef INC_6502EMU_CPU_H
#define INC_6502EMU_CPU_H

using Byte = unsigned char;
using Word = unsigned short;
using u32 = unsigned int;

struct Memory {
    static constexpr u32 MAX_MEM = 1024 * 64;
    Byte Data[MAX_MEM];

    void init() {
        for (unsigned char &i: Data) {
            i = 0;
        }
    }

    // read byte from memory
    Byte operator[](u32 address) const {
        if (address > MAX_MEM) {
            throw std::invalid_argument("address cant be larger than maximum memory");
        }
        return Data[address];
    }

    // write byte to memory
    Byte &operator[](u32 address) {
        if (address > MAX_MEM) {
            throw std::invalid_argument("address cant be larger than maximum memory");
        }
        return Data[address];
    }

    // write 2 bytes
    void write_word(u32 &cycles, Word w, u32 address) {
        Data[address] = w & 0xFF;
        Data[address + 1] = (w >> 8);
        cycles -= 2;
    }

};

class CPU {
public:
    Memory &memory;
    u32 cycles;

    Word pc; // program counter
    Byte sp; // stack pointer

    enum Opcodes {
        // LDA
        LDA_IM = 0xA9,
        LDA_ZP = 0xA5,
        LDA_ZPX = 0xB5,
        //JSR
        JSR = 0x20
    };

    CPU(Memory &memory, u32 cycles) : memory(memory), cycles(cycles) {}

    void execute() {
        while (cycles > 0) {
            Byte instruction = fetch_byte();

            switch (instruction) {
                case LDA_IM: {
                    Byte value = fetch_byte();
                    ACC = value;
                    Z = (ACC == 0);

                    N = (ACC & 0b10000000) > 0; // if bit 7 of accumulator is set, raise negative flag
                    break;
                }
                case LDA_ZP: {

                    Byte zero_page_address = fetch_byte();

                    ACC = read_byte(zero_page_address);
                    Z = (ACC == 0);

                    N = (ACC & 0b10000000) > 0; // if bit 7 of accumulator is set, raise negative flag
                    break;
                }
                case LDA_ZPX: {

                    Byte zero_page_address = fetch_byte();
                    zero_page_address += X;
                    cycles--;
                    ACC = read_byte(zero_page_address);

                    Z = (ACC == 0);
                    N = (ACC & 0b10000000) > 0; // if bit 7 of accumulator is set, raise negative flag
                    break;
                }
                case JSR: {
                    Word sub_addr = fetch_word();
                    memory[sp] = pc - 1;
                    memory.write_word(cycles, pc - 1, sp);
                    sp++;
                    pc = sub_addr;

                    cycles--;
                    break;
                }
                default: {
                    std::cout << "Instruction not handled\n";
                    break;
                }
            }
        }
    }

    void reset() {
        // Sets program counter and stack pointers to their default positions
        // Sets all flags and registers to 0
        // Initializes memory
        pc = 0xFFFC;
        sp = 0x0100;
        D = 0;
        ACC = X = Y = 0;
        C = C = I = D = B = O = N = 0;
        memory.init();
    }

private:
    // registers
    Byte ACC, X, Y;

    // flags (c++ bit fields)
    Byte C: 1; // carry flag - set if last operation caused an overflow from bit 7 of the result or an underflow from bit 0.
    Byte Z: 1; // zero flag - set if result of the last operation was zero
    Byte I: 1; // interrupt disable flag - set if the program has executed a 'Set Interrupt Disable' instruction
    Byte D: 1; // decimal mode flag - if its set the processor will do BCD (binary coded decimal) arithmetic
    Byte B: 1; // break command flag - set when BRK instruction has been executed
    Byte O: 1; // overflow flag - set when the arithmetic result has yielded an invalid 2's complement result
    Byte N: 1; // negative flag - set if the result of the last operation had bit 7 set to a one

    Byte fetch_byte() {
        // Get a byte from memory, increments program counter
        Byte data = memory[pc];
        pc++;
        cycles--;
        return data;
    }

    Byte read_byte(Byte address) {
        // Read a byte from memory (does not update program counter)
        Byte data = memory[address];
        cycles--;
        return data;
    }

    Word fetch_word() {
        // 6502 is little endian
        Word data = memory[pc];
        pc++;
        data |= (memory[pc] << 8);
        pc++;
        cycles -= 2;
        return data;
    }
};

#endif //INC_6502EMU_CPU_H
