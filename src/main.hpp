#ifndef MAIN
#define MAIN

#include <iostream>
#include <cstring>
#include <map>
#include <vector>
#include <algorithm>
#include <set>
#include <cmath>
#include <fstream>

std::string message;

// Memory array, 4 bytes at a time
unsigned int memory[262144];

long long int totalCycles = 0;

int n;

extern bool optimize;

// Memory array, 4 bytes at a time
extern unsigned int memory[262144];

// Types of instructions
enum InstructionType
{
    jump,
    add,
    sub,
    mul,
    beq,
    bne,
    slt,
    lw,
    sw,
    addi
};

std::string instruction_type_string[] = {"j", "add", "sub", "mul", "beq", "bne", "slt", "lw", "sw", "addi"};

// Set to check a reserved word
std::set<std::string> reserved_words{"add", "j", "sub", "mul", "beq", "bne", "slt", "lw", "sw", "addi"};

std::map<InstructionType, int> op_codes({std::make_pair(add, 32),
                                         std::make_pair(slt, 42),
                                         std::make_pair(sub, 34),
                                         std::make_pair(jump, 2),
                                         std::make_pair(mul, 24),
                                         std::make_pair(beq, 4),
                                         std::make_pair(bne, 5),
                                         std::make_pair(lw, 35),
                                         std::make_pair(sw, 43),
                                         std::make_pair(addi, 8)});

std::map<std::string, int> register_map = {
    std::make_pair("zero", 0),
    std::make_pair("at", 1),
    std::make_pair("v0", 2),
    std::make_pair("v1", 3),
    std::make_pair("a0", 4),
    std::make_pair("a1", 5),
    std::make_pair("a2", 6),
    std::make_pair("a3", 7),
    std::make_pair("t0", 8),
    std::make_pair("t1", 9),
    std::make_pair("t2", 10),
    std::make_pair("t3", 11),
    std::make_pair("t4", 12),
    std::make_pair("t5", 13),
    std::make_pair("t6", 14),
    std::make_pair("t7", 15),
    std::make_pair("s0", 16),
    std::make_pair("s1", 17),
    std::make_pair("s2", 18),
    std::make_pair("s3", 19),
    std::make_pair("s4", 20),
    std::make_pair("s5", 21),
    std::make_pair("s6", 22),
    std::make_pair("s7", 23),
    std::make_pair("t8", 24),
    std::make_pair("t9", 25),
    std::make_pair("k0", 26),
    std::make_pair("k1", 27),
    std::make_pair("gp", 28),
    std::make_pair("sp", 29),
    std::make_pair("fp", 30),
    std::make_pair("ra", 31)};

std::map<int, std::string> num_to_reg = {
    std::make_pair(4, "a0"),
    std::make_pair(5, "a1"),
    std::make_pair(6, "a2"),
    std::make_pair(7, "a3"),
    std::make_pair(1, "at"),
    std::make_pair(30, "fp"),
    std::make_pair(28, "gp"),
    std::make_pair(26, "k0"),
    std::make_pair(27, "k1"),
    std::make_pair(31, "ra"),
    std::make_pair(16, "s0"),
    std::make_pair(17, "s1"),
    std::make_pair(18, "s2"),
    std::make_pair(19, "s3"),
    std::make_pair(20, "s4"),
    std::make_pair(21, "s5"),
    std::make_pair(22, "s6"),
    std::make_pair(23, "s7"),
    std::make_pair(29, "sp"),
    std::make_pair(8, "t0"),
    std::make_pair(9, "t1"),
    std::make_pair(10, "t2"),
    std::make_pair(11, "t3"),
    std::make_pair(12, "t4"),
    std::make_pair(13, "t5"),
    std::make_pair(14, "t6"),
    std::make_pair(15, "t7"),
    std::make_pair(24, "t8"),
    std::make_pair(25, "t9"),
    std::make_pair(2, "v0"),
    std::make_pair(3, "v1"),
    std::make_pair(0, "zero"),
};

std::map<InstructionType, long long int> instruction_count = {
    std::make_pair(jump, 0),
    std::make_pair(add, 0),
    std::make_pair(sub, 0),
    std::make_pair(mul, 0),
    std::make_pair(slt, 0),
    std::make_pair(addi, 0),
    std::make_pair(bne, 0),
    std::make_pair(beq, 0),
    std::make_pair(lw, 0),
    std::make_pair(sw, 0),
};

#endif