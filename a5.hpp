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

class Request
{
public:
    bool load, valid = true, busy = true;
    
    unsigned int address;
    
    // Save -> Word; Load -> Reg
    unsigned int reg;

    unsigned int row;

    static Request null;

    Request(bool load, unsigned int address, unsigned int reg);
    
    Request()
    {
        valid = false;
        busy = false;
        load = 0;
        address = 0;
        reg = 0;
        row = 0;
    }

    bool operator==(const Request& r)
    {
        return r.load == load && r.address == address && r.reg == reg;
    }

    bool operator!=(const Request& r)
    {
        return r.load != load || r.address != address || r.reg != reg;
    }
};

class Core
{

private:
    std::ifstream instream;

    static std::string instruction_type_string[10];
    static std::set<std::string> reserved_words;
    static std::map<InstructionType, int> op_codes;
    static std::map<std::string, int> register_map;

public:

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

    static std::map<int, std::string> num_to_reg;

    // Size of save buffer
    static const int saveQuBufferLength = 64;

    // Write port is busy or not
    bool writeBusy;
    
    // To check if the register is waiting or not
    bool waitReg[32];

    // To check if the memory is waiting
    unsigned int waitMem;

    // Load buffer
    Request loadQu[32];

    // Save buffer
    Request saveQu[saveQuBufferLength];

    // Best request,if  bestReq[0] = 1 then load else save, bestReq[1] -> address of request
    int bestReq[2];

    Request bestRequest;

    // Number of pending requests
    int pendingRequests;

    // Processor is active or not, inactive in case of compilation or execution error
    bool active;

    // Message to print cycle info
    std::string message;

    // To check the core number
    unsigned int core_num;

    // Base address of the memory allocated to the core
    unsigned int base_address;

    // Register file
    unsigned int register_file[32];

    // Instruction memory
    unsigned int instruction_memory[1024];
    
    // For parsing
    unsigned int curParsePointer;

    // Last Instruciton address
    unsigned int endCommand;
    
    // PC
    unsigned int current;

    // Branches
    std::map<std::string, unsigned int> branches;

    // Labels
    std::map<unsigned int, std::string> labels;

    // Parsing data
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

    // Print execution data after completion of execution
    void printData();

    // Add memory request to load or save buffer
    void addRequest(Request req);

    // Get the next best request
    Request getNextRequest();

    // Set the best request
    void setBestRequest();
};

class DRAM_Req
{
public:
    Core *core;
    Request req;

    static DRAM_Req null;

    DRAM_Req()
    {

    }

    bool operator==(DRAM_Req& r)
    {
        return r.core == core && r.req == req;
    }

    bool operator!=(DRAM_Req& r)
    {
        return r.req != req || r.core != core;
    }
};

class DRAM
{
public:
    // Memory
    static unsigned int memory[262144];
    
    // Cores
    static std::vector<Core *> cores;

    // Delays
    static int ROW_ACCESS_DELAY;
    static int COL_ACCESS_DELAY;

    // Messages
    static std::string message;
    static std::string mrm_message;

    // Currently active row
    static int activeRow;
    
    // Cycles left for DRAM function
    static int writeLeft;
    static int rowLeft;
    static int colLeft;

    // Number of cycles left for mrm
    static int mrmWaitLeft;

    // Request currently processed by DRAM, else nullptr
    static DRAM_Req activeRequest, tempRequest;

    // true when DRAM busy
    static bool busy;

    // Called on every cycle
    static bool execute();

    // Process DRAM execution
    static void process();

    // Returns the next request to be processed, called only when, DRAM not busy
    static DRAM_Req getNextRequest();
};

#endif