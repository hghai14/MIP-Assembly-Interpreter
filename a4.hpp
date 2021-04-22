#ifndef A4
#define A4

#include <iostream>
#include <cstring>
#include <map>
#include <vector>
#include <algorithm>
#include <set>
#include <cmath>

extern int busy[32];
extern std::set<int> waiting;
extern std::set<unsigned int> busy_mem;

extern bool optimize;
extern long long row_updates_count;

// Memory array, 4 bytes at a time
extern unsigned int memory[262144];

// Register memory
extern unsigned int register_file[32];

class AccessType{

public:
    std::string message;
    int param;
    int cycles;

    AccessType(std::string message, int param, int cycles);
    ~AccessType();

};

class DRAM{

public:

    bool type;
    unsigned int param;
    unsigned int address;

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

extern std::vector<DRAM*> dram_qu;
extern DRAM* cur;

void printData(long long cycle_num, std::string message);

void configQueue(DRAM* req);

void delete_redundant(unsigned int a);

#endif