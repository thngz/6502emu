#include <iostream>

// https://www.obelisk.me.uk/6502/
// https://www.youtube.com/watch?v=qJgsuQoy9bc

using Byte = unsigned char;
using Word = unsigned short;
using u32 = unsigned int;

struct Memory {
    static constexpr u32 MAX_MEM = 1024 * 64;
    Byte Data[MAX_MEM];

    void Init() {
        for (unsigned char &i: Data) {
            i = 0;
        }
    }

    // read byte from memory
    Byte operator[](u32 Address) const {
        if (Address > MAX_MEM) {
            throw std::invalid_argument("Address cant be larger than maximum memory");
        }
        return Data[Address];
    }

    // write byte to memory
    Byte &operator[](u32 Address) {
        if (Address > MAX_MEM) {
            throw std::invalid_argument("Address cant be larger than maximum memory");
        }
        return Data[Address];
    }

    // write 2 bytes
    void WriteWord(u32 &cycles, Word w, u32 Address) {
        Data[Address] = w & 0xFF;
        Data[Address + 1] = (w >> 8);
        cycles -= 2;
    }
};

class CPU {
public:
    Memory& memory;
    u32 cycles;

    CPU(Memory &memory, u32 cycles) : memory(memory), cycles(cycles) {}

    Word pc; // program counter
    Byte sp; // stack pointer

    // Opcodes
    enum Instructions {
        // LDA
        INS_LDA_IM = 0xA9,
        INS_LDA_ZP = 0xA5,
        INS_LDA_ZPX = 0xB5,
        //JSR
        INS_JSR = 0x20
    };

    void Execute() {
        while (cycles > 0) {
            Byte instruction = FetchByte();

            switch (instruction) {
                case INS_LDA_IM: {
                    Byte value = FetchByte();
                    ACC = value;
                    Z = (ACC == 0);

                    N = (ACC & 0b10000000) > 0; // if bit 7 of accumulator is set, raise negative flag
                    break;
                }
                case INS_LDA_ZP: {

                    Byte zero_page_address = FetchByte();

                    ACC = ReadByte(zero_page_address);
                    Z = (ACC == 0);

                    N = (ACC & 0b10000000) > 0; // if bit 7 of accumulator is set, raise negative flag
                    break;
                }
                case INS_LDA_ZPX: {

                    Byte zero_page_address = FetchByte();
                    zero_page_address += X;
                    cycles--;
                    ACC = ReadByte(zero_page_address);

                    Z = (ACC == 0);
                    N = (ACC & 0b10000000) > 0; // if bit 7 of accumulator is set, raise negative flag
                    break;
                }
                case INS_JSR: {
                    Word sub_addr = FetchWord();
                    memory[sp] = pc - 1;
                    memory.WriteWord(cycles, pc - 1, sp);
                    sp++;
                    pc = sub_addr;

                    cycles--;
                }
                default: {
                    std::cout << "Instruction not handled\n";
                    break;
                }
            }
        }
    }
    void Reset() {
        pc = 0xFFFC;
        sp = 0x0100;
        D = 0;
        ACC = X = Y = 0;
        C = C = I = D = B = O = N = 0;
        memory.Init();
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
    Byte FetchByte() {
        Byte data = memory[pc];
        pc++;
        cycles--;
        return data;
    }

    Byte ReadByte(Byte address) {
        Byte data = memory[address];
        cycles--;
        return data;
    }

    Word FetchWord() {
        // 6502 is little endian
        Word data = memory[pc];
        pc++;

        data |= (memory[pc] << 8);
        pc++;
        cycles -= 2;
        return data;
    }
};

int main() {
    Memory mem;
    CPU cpu(mem, 9);
    cpu.Reset();
    mem[0xFFFC] = CPU::INS_JSR;
    mem[0xFFFD] = 0x42;
    mem[0xFFFE] = 0x42;
    mem[0x4242] = CPU::INS_LDA_IM;
    mem[0x4243] = 0x84;
    cpu.Execute();
    return 0;
}
