#include "Core.hpp"

Core::Core(std::string path, unsigned int core_num)
{
    this->core_num = core_num;
    this->base_address = core_num * (((unsigned int)pow(2, 18)) / n);
    this->curParsePointer = this->base_address;
    instream.open(path, std::ofstream::in);
    if (!instream)
    {
        std::cerr << "Error: file \"" << path << "\" could not be opened" << std::endl;
        exit(-1);
    }
}

void Core::executeJ(std::vector<unsigned int> &params)
{
    std::string label = labels[(int)params[0]];

    if (branches.find(label) == branches.end())
    {
        throwRunTimeError("Address not found: " + label, current);
        return;
    }
    current = branches[label];
    message = "Jumped to " + label;
    instruction_count[jump]++;
}

void Core::executeAddi(std::vector<unsigned int> &params)
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
    instruction_count[addi]++;
}

void Core::executeAdd(std::vector<unsigned int> &params)
{
    int a = register_file[params[0]];
    int b = register_file[params[1]];
    detectOFadd(a, b, current, core_num);
    register_file[params[2]] = (unsigned int)(a + b);
    message = "Added register this to this";
    message = "Add instruction executed, " +
                num_to_reg[params[2]] + " = " + num_to_reg[params[0]] + " + " + num_to_reg[params[1]] + " = " +
                std::to_string((int)register_file[params[2]]);
    instruction_count[add]++;
}

void Core::executeSub(std::vector<unsigned int> &params)
{
    int a = register_file[params[0]];
    int b = register_file[params[1]];
    detectOFsub(a, b, current, core_num);
    register_file[params[2]] = (unsigned int)(a - b);
    message = "Subtract instruction executed, " +
                num_to_reg[params[2]] + " = " + num_to_reg[params[0]] + " - " + num_to_reg[params[1]] + " = " +
                std::to_string((int)register_file[params[2]]);
    instruction_count[sub]++;
}

void Core::executeMul(std::vector<unsigned int> &params)
{
    int a = register_file[params[0]];
    int b = register_file[params[1]];
    detectOFmul(a, b, current, core_num);
    register_file[params[2]] = (unsigned int)(a * b);
    message = "Multiply instruction executed, " +
                num_to_reg[params[2]] + " = " + num_to_reg[params[0]] + " * " + num_to_reg[params[1]] + " = " +
                std::to_string((int)register_file[params[2]]);
    instruction_count[mul]++;
}

void Core::executeSlt(std::vector<unsigned int> &params)
{
    register_file[params[2]] = register_file[params[0]] < register_file[params[1]];
    message = "Set on less than instruction executed, " +
                num_to_reg[params[2]] + " = " + num_to_reg[params[0]] + " < " + num_to_reg[params[1]] + " = " +
                (register_file[params[0]] < register_file[params[1]] ? "1" : "0");
    instruction_count[slt]++;
}

void Core::executeBeq(std::vector<unsigned int> &params)
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
    instruction_count[beq]++;
}

void Core::executeBne(std::vector<unsigned int> &params)
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
    instruction_count[bne]++;
}

void Core::executeLw(std::vector<unsigned int> &params)
{
    params[1] = params[1] << 16;
    int params1 = (int)params[1];
    params1 = params1 >> 16;
    int address = (params1 + (int)register_file[params[2]]);
    if (address >= pow(2, 20) || address / 4 < (int)endCommand)
    {
        throwRunTimeError("Invalid memory address", current);
    }
    if (address % 4 != 0)
    {
        throwRunTimeError("Invalid memory address: should be aligned with 4", current);
    }
    message = "Load word instruction generated for DRAM from memory address " +
                std::to_string(address) +
                " to the register " + num_to_reg[params[0]];
    instruction_count[lw]++;
}

void Core::executeSw(std::vector<unsigned int> &params)
{
    params[1] = params[1] << 16;
    int params1 = (int)params[1];
    params1 = params1 >> 16;
    int address = (params1 + (int)register_file[params[2]]);

    if (address >= pow(2, 20) || address / 4 < (int)endCommand)
    {
        throwRunTimeError("Invalid memory address", current);
    }
    if (address % 4 != 0)
    {
        throwRunTimeError("Invalid memory address: should be aligned with 4", current);
    }
    message = "Save word instruction generated for DRAM to memory address " +
                std::to_string(address) +
                " from the register " + num_to_reg[params[0]];
    instruction_count[sw]++;
}

// Funciton to execute the commands
void Core::executeCommand(InstructionType i_type, std::vector<unsigned int> &params)
{
    switch (i_type)
    {
        case jump: executeJ(params); break;
        case addi: executeAddi(params); break;
        case add: executeAdd(params); break;
        case sub: executeSub(params); break;
        case mul: executeMul(params); break;
        case slt: executeSlt(params); break;
        case beq: executeBeq(params); break;
        case bne: executeBne(params); break;
        case lw: executeLw(params); break;
        case sw: executeSw(params); break;
        default: break;
    }
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
    if (current < endCommand)
    {
        // end
        unsigned int instr = instruction_memory[current];
        unsigned int opcode = instr >> 26;

        InstructionType i_type = getInstructionType(opcode, current, core_num);

        std::vector<unsigned int> params = getParams(i_type, instr, core_num);
        executeCommand(i_type, params);

        register_file[0] = (unsigned int)0;

        return true;
    }
    else
    {
        return false;
    }
}

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

    std::string s;

    while (std::getline(instream, s))
    {
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