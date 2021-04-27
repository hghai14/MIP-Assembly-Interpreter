// Change

/* Deleted
#include <iostream>
#include <cstring>
#include <map>
#include <vector>
#include <algorithm>
#include <set>
#include <cmath>
*/

#include "a4.hpp"

std::string message;

// Memory array, 4 bytes at a time
unsigned int memory[262144];

long long int totalCycles = 0;

int n;

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

Core::Core(std::string path, unsigned int core_num)
{
    this->core_num = core_num;
    this->base_address = core_num*(((unsigned int) pow(2, 18)) / n);
    this->curParsePointer = this->base_address;
    instream.open(path, std::ofstream::in);
    if (!instream)
    {
        std::cerr << "Error: file \"" << path << "\" could not be opened" << std::endl;
        exit(-1);
    }
}

int main(int argc, char *argv[])
{

    if (argc < 2)
    {
        std::cout << "Please give input file path" << std::endl;
        exit(-1);
    }

    std::ifstream inFile;
    inFile.open(argv[1], std::ofstream::in);

    if (!inFile)
    {
        std::cerr << "Input correct file path" << std::endl;
        exit(-1);
    }

    std::cout << "Input the number of files: ";
    // return 0;
    std::string temp;
    getline(inFile, temp);
    n = std::stoi(temp);

    if (((unsigned int) pow(2, 18)) % n != 0)
    {
        std::cout << "Invalid number of cores given: Number of cores must be in power of 2";
        exit(-1);
    }

    Core *cores[n];

    for (int i = 0; i < n; i++)
    {
        std::cout << "Input the path of file " << (i + 1) << ": ";
        std::string path;
        getline(inFile, path);
        cores[i] = new Core(path, (unsigned int) i);
        cores[i]->compile();
        cores[i]->setup();
    }

    while (true)
    {
        bool f = false;
        for (unsigned int i = 0; i < n; i++)
        {
            f = f | cores[i]->execute();
        }
        if (!f)
        {
            break;
        }
        std::cout << totalCycles << std::endl;
    }

    std::cout << "Input DRAM row access delay: ";
    std::cin >> DRAM::ROW_ACCESS_DELAY;

    std::cout << "Input DRAM column access delay: ";
    std::cin >> DRAM::COL_ACCESS_DELAY;

    optimize = true;

    for (int i = 0; i < n; i++)
    {
        delete cores[i];
    }

    return 0;
}

// Decimal to hex converter for printing
std::string decToHex(unsigned int n)
{
    char output[10];
    output[0] = '0';
    output[1] = 'x';

    for (int i = 9; i >= 2; i--)
    {
        int temp = n % 16;
        if (temp < 10)
        {
            output[i] = (char)(temp + 48);
        }
        else
        {
            output[i] = (char)(temp + 97 - 10);
        }
        n /= 16;
    }

    return std::string(output, output + 10);
}

// Function to print register file
void printRegisterFile(unsigned int register_file[], unsigned int core_num)
{

    std::cout << std::endl
              << "Register file of core " << core_num << std::endl;

    for (int i = 0; i < 32; i++)
    {

        std::cout << "R" << i << "\t" << num_to_reg[i] << "\t" << decToHex(register_file[i]) << "\t" << (int)register_file[i] << std::endl;
    }

    std::cout << std::endl;
}

// Functions to throw runtime error
void throwRunTimeError(const std::string &message, unsigned int current, unsigned int core_num)
{
    std::cerr << "Runtime error at instruction number " << current << " in core " << core_num << std::endl;
    std::cerr << message << std::endl;
    exit(-1);
}

void throwRunTimeError(const std::string &message, unsigned int core_num)
{
    std::cerr << "Runtime error occured in core " << core_num << std::endl;
    std::cerr << message << std::endl;
    exit(-1);
}

// Functions to throw compilation error
void throwError(const std::string &message, int &lineNum, int &column, unsigned int core_num)
{
    std::cerr << "Syntax error: At line " << lineNum << ", column " << column + 1 << " in core " << core_num << std::endl;
    std::cerr << message << std::endl;
    exit(-1);
}

void throwError(const std::string &message, int &lineNum, unsigned int core_num)
{
    std::cerr << "Syntax error: At line " << lineNum << " in core " << core_num << std::endl;
    std::cerr << message << std::endl;
    exit(-1);
}

void printOverFlowMessage(std::string &s, unsigned int &current, unsigned int core_num)
{
    std::cerr << s << std::endl;
    std::cerr << "At instruction " << current << " in core " << core_num << std::endl;
}

void detectOFadd(int a, int b, unsigned int current, unsigned int core_num)
{
    std::string msg = "Warning : Overflow detected in addition";
    if ((b > 0) && (a > INT32_MAX - b))
    {
        printOverFlowMessage(msg, current, core_num);
    }
    else if ((b < 0) && (a < INT32_MIN - b))
    {
        printOverFlowMessage(msg, current, core_num);
    }
}

void detectOFmul(int a, int b, unsigned int current, unsigned int core_num)
{
    std::string msg = "Warning : Overflow detected in multiplication";
    // Change
    // Added
    if (b == 0)
    {
        return;
    }
    // end
    if ((a == -1) && (b == INT32_MIN))
    {
        printOverFlowMessage(msg, current, core_num);
    }
    else if ((b == -1) && (a == INT32_MIN))
    {
        printOverFlowMessage(msg, current, core_num);
    }
    else if (a > INT32_MAX / b)
    {
        printOverFlowMessage(msg, current, core_num);
    }
    else if ((a < INT32_MIN / b))
    {
        printOverFlowMessage(msg, current, core_num);
    }
}

void detectOFsub(int a, int b, unsigned int current, unsigned int core_num)
{
    std::string msg = "Warning : Overflow detected in subtraction";
    if ((b < 0) && (a > INT32_MAX + b))
    {
        printOverFlowMessage(msg, current, core_num);
    }
    else if ((b > 0) && (a < INT32_MIN + b))
    {
        printOverFlowMessage(msg, current, core_num);
    }
}

// Function to get the instruction type based on opcode of the instruction
InstructionType getInstructionType(unsigned int opcode, unsigned int &current, unsigned int core_num)
{
    for (std::pair<InstructionType, int> temp : op_codes)
    {
        if (temp.second == opcode)
        {
            return temp.first;
        }
    }
    throwRunTimeError("No such instruction", current, core_num);
    return lw;
}

// Funciton to execute the commands
void Core::executeCommand(InstructionType i_type, std::vector<unsigned int> &params)
{
    current++;
    // Change
    // Added
    DRAM *new_instr = nullptr;
    // end
    message = "";

    // Change
    // Added
    unsigned int to_check = -1;
    switch (i_type)
    {
    case jump:
        break;
    case bne:
        break;
    case beq:
        break;
    case sw:
        break;
    case addi:
        if (params[1] != params[0])
        {
            to_check = params[1];
        }
        break;
    case lw:
        if (params[2] != params[0])
        {
            to_check = params[0];
        }
        break;
    default:
        if (params[2] != params[0] && params[2] != params[1])
        {
            to_check = params[2];
        }
        break;
    }

    if (to_check != -1)
    {
        delete_redundant(to_check);
    }

    // end

    // Change
    // So many changes
    if (i_type == jump)
    {
        std::string label = labels[(int)params[0]];

        if (branches.find(label) == branches.end())
        {
            throwRunTimeError("Address not found: " + label, current);
            return;
        }
        current = branches[label];
        message = "Jumped to " + label;
        waiting.clear();
        instruction_count[jump]++;
    }
    else if (i_type == add)
    {
        // Changed
        if (busy[params[0]] > 0 ||
            busy[params[1]] > 0 ||
            busy[params[2]] > 0)
        {
            current--;
            waiting.insert(params[0]);
            waiting.insert(params[1]);
            waiting.insert(params[2]);
        }
        else
        {
            int a = register_file[params[0]];
            int b = register_file[params[1]];
            detectOFadd(a, b, current, core_num);
            register_file[params[2]] = (unsigned int)(a + b);
            message = "Added register this to this";
            message = "Add instruction executed, " +
                      num_to_reg[params[2]] + " = " + num_to_reg[params[0]] + " + " + num_to_reg[params[1]] + " = " +
                      std::to_string((int)register_file[params[2]]);
            waiting.clear();
            instruction_count[add]++;
        }
        // end
    }
    else if (i_type == sub)
    {
        if (busy[params[0]] > 0 ||
            busy[params[1]] > 0 ||
            busy[params[2]] > 0)
        {
            current--;
            waiting.insert(params[0]);
            waiting.insert(params[1]);
            waiting.insert(params[2]);
        }
        else
        {
            int a = register_file[params[0]];
            int b = register_file[params[1]];
            detectOFsub(a, b, current, core_num);
            register_file[params[2]] = (unsigned int)(a - b);
            message = "Subtract instruction executed, " +
                      num_to_reg[params[2]] + " = " + num_to_reg[params[0]] + " - " + num_to_reg[params[1]] + " = " +
                      std::to_string((int)register_file[params[2]]);
            waiting.clear();
            instruction_count[sub]++;
        }
    }
    else if (i_type == mul)
    {
        // Changed
        if (busy[params[0]] > 0 ||
            busy[params[1]] > 0 ||
            busy[params[2]] > 0)
        {
            current--;
            waiting.insert(params[0]);
            waiting.insert(params[1]);
            waiting.insert(params[2]);
        }
        else
        {
            int a = register_file[params[0]];
            int b = register_file[params[1]];
            detectOFmul(a, b, current, core_num);
            register_file[params[2]] = (unsigned int)(a * b);
            message = "Multiply instruction executed, " +
                      num_to_reg[params[2]] + " = " + num_to_reg[params[0]] + " * " + num_to_reg[params[1]] + " = " +
                      std::to_string((int)register_file[params[2]]);
            waiting.clear();
            instruction_count[mul]++;
        }
        // end
    }
    else if (i_type == slt)
    {
        if (busy[params[0]] > 0 ||
            busy[params[1]] > 0 ||
            busy[params[2]] > 0)
        {
            current--;
            waiting.insert(params[0]);
            waiting.insert(params[1]);
            waiting.insert(params[2]);
        }
        else
        {
            register_file[params[2]] = register_file[params[0]] < register_file[params[1]];
            message = "Set on less than instruction executed, " +
                      num_to_reg[params[2]] + " = " + num_to_reg[params[0]] + " < " + num_to_reg[params[1]] + " = " +
                      (register_file[params[0]] < register_file[params[1]] ? "1" : "0");
            waiting.clear();
            instruction_count[slt]++;
        }
    }
    else if (i_type == addi)
    {
        // Changed
        if (busy[params[0]] > 0 ||
            busy[params[1]] > 0)
        {
            current--;
            waiting.insert(params[0]);
            waiting.insert(params[1]);
        }
        else
        {
            unsigned int giv = params[2];
            int num = (int)(giv << 16);
            num = num >> 16;
            int a = register_file[params[0]];
            detectOFadd(a, num, current, core_num);
            register_file[params[1]] = (unsigned int)(a + num);
            message = "Add immediate instruction executed, " +
                      num_to_reg[params[1]] + " = " + num_to_reg[params[0]] + " + " + std::to_string(num) +
                      " = " + std::to_string((int)register_file[params[1]]);
            waiting.clear();
            instruction_count[addi]++;
        }
        // end
    }
    else if (i_type == beq)
    {
        // Changed
        if (busy[params[0]] > 0 ||
            busy[params[1]] > 0)
        {
            current--;
            waiting.insert(params[0]);
            waiting.insert(params[1]);
        }
        else
        {
            std::string label = labels[(int)params[2]];
            if (branches.find(label) == branches.end())
            {
                throwRunTimeError("Identifier not defined: " + label, current);
                return;
            }
            message = "Equality checked between registers " + num_to_reg[params[0]] + " and " + num_to_reg[params[1]] + ", ";
            if (register_file[params[0]] == register_file[params[1]])
            {
                current = branches[label];
                message += "Branched to the label " + label + "";
            }
            else
            {
                message += "No branching done";
            }
            waiting.clear();
            instruction_count[beq]++;
        }
        // end
    }
    else if (i_type == bne)
    {
        // Changed
        if (busy[params[0]] > 0 ||
            busy[params[1]] > 0)
        {
            current--;
            waiting.insert(params[0]);
            waiting.insert(params[1]);
        }
        else
        {
            std::string label = labels[(int)params[2]];
            if (branches.find(label) == branches.end())
            {
                throwRunTimeError("Identifier not defined: " + label, current);
                return;
            }
            message = "Inequality checked between registers " + num_to_reg[params[0]] + " and " + num_to_reg[params[1]] + ", ";
            if (register_file[params[0]] != register_file[params[1]])
            {
                current = branches[label];
                message += "Branched to the label " + label + "";
            }
            else
            {
                message += "No branching done";
            }
            waiting.clear();
            instruction_count[bne]++;
        }
        // end
    }
    else if (i_type == lw)
    {
        params[1] = params[1] << 16;
        int params1 = (int)params[1];
        params1 = params1 >> 16;
        int address = (params1 + (int)register_file[params[2]]);
        if (busy[params[0]] > 0 ||
            busy[params[2]] > 0 ||
            busy_mem.find(address) != busy_mem.end())
        {
            current--;
            waiting.insert(params[0]);
            waiting.insert(params[2]);
        }
        else
        {
            if (address >= pow(2, 20) || address / 4 < (int)endCommand)
            {
                throwRunTimeError("Invalid memory address", current);
            }
            if (address % 4 != 0)
            {
                throwRunTimeError("Invalid memory address: should be aligned with 4", current);
            }
            new_instr = new DRAM(true, params[0], (unsigned int)address);
            busy[params[0]] = 1;
            busy_mem.insert((unsigned int)address);
            message = "Load word instruction generated for DRAM from memory address " +
                      std::to_string(address) +
                      " to the register " + num_to_reg[params[0]];
            waiting.clear();
            instruction_count[lw]++;
        }
    }
    else if (i_type == sw)
    {
        params[1] = params[1] << 16;
        int params1 = (int)params[1];
        params1 = params1 >> 16;
        int address = (params1 + (int)register_file[params[2]]);

        if (busy[params[0]] > 0 ||
            busy[params[2]] > 0)
        {
            current--;
            waiting.insert(params[0]);
            waiting.insert(params[2]);
        }
        else
        {
            if (address >= pow(2, 20) || address / 4 < (int)endCommand)
            {
                throwRunTimeError("Invalid memory address", current);
            }
            if (address % 4 != 0)
            {
                throwRunTimeError("Invalid memory address: should be aligned with 4", current);
            }
            busy_mem.insert((unsigned int)address);
            new_instr = new DRAM(false, register_file[params[0]], (unsigned int)address);
            message = "Save word instruction generated for DRAM to memory address " +
                      std::to_string(address) +
                      " from the register " + num_to_reg[params[0]];
            waiting.clear();
            instruction_count[sw]++;
        }
    }
    // end

    // Change
    // Added
    printData(totalCycles, message);
    configQueue(new_instr);
    // end
}

// Get parameters for the instruction from the instruction stored as 32 bit in the memory
std::vector<unsigned int> getParams(InstructionType i_type, unsigned int &instr, unsigned int core_num)
{

    if (i_type == jump)
    {
        return std::vector<unsigned int>{instr & (unsigned int)67108863};
    }
    else if (i_type == add || i_type == sub || i_type == mul || i_type == slt)
    {
        return std::vector<unsigned int>{(instr >> 21) & (unsigned int)31,
                                         (instr >> 16) & (unsigned int)31,
                                         (instr >> 11) & (unsigned int)31};
    }
    else if (i_type == addi)
    {
        unsigned int num = instr & (unsigned int)67108863;
        return std::vector<unsigned int>{(instr >> 21) & (unsigned int)31,
                                         (instr >> 16) & (unsigned int)31,
                                         num};
    }
    else if (i_type == beq || i_type == bne)
    {
        return std::vector<unsigned int>{(instr >> 16) & (unsigned int)31,
                                         (instr >> 21) & (unsigned int)31,
                                         instr & (unsigned int)65535};
    }
    else if (i_type == lw)
    {
        return std::vector<unsigned int>{(instr >> 16) & (unsigned int)31,
                                         instr & (unsigned int)65535,
                                         (instr >> 21) & (unsigned int)31};
    }
    else if (i_type == sw)
    {
        return std::vector<unsigned int>{(instr >> 21) & (unsigned int)31,
                                         instr & (unsigned int)65535,
                                         (instr >> 16) & (unsigned int)31};
    }
    else
    {
        throwRunTimeError("Invalid instruction", (unsigned int)0, core_num);
    }

    return std::vector<unsigned int>{};
}

void Core::setup()
{
    if (branches.find("main") == branches.end())
    {
        throwRunTimeError("Address not found: main", core_num);
    }
    else
    {
        // Initialize program counter
        current = branches["main"];
    }
}

// Function to execute the program
bool Core::execute()
{
    // if (branches.find("main") == branches.end())
    // {
    //     throwRunTimeError("Address not found: main", core_num);
    //     return false;
    // }
    // else
    // {
    //     // Initialize program counter
    //     current = branches["main"];
    // }

    // While program counter is within the range of instructions, execute instructions
    // Change
    // while (current < endCommand  || dram_qu.size() > 0){
    if (current < endCommand)
    {
        // end
        unsigned int instr = memory[current];
        unsigned int opcode = instr >> 26;

        InstructionType i_type = getInstructionType(opcode, current, core_num);

        std::vector<unsigned int> params = getParams(i_type, instr, core_num);
        executeCommand(i_type, params);

        register_file[0] = (unsigned int) 0;

        totalCycles++;

        return true;
    }
    else
    {
        return false;
    }

    // message = "";
    // while (cur != nullptr || dram_qu.size() > 0)
    // {
    //     printData(totalCycles, message);
    //     configQueue(nullptr);
    //     register_file[0] = (unsigned int)0;
    //     totalCycles++;
    // }
    // printRegisterFile(register_file, core_num);
    // std::cout << std::endl
    //           << "Total row updates = " << row_updates_count << std::endl;
    // // end

    // std::cout << std::endl
    //           << "Total clock cycles: " << totalCycles << std::endl
    //           << std::endl;

    // std::cout << "Number of times each instruction is executed: " << std::endl
    //           << std::endl;
    // for (std::pair<InstructionType, long long int> p : instruction_count)
    // {
    //     std::cout << instruction_type_string[(int)p.first] << "\t" << p.second << std::endl;
    // }
}

// To check if a char is a whitespace
bool isSpace(const char &c)
{
    return c == ' ' || c == '\t';
}

// To check if a char is an english alphabet
bool isAlpha(const char &c)
{
    int i = (int)c;
    return (i > 96 && i < 97 + 26) || (i > 64 && i < 65 + 26);
}

// To check if a char is an integer
bool isNum(const char &c)
{
    return ((int)c) >= 48 && ((int)c) <= 57;
}

// Sets the value of i to the next non space char in the string
void toNextNonSpace(std::string &s, int &i)
{
    if (i >= s.length())
    {
        return;
    }
    while (s.at(i) == ' ' || s.at(i) == '\t')
    {
        i++;
        if (i >= s.length())
        {
            return;
        }
    }
}

// Returns the arguments for the instruciton after instruction type is detected
std::vector<std::string> getTokens(std::string &s, int &i, int &lineNum)
{
    std::vector<std::string> res;
    int state = 0;
    int pre = i;
    for (int j = i; j < s.length(); j++)
    {
        switch (state)
        {
        case 0:
            if (s.at(j) == ',')
            {
                state = 2;
                res.push_back(s.substr(pre, j - pre));
            }
            else if (isSpace(s.at(j)))
            {
                state = 1;
                res.push_back(s.substr(pre, j - pre));
            }
            break;
        case 1:
            if (s.at(j) == ',')
            {
                state = 2;
            }
            else if (!isSpace(s.at(j)))
            {
                state = 0;
                pre = j;
            }
            break;
        case 2:
            if (s.at(j) == ',')
            {
                throwError("Invalid input: Not expecting a ','", lineNum, j);
            }
            else if (!isSpace(s.at(j)))
            {
                state = 0;
                pre = j;
            }
            break;
        default:
            break;
        }
    }
    if (state == 0)
    {
        res.push_back(s.substr(pre));
    }

    return res;
}

std::vector<std::string> getLoadSaveTokens(std::string &s, int &i, int &lineNum, unsigned int core_num)
{
    std::vector<std::string> res;
    int j = i;
    while (!isSpace(s.at(i)) && s.at(i) != ',')
    {
        i++;
        if (i >= s.length())
        {
            throwError("Invaid input", lineNum, core_num);
        }
    }
    res.push_back(s.substr(j, i - j));
    // Change
    // while (!isNum(s.at(i))){
    while (!isNum(s.at(i)) && s.at(i) != '-')
    {
        // end
        i++;
        if (i >= s.length())
        {
            throwError("Invaid input", lineNum, core_num);
        }
    }
    j = i;
    // Change
    // Added
    if (s.at(i) == '-')
    {
        i++;
    }
    // end
    while (isNum(s.at(i)))
    {
        i++;
        if (i >= s.length())
        {
            throwError("Invaid input", lineNum, core_num);
        }
    }
    res.push_back(s.substr(j, i - j));
    while (s.at(i) != '(')
    {
        if (!isSpace(s.at(i)))
        {
            throwError("Invaid input", lineNum, i, core_num);
        }
        i++;
        if (i >= s.length())
        {
            throwError("Invaid input", lineNum, core_num);
        }
    }
    i++;
    toNextNonSpace(s, i);
    if (i >= s.length())
    {
        throwError("Invalid input", lineNum, core_num);
    }
    j = i;
    while (!isSpace(s.at(i)) && s.at(i) != ')')
    {
        i++;
        if (i >= s.length())
        {
            throwError("Invalid input", lineNum, core_num);
        }
    }
    res.push_back(s.substr(j, i - j));
    toNextNonSpace(s, i);
    if (s.at(i) != ')')
    {
        throwError("Invalid input: expected a \')\'", lineNum, i, core_num);
    }
    i++;
    toNextNonSpace(s, i);
    if (i < s.length())
    {
        throwError("Invalid input", lineNum, core_num);
    }
    return res;
}

// Parse function to parse the text
void Core::parse(std::string s, int &lineNum)
{
    std::string arg1;
    int i = 0;

    // Ignore starting whitespace
    toNextNonSpace(s, i);

    if (i >= s.length())
    {
        lineNum++;
        return;
    }

    // A line cannot start with a non-alpha character
    if (!isAlpha(s.at(i)))
    {
        throwError("Expected an english alphabet", lineNum, i, core_num);
    }

    int j = i++;

    while (isAlpha(s.at(i)) || isNum(s.at(i)))
    {
        i++;
        if (i == s.length())
        {
            throwError("Invalid input", lineNum, i, core_num);
        }
    }

    // First argument of the instruction
    arg1 = s.substr(j, i - j);

    toNextNonSpace(s, i);

    if (i >= s.length())
    {
        throwError("Invalid input", lineNum, core_num);
    }

    // To check if the instruction is a branch
    if (s.at(i) == ':')
    {
        if (branches.find(arg1) != branches.end())
        {
            throwError("Invalid input: Duplicate labels found", lineNum, core_num);
        }
        branches.insert(make_pair(arg1, curParsePointer));
        i++;
        toNextNonSpace(s, i);
        if (i >= s.length())
        {
            return;
        }
        if (!isAlpha(s.at(i)))
        {
            throwError("Expected an alphabet got a number!", lineNum, i, core_num);
        }
        int k = i++;

        while (isAlpha(s.at(i)) || isNum(s.at(i)))
        {
            i++;
            if (i == s.length())
            {
                throwError("Invalid input", lineNum, i, core_num);
            }
        }
        arg1 = s.substr(k, i - k);
        toNextNonSpace(s, i);
    }

    if (i >= s.length())
    {
        throwError("Expecting an argument", lineNum, core_num);
    }

    std::vector<std::string> tokens;

    if (arg1 == "lw" || arg1 == "sw")
    {
        tokens = getLoadSaveTokens(s, i, lineNum, core_num);
    }
    else
    {
        tokens = getTokens(s, i, lineNum);
    }

    if (arg1 == "j")
    {
        parseJump(tokens, lineNum);
    }
    else if (arg1 == "add")
    {
        parseAdd(tokens, lineNum);
    }
    else if (arg1 == "sub")
    {
        parseSub(tokens, lineNum);
    }
    else if (arg1 == "mul")
    {
        parseMul(tokens, lineNum);
    }
    else if (arg1 == "beq")
    {
        parseBeq(tokens, lineNum);
    }
    else if (arg1 == "bne")
    {
        parseBne(tokens, lineNum);
    }
    else if (arg1 == "slt")
    {
        parseSlt(tokens, lineNum);
    }
    else if (arg1 == "lw")
    {
        parseLw(tokens, lineNum);
    }
    else if (arg1 == "sw")
    {
        parseSw(tokens, lineNum);
    }
    else if (arg1 == "addi")
    {
        parseAddi(tokens, lineNum);
    }
    else
    {
        throwError("Invalid token found " + arg1, lineNum, i, core_num);
    }

    lineNum++;
}

// Function to compile the program
void Core::compile()
{

    curParsePointer = 0;
    int lineNum = 1;

    while (!feof(stdin))
    {

        std::string s;
        std::getline(instream, s);

        parse(s, lineNum);
    }

    endCommand = curParsePointer;

    register_file[register_map["sp"]] = (unsigned int)pow(2, 20);
}

// Function to parse the instruction and store it in the memory
void Core::parseInstruction(InstructionType i_type, std::vector<int> params, int &lineNum)
{
    if (i_type == jump)
    {
        unsigned int address = params[0];
        unsigned int opcode = op_codes[jump] << 26;
        memory[curParsePointer++] = address + opcode;
    }
    else if (i_type == add || i_type == sub || i_type == mul || i_type == slt)
    {
        unsigned int rs = params[1] << 21;
        unsigned int rt = params[2] << 16;
        unsigned int rd = params[0] << 11;
        unsigned int opcode = op_codes[i_type] << 26;
        memory[curParsePointer++] = opcode + rs + rt + rd;
    }
    else if (i_type == addi)
    {
        unsigned int rs = params[1] << 21;
        unsigned int rt = params[0] << 16;
        if (params[2] > INT16_MAX || params[2] < INT16_MIN)
            throwError("Integer out of range", lineNum, core_num);
        unsigned int num = params[2] & (((unsigned int)(-1)) >> 16);
        unsigned int opcode = op_codes[i_type] << 26;
        memory[curParsePointer++] = opcode + rs + rt + num;
    }
    else if (i_type == beq || i_type == bne)
    {
        unsigned int rs = params[1] << 21;
        unsigned int rt = params[0] << 16;
        unsigned int address = params[2];
        unsigned int opcode = op_codes[i_type] << 26;
        memory[curParsePointer++] = opcode + rs + rt + address;
    }
    else if (i_type == lw)
    {
        unsigned int rs = params[2] << 21;
        unsigned int rt = params[0] << 16;
        // Change
        // unsigned int shamt = params[1];
        unsigned int shamt = ((int)pow(2, 16) - 1) & params[1];
        // end
        unsigned int opcode = op_codes[i_type] << 26;
        memory[curParsePointer++] = opcode + rs + rt + shamt;
    }
    else if (i_type == sw)
    {
        unsigned int rs = params[0] << 21;
        unsigned int rt = params[2] << 16;
        // Change
        // unsigned int shamt = params[1];
        unsigned int shamt = ((int)pow(2, 16) - 1) & params[1];
        // end
        unsigned int opcode = op_codes[i_type] << 26;
        memory[curParsePointer++] = opcode + rs + rt + shamt;
    }
    else
    {
    }
}

// Function to parse intructions which need 3 arguments
void Core::parse3Arg(std::vector<std::string> &tokens, int &lineNum, InstructionType i_type)
{
    if (tokens.size() != 3)
    {
        throwError("Invalid input", lineNum, core_num);
        return;
    }

    for (int i = 0; i < 3; i++)
    {
        if (tokens[i].length() < 2)
        {
            throwError("Invalid input", lineNum, core_num);
        }
        if (tokens[i].at(0) != '$')
        {
            throwError("Invalid input", lineNum, core_num);
        }
    }

    std::string arg1 = tokens[0].substr(1);
    std::string arg2 = tokens[1].substr(1);
    std::string arg3 = tokens[2].substr(1);

    if (register_map.find(arg1) == register_map.end())
    {
        throwError("Invalid register", lineNum, core_num);
        return;
    }
    if (register_map.find(arg2) == register_map.end())
    {
        throwError("Invalid register", lineNum, core_num);
        return;
    }
    if (register_map.find(arg3) == register_map.end())
    {
        throwError("Invalid register", lineNum, core_num);
        return;
    }

    parseInstruction(i_type, std::vector<int>{register_map[arg1], register_map[arg2], register_map[arg3]}, lineNum);
}

// Function to parse jump instruction
void Core::parseJump(std::vector<std::string> &tokens, int &lineNum)
{

    if (tokens.size() != 1)
    {
        throwError("Invalid input", lineNum, core_num);
    }

    std::string arg = tokens[0];

    labels.insert(std::make_pair(label_ptr, arg));

    parseInstruction(jump, std::vector<int>{label_ptr}, lineNum);
    label_ptr++;
}

// Function to parse add instruction
void Core::parseAdd(std::vector<std::string> &tokens, int &lineNum)
{
    parse3Arg(tokens, lineNum, add);
}

// Function to parse sub instruction
void Core::parseSub(std::vector<std::string> &tokens, int &lineNum)
{
    parse3Arg(tokens, lineNum, sub);
}

// Function to parse mul instruction
void Core::parseMul(std::vector<std::string> &tokens, int &lineNum)
{
    Core::parse3Arg(tokens, lineNum, mul);
}

// Function to parse slt instruction
void Core::parseSlt(std::vector<std::string> &tokens, int &lineNum)
{
    Core::parse3Arg(tokens, lineNum, slt);
}

// Function to parse addi instruction
void Core::parseAddi(std::vector<std::string> &tokens, int &lineNum)
{

    if (tokens.size() != 3)
    {
        throwError("Invalid input", lineNum, core_num);
    }

    for (int i = 0; i < 2; i++)
    {
        if (tokens[i].length() < 2)
        {
            throwError("Invalid input", lineNum, core_num);
        }
        if (tokens[i].at(0) != '$')
        {
            throwError("Invalid input", lineNum, core_num);
        }
    }

    int num;
    try
    {
        num = std::stoi(tokens[2]);
    }
    catch (std::exception &e)
    {
        throwError("Invalid integer argument", lineNum, core_num);
        return;
    }

    std::string arg1 = tokens[0].substr(1);
    std::string arg2 = tokens[1].substr(1);

    if (register_map.find(arg1) == register_map.end())
    {
        throwError("Invalid register", lineNum, core_num);
        return;
    }
    if (register_map.find(arg2) == register_map.end())
    {
        throwError("Invalid register", lineNum, core_num);
        return;
    }

    parseInstruction(addi, std::vector<int>{register_map[arg1], register_map[arg2], num}, lineNum);
}

// Function to parse beq instruction
void Core::parseBeq(std::vector<std::string> &tokens, int &lineNum)
{
    if (tokens.size() != 3)
    {
        throwError("Invalid input", lineNum, core_num);
    }

    for (int i = 0; i < 2; i++)
    {
        if (tokens[i].length() < 2)
        {
            throwError("Invalid input", lineNum, core_num);
        }
        if (tokens[i].at(0) != '$')
        {
            throwError("Invalid input", lineNum, core_num);
        }
    }
    std::string arg1 = tokens[0].substr(1);
    std::string arg2 = tokens[1].substr(1);
    std::string arg3 = tokens[2];

    if (register_map.find(arg1) == register_map.end())
    {
        throwError("Invalid register", lineNum, core_num);
        return;
    }
    if (register_map.find(arg2) == register_map.end())
    {
        throwError("Invalid register", lineNum, core_num);
        return;
    }

    labels.insert(std::make_pair(label_ptr, arg3));

    parseInstruction(beq, std::vector<int>{register_map[arg1], register_map[arg2], label_ptr}, lineNum);
    label_ptr++;
}

// Function to parse bne instruction
void Core::parseBne(std::vector<std::string> &tokens, int &lineNum)
{
    if (tokens.size() != 3)
    {
        throwError("Invalid input", lineNum, core_num);
    }

    for (int i = 0; i < 2; i++)
    {
        if (tokens[i].length() < 2)
        {
            throwError("Invalid input", lineNum, core_num);
        }
        if (tokens[i].at(0) != '$')
        {
            throwError("Invalid input", lineNum, core_num);
        }
    }
    std::string arg1 = tokens[0].substr(1);
    std::string arg2 = tokens[1].substr(1);
    std::string arg3 = tokens[2];

    if (register_map.find(arg1) == register_map.end())
    {
        throwError("Invalid register", lineNum, core_num);
        return;
    }
    if (register_map.find(arg2) == register_map.end())
    {
        throwError("Invalid register", lineNum, core_num);
        return;
    }
    labels.insert(std::make_pair(label_ptr, arg3));

    parseInstruction(bne, std::vector<int>{register_map[arg1], register_map[arg2], label_ptr}, lineNum);
    label_ptr++;
}

// Function to parse lw instruction
void Core::parseLw(std::vector<std::string> &tokens, int &lineNum)
{
    std::string arg1 = tokens[0].substr(1);
    std::string arg3 = tokens[2].substr(1);

    int arg2;
    try
    {
        arg2 = stoi(tokens[1]);
    }
    catch (std::exception &e)
    {
        throwError("Invalid offset", lineNum, core_num);
    }

    if (register_map.find(arg1) == register_map.end())
    {
        throwError("Invalid register", lineNum, core_num);
        return;
    }

    if (register_map.find(arg3) == register_map.end())
    {
        throwError("Invalid register", lineNum, core_num);
        return;
    }

    parseInstruction(lw, std::vector<int>{register_map[arg1], arg2, register_map[arg3]}, lineNum);
}

// Function to parse sw instruction
void Core::parseSw(std::vector<std::string> &tokens, int &lineNum)
{

    std::string arg1 = tokens[0].substr(1);
    std::string arg3 = tokens[2].substr(1);

    int arg2;
    try
    {
        arg2 = stoi(tokens[1]);
    }
    catch (std::exception &e)
    {
        throwError("Invalid offset", lineNum, core_num);
    }

    if (register_map.find(arg1) == register_map.end())
    {
        throwError("Invalid register", lineNum, core_num);
        return;
    }

    if (register_map.find(arg3) == register_map.end())
    {
        throwError("Invalid register", lineNum, core_num);
        return;
    }

    parseInstruction(sw, std::vector<int>{register_map[arg1], arg2, register_map[arg3]}, lineNum);
}
