#include "a5.hpp"

// Memory array, 4 bytes at a time
unsigned int DRAM::memory[262144];

// Total number of cores
int n;

// Count of total cycles
long long int totalCycles = 0;

// Vector of number of cores
std::vector<Core*> DRAM::cores;

std::string Core::instruction_type_string[] = {"j", "add", "sub", "mul", "beq", "bne", "slt", "lw", "sw", "addi"};

// Set to check a reserved word
std::set<std::string> Core::reserved_words{"add", "j", "sub", "mul", "beq", "bne", "slt", "lw", "sw", "addi"};

std::map<InstructionType, int> Core::op_codes({std::make_pair(add, 32),
                                         std::make_pair(slt, 42),
                                         std::make_pair(sub, 34),
                                         std::make_pair(jump, 2),
                                         std::make_pair(mul, 24),
                                         std::make_pair(beq, 4),
                                         std::make_pair(bne, 5),
                                         std::make_pair(lw, 35),
                                         std::make_pair(sw, 43),
                                         std::make_pair(addi, 8)});

std::map<std::string, int> Core::register_map = {
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

std::map<int, std::string> Core::num_to_reg = {
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

Core::Core(std::string path, unsigned int core_num)
{
    // Inactive when any parse or run time error
    this->active = true;

    // Core number
    this->core_num = core_num;

    // Base address of the memory allocated to this core
    this->base_address = core_num * (((unsigned int)pow(2, 18)) / n) * 4;

    // Parse pointer to allocate memory
    this->curParsePointer = 0;

    // Input stream from file
    instream.open(path, std::ofstream::in);
    if (!instream)
    {
        std::cerr << "Error: file \"" << path << "\" could not be opened" << std::endl;
        exit(-1);
    }

    // Boolean variable that asserts whether register file is busy or not
    writeBusy = false;

    // Reseting wait buffer of the memory manager
    memset(waitReg, 0, sizeof(waitReg));

    // Memory address waiting
    waitMem = -1;

    // Reseting load request qu
    memset(loadQu, 0, sizeof(loadQu));

    // Reseting save request queue
    memset(saveQu, 0, sizeof(saveQu));

    // Initializing best request to -1
    bestReq[0] = bestReq[1] = -1;

    // Count for number of pending requests
    pendingRequests = 0;

    for (int i = 0; i < 32; i++)
    {
        loadQu[i] = Request::null;
    }

    for (int i = 0; i < saveQuBufferLength; i++)
    {
        saveQu[i] = Request::null;
    }
}

int main(int argc, char *argv[])
{
    // Check whether sufficient arguments given or not
    if (argc < 2)
    {
        std::cout << "Please give input file path" << std::endl;
        exit(-1);
    }

    freopen("out.txt", "w", stdout);

    // Read stream for input file
    std::ifstream inFile(argv[1], std::ofstream::in);
    if (!inFile)
    {
        std::cerr << "Input correct file path" << std::endl;
        exit(-1);
    }

    int m;

    // Reading number of cores and row and column access delay
    inFile >> n >> m >>  DRAM::ROW_ACCESS_DELAY >> DRAM::COL_ACCESS_DELAY;

    // Flushing the buffer to next line
    std::string temp;
    getline(inFile, temp);

    // Loop to read file paths and create objects of Core class
    for (int i = 0; i < n; i++)
    {
        // Input file path
        std::string path;
        getline(inFile, path);

        // Push the core in the vector
        DRAM::cores.push_back(new Core(path, (unsigned int) i));

        // Try compiling the core
        try
        {
            // Compile the core and setup its program counter to main branch
            DRAM::cores[i]->compile();
            DRAM::cores[i]->setup();
        }catch (char* exception)
        {
            // In case of error, inactive the core
            DRAM::cores[i]->active = false;
        }
    }

    // While the program is running, execute all the cores and increment a cycle everytime
    while (totalCycles < m)
    {
        // Increament the cycle count
        totalCycles++;

        // Bool var to detect if there exists and active core
        bool f = false;

        f = f | DRAM::execute();

        // Loop through all the cores
        for (unsigned int i = 0; i < n; i++)
        {
            // Try to execute the instruction at current program counter of the core
            try
            {
                // To detect if the core is active or not
                f = f | DRAM::cores[i]->execute();
            }catch (char* exception)
            {
                // In case of error, deactivate the core
                DRAM::cores[i]->active = false;
            }
        }

        // If all the cores and DRAM are inactive then break
        if (!f)
        {
            break;
        }

        // Print the cycle status
        std::cout << "Cycle: " << totalCycles << std::endl;

        // Loop through each core and print the message
        for (unsigned int i = 0; i < n; i++)
        {
            std::cout << "Core " << i << ": " << DRAM::cores[i]->message << std::endl;
        } 

        // Print the DRAM message seprately
        std::cout << "DRAM: " << DRAM::message << std::endl << std::endl;
    }

    int totalIntructions = 0;

    // Print the stats of each core and free the space in heap (Good practice)
    for (int i = 0; i < n; i++)
    {
        std::cout << "Core " << i << " process data: " << std::endl;
        DRAM::cores[i]->printData();

        for (std::pair<InstructionType, int> p: DRAM::cores[i]->instruction_count)
        {
            totalIntructions += p.second;
        }

        delete DRAM::cores[i];
    }

    // Print throughput
    std::cout << std::endl << "Throughput: " << (float) totalIntructions / totalCycles << " Instructions per cycle" << std::endl;

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
void Core::printRegisterFile(unsigned int register_file[], unsigned int core_num)
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
    throw "exception";
}

void throwRunTimeError(const std::string &message, unsigned int core_num)
{
    std::cerr << "Runtime error occured in core " << core_num << std::endl;
    std::cerr << message << std::endl;
    throw "exception";
}

// Functions to throw compilation error
void throwError(const std::string &message, int &lineNum, int &column, unsigned int core_num)
{
    std::cerr << "Syntax error: At line " << lineNum << ", column " << column + 1 << " in core " << core_num << std::endl;
    std::cerr << message << std::endl;
    throw "exception";
}

void throwError(const std::string &message, int &lineNum, unsigned int core_num)
{
    std::cerr << "Syntax error: At line " << lineNum << " in core " << core_num << std::endl;
    std::cerr << message << std::endl;
    throw "exception";
}

void printOverFlowMessage(std::string &s, unsigned int &current, unsigned int core_num)
{
    std::cerr << s << std::endl;
    std::cerr << "At instruction " << current << " in core " << core_num << std::endl;
}

// Funciton to detect overflow in addition
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

// Function to detect overlow in multiplication
void detectOFmul(int a, int b, unsigned int current, unsigned int core_num)
{
    std::string msg = "Warning : Overflow detected in multiplication";
    if (b == 0)
    {
        return;
    }
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

// Funciton to detect subtraction
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
InstructionType Core::getInstructionType(unsigned int opcode, unsigned int &current, unsigned int core_num)
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

// Setup function of the core, sets the program counter to the main branch
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
    // Sets the best request at the memory manager buffer
    setBestRequest();

    // Initialize the message to null

    // If the program counter is less than the last instruction then procesd
    if (current < endCommand)
    {
        message = "Stalled";
        // Decode instruction into op code and instruction
        unsigned int instr = instruction_memory[current];
        unsigned int opcode = instr >> 26;

        // Get instruction type from op code
        InstructionType i_type = getInstructionType(opcode, current, core_num);

        // Get params from the instruction
        std::vector<unsigned int> params = getParams(i_type, instr, core_num);
        
        // Execute command based on intruction type and parameters
        executeCommand(i_type, params);

        // Hard wire register zero to 0
        register_file[0] = (unsigned int)0;

        return true;
    }
    else if (pendingRequests > 0)
    {
        message = "Stalled";
        // Else if any request pending then proceed
        return true;
    }
    else
    {
        message = "Finshed";
        // Else return false because no work to do
        return false;
    }
}

// ********************************************************
// ********************************************************
// ****************** PARSING CODE BELOW ******************
// ********************************************************
// ********************************************************

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
    while (!isNum(s.at(i)) && s.at(i) != '-')
    {
        i++;
        if (i >= s.length())
        {
            throwError("Invaid input", lineNum, core_num);
        }
    }
    j = i;
    if (s.at(i) == '-')
    {
        i++;
    }
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
            lineNum++;
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
    // Set the parse pointer to 0
    curParsePointer = 0;
    
    // Current line number to 1
    int lineNum = 1;
    
    // String buffer for input
    std::string s;

    // Loop till any line available
    while (std::getline(instream, s))
    {
        // Parse the line
        parse(s, lineNum);
    }

    // Once the parsing done, set the endCommand pointer
    endCommand = curParsePointer;

    // Set the stack pointer the base address of the memory
    register_file[register_map["sp"]] = base_address;
}

// Function to parse the instruction and store it in the memory
void Core::parseInstruction(InstructionType i_type, std::vector<int> params, int &lineNum)
{
    if (i_type == jump)
    {
        unsigned int address = params[0];
        unsigned int opcode = op_codes[jump] << 26;
        instruction_memory[curParsePointer++] = address + opcode;
    }
    else if (i_type == add || i_type == sub || i_type == mul || i_type == slt)
    {
        unsigned int rs = params[1] << 21;
        unsigned int rt = params[2] << 16;
        unsigned int rd = params[0] << 11;
        unsigned int opcode = op_codes[i_type] << 26;
        instruction_memory[curParsePointer++] = opcode + rs + rt + rd;
    }
    else if (i_type == addi)
    {
        unsigned int rs = params[1] << 21;
        unsigned int rt = params[0] << 16;
        if (params[2] > INT16_MAX || params[2] < INT16_MIN)
            throwError("Integer out of range", lineNum, core_num);
        unsigned int num = params[2] & (((unsigned int)(-1)) >> 16);
        unsigned int opcode = op_codes[i_type] << 26;
        instruction_memory[curParsePointer++] = opcode + rs + rt + num;
    }
    else if (i_type == beq || i_type == bne)
    {
        unsigned int rs = params[1] << 21;
        unsigned int rt = params[0] << 16;
        unsigned int address = params[2];
        unsigned int opcode = op_codes[i_type] << 26;
        instruction_memory[curParsePointer++] = opcode + rs + rt + address;
    }
    else if (i_type == lw)
    {
        unsigned int rs = params[2] << 21;
        unsigned int rt = params[0] << 16;
        unsigned int shamt = ((int)pow(2, 16) - 1) & params[1];
        unsigned int opcode = op_codes[i_type] << 26;
        instruction_memory[curParsePointer++] = opcode + rs + rt + shamt;
    }
    else if (i_type == sw)
    {
        unsigned int rs = params[0] << 21;
        unsigned int rt = params[2] << 16;
        unsigned int shamt = ((int)pow(2, 16) - 1) & params[1];
        unsigned int opcode = op_codes[i_type] << 26;
        instruction_memory[curParsePointer++] = opcode + rs + rt + shamt;
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