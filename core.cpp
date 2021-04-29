#include "a5.hpp"

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
    if (writeBusy)
    {
        current--;
        return;
    }
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
    if (writeBusy)
    {
        current--;
        return;
    }
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
    if (writeBusy)
    {
        current--;
        return;
    }
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
    if (writeBusy)
    {
        current--;
        return;
    }
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
    if (writeBusy)
    {
        current--;
        return;
    }
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
        throwRunTimeError("Branch not defined: " + label, current);
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
        throwRunTimeError("Branch not defined: " + label, current);
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
    DRAM::addRequest(new Request(this, true, address, params[0]));
    pendingRequests++;
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
    DRAM::addRequest(new Request(this, false, address, -1));
    pendingRequests++;
    message = "Save word instruction generated for DRAM to memory address " +
                std::to_string(address) +
                " from the register " + num_to_reg[params[0]];
    instruction_count[sw]++;
}

// Funciton to execute the commands
void Core::executeCommand(InstructionType i_type, std::vector<unsigned int> &params)
{
    if (pendingRequests > 0)
    {
        return;
    }
    current++;
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

void Core::printData()
{
    
}
