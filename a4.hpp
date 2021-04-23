#ifndef A4
#define A4

#include <iostream>
#include <cstring>
#include <map>
#include <vector>
#include <algorithm>
#include <set>
#include <cmath>
#include <fstream>

extern int busy[32];
extern std::set<int> waiting;
extern std::set<unsigned int> busy_mem;

extern bool optimize;
extern long long row_updates_count;

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

class Core
{

private:
    std::ifstream instream;

public:

    unsigned int core_num;

    unsigned int base_address;

    unsigned int register_file[32];
    unsigned int curParsePointer;
    unsigned int endCommand; // Last Instruciton address
    unsigned int current; // PC
    std::map<std::string, unsigned int> branches;
    std::map<unsigned int, std::string> labels;
    int label_ptr;

    Core(std::string path, unsigned int core_num);

    void compile();
    bool execute();

    void executeCommand(InstructionType i_type, std::vector<unsigned int> &params);

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

};

class AccessType
{

public:
    std::string message;
    int param;
    int cycles;

    AccessType(std::string message, int param, int cycles);
    ~AccessType();
};

class DRAM
{

public:
    bool type;
    unsigned int param;
    unsigned int address;

    Core* core;

    std::set<int> depends;

    static int ROW_ACCESS_DELAY;
    static int COL_ACCESS_DELAY;

    static int active_row;
    static bool buffer_updated;

    unsigned int row_num;
    unsigned int col_num;

    std::vector<AccessType> qu;

    DRAM(bool type, unsigned int param, unsigned int address);
    ~DRAM();

    void next();
};

extern std::vector<DRAM *> dram_qu;
extern DRAM *cur;

void printData(long long cycle_num, std::string message);

void configQueue(DRAM *req);

void delete_redundant(unsigned int a);


#endif