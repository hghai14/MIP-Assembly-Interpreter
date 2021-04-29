#ifndef A5
#define A5

#include <iostream>
#include <cstring>
#include <map>
#include <vector>
#include <algorithm>
#include <set>
#include <cmath>
#include <fstream>

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

std::string decToHex(unsigned int n);

void printRegisterFile(unsigned int register_file[], unsigned int core_num);

void throwRunTimeError(const std::string &message, unsigned int current, unsigned int core_num);

void throwRunTimeError(const std::string &message, unsigned int core_num);

void throwError(const std::string &message, int &lineNum, int &column, unsigned int core_num);

void throwError(const std::string &message, int &lineNum, unsigned int core_num);

void printOverFlowMessage(std::string &s, unsigned int &current, unsigned int core_num);

void detectOFadd(int a, int b, unsigned int current, unsigned int core_num);

void detectOFmul(int a, int b, unsigned int current, unsigned int core_num);

void detectOFsub(int a, int b, unsigned int current, unsigned int core_num);

class Core
{

private:
    std::ifstream instream;
    std::string message;
   
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

    static std::string instruction_type_string[10];
    static std::set<std::string> reserved_words;
    static std::map<InstructionType, int> op_codes;
    static std::map<std::string, int> register_map;
    static std::map<int, std::string> num_to_reg;

public:

    unsigned int core_num;

    unsigned int base_address;

    unsigned int register_file[32];
    unsigned int instruction_memory[1024];
    unsigned int curParsePointer;
    unsigned int endCommand; // Last Instruciton address
    unsigned int current; // PC
    std::map<std::string, unsigned int> branches;
    std::map<unsigned int, std::string> labels;
    int label_ptr;

    Core(std::string path, unsigned int core_num);

    void compile();
    bool execute();
    void setup();

    void executeCommand(InstructionType i_type, std::vector<unsigned int> &params);

    void executeJ(std::vector<unsigned int> &params);
    void executeAddi(std::vector<unsigned int> &params);
    void executeAdd(std::vector<unsigned int> &params);
    void executeSub(std::vector<unsigned int> &params);
    void executeMul(std::vector<unsigned int> &params);
    void executeSlt(std::vector<unsigned int> &params);
    void executeBeq(std::vector<unsigned int> &params);
    void executeBne(std::vector<unsigned int> &params);
    void executeLw(std::vector<unsigned int> &params);
    void executeSw(std::vector<unsigned int> &params);

    void parse(std::string s, int &lineNum);
    void parseInstruction(InstructionType i_type, std::vector<int> params, int &lineNum);
    void parse3Arg(std::vector<std::string> &tokens, int &lineNum, InstructionType i_type);

    void parseJump(std::vector<std::string> &tokens, int &lineNum);
    void parseAdd(std::vector<std::string> &tokens, int &lineNum);
    void parseSub(std::vector<std::string> &tokens, int &lineNum);
    void parseMul(std::vector<std::string> &tokens, int &lineNum);
    void parseBeq(std::vector<std::string> &tokens, int &lineNum);
    void parseBne(std::vector<std::string> &tokens, int &lineNum);
    void parseSlt(std::vector<std::string> &tokens, int &lineNum);
    void parseLw(std::vector<std::string> &tokens, int &lineNum);
    void parseSw(std::vector<std::string> &tokens, int &lineNum);
    void parseAddi(std::vector<std::string> &tokens, int &lineNum);

    void printRegisterFile(unsigned int register_file[], unsigned int core_num);
    InstructionType getInstructionType(unsigned int opcode, unsigned int &current, unsigned int core_num);

};

class DRAM
{
public:
    static int ROW_ACCESS_DELAY;
    static int COL_ACCESS_DELAY;
};


#endif