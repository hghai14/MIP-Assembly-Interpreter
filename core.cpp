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
    else if (loadQu[params[0]] != nullptr)
    {
        waitReg[params[0]] = true;
        current--;
        return;
    }

    if (loadQu[params[1]] != nullptr)
    {
        loadQu[params[1]] = nullptr;
        pendingRequests--;
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
    else if (loadQu[params[0]] != nullptr || loadQu[params[1]] != nullptr)
    {
        waitReg[params[0]] = waitReg[params[1]] = true;
        current--;
        return;
    }

    if (loadQu[params[2]] != nullptr)
    {
        loadQu[params[2]] = nullptr;
        pendingRequests--;
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
    else if (loadQu[params[0]] != nullptr || loadQu[params[1]] != nullptr)
    {
        waitReg[params[0]] = waitReg[params[1]] = true;
        current--;
        return;
    }

    if (loadQu[params[2]] != nullptr)
    {
        loadQu[params[2]] = nullptr;
        pendingRequests--;
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
    else if (loadQu[params[0]] != nullptr || loadQu[params[1]] != nullptr)
    {
        waitReg[params[0]] = waitReg[params[1]] = true;
        current--;
        return;
    }

    if (loadQu[params[2]] != nullptr)
    {
        loadQu[params[2]] = nullptr;
        pendingRequests--;
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
    else if (loadQu[params[0]] != nullptr || loadQu[params[1]] != nullptr)
    {
        waitReg[params[0]] = waitReg[params[1]] = true;
        current--;
        return;
    }

    if (loadQu[params[2]] != nullptr)
    {
        loadQu[params[2]] = nullptr;
        pendingRequests--;
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

    if (loadQu[params[0]] != nullptr || loadQu[params[1]] != nullptr)
    {
        waitReg[params[0]] = waitReg[params[1]] = true;
        current--;
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

    if (loadQu[params[0]] != nullptr || loadQu[params[1]] != nullptr)
    {
        waitReg[params[0]] = waitReg[params[1]] = true;
        current--;
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
    static unsigned int forward_value = -1;
    static bool forward = false;
    // Address calculation and validation
    params[1] = params[1] << 16;
    int params1 = (int)params[1];
    params1 = params1 >> 16;
    int address = (params1 + (int)register_file[params[2]]) + base_address;
    if (address % 4 != 0)
    {
        throwRunTimeError("Invalid memory address: should be aligned with 4", current);
    }

    if (loadQu[params[2]] != nullptr)
    {
        waitReg[params[2]] = true;
        waitMem = address;
        current--;
        return;
    }

    // Forwarding
    if (saveQu[address % 64] != nullptr)
    {
        if (saveQu[address % 64]->address == address && !writeBusy)
        {
            register_file[params[0]] = saveQu[address % 64]->reg;
            message = "Forwarded save instruction at address " + std::to_string(address) + " to load the value in register " + num_to_reg[params[0]];
            instruction_count[lw]++;
            return;
        }
        else if (saveQu[address % 64]->address == address && writeBusy)
        {
            forward_value = saveQu[address % 64]->reg;
            forward = true;
        }
    }
    else if (forward)
    {
        register_file[params[0]] = forward_value;
        message = "Forwarded save instruction at address " + std::to_string(address) + " to load the value in register " + num_to_reg[params[0]];
        instruction_count[lw]++;
        forward_value = -1;
        forward = false;
    }
    else
    {
        waitMem = address;
        current--;
        return;
    }

    // Request generation
    addRequest(new Request(true, address, params[0]));
    pendingRequests++;
    message = "Load word instruction generated for DRAM from memory address " +
                std::to_string(address) +
                " to the register " + num_to_reg[params[0]];
    instruction_count[lw]++;
}

void Core::executeSw(std::vector<unsigned int> &params)
{
    // Address calculation anf validation
    params[1] = params[1] << 16;
    int params1 = (int)params[1];
    params1 = params1 >> 16;
    int address = (params1 + (int)register_file[params[2]]) + base_address;
    if (address % 4 != 0)
    {
        throwRunTimeError("Invalid memory address: should be aligned with 4", current);
    }

    // Validation
    if (saveQu[address % 64] != nullptr)
    {
        if (saveQu[address % 64]->address != address)
        {
            current--;
            return;
        }
    }
    
    if (loadQu[params[0]] != nullptr || loadQu[params[2]] != nullptr)
    {
        waitReg[params[0]] = waitReg[params[1]] = true;
        current--;
        return;
    }

    // Request generation
    addRequest(new Request(false, address, register_file[params[0]]));
    pendingRequests++;
    message = "Save word instruction generated for DRAM to memory address " +
                std::to_string(address) +
                " from the register " + num_to_reg[params[0]];
    instruction_count[sw]++;
}

// Funciton to execute the commands
void Core::executeCommand(InstructionType i_type, std::vector<unsigned int> &params)
{
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

void Core::addRequest(Request *req)
{
    if (req->load)
    {
        loadQu[req->reg] = req;
    }
    else
    {
        saveQu[req->address % 64] = req;
    }

}

void Core::setBestRequest()
{
    std::set<int*> row;
    std::set<int*> wait_reg;
    std::set<int*> wait_mem;
    // Row match
    // WaitReg match
    // WaitMem match
    
}

Request* Core::getNextRequest()
{
    if (bestReq[0] = -1)
    {
        return nullptr;
    }
    else
    {
        if (bestReq[0] == 1)
        {
            Request *temp = loadQu[bestReq[1]];
            loadQu[bestReq[1]] = nullptr;
            return temp;
        }
        else
        {
            Request *temp = saveQu[bestReq[1]];
            saveQu[bestReq[1]] = nullptr;
            return temp;
        }
    }
}
