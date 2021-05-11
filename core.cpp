#include "a5.hpp"

// Function to execute jump commans
void Core::executeJ(std::vector<unsigned int> &params)
{
    // Get the label in string format from the params
    std::string label = labels[(int)params[0]];

    // If branch not found then throw runtime error
    if (branches.find(label) == branches.end())
    {
        throwRunTimeError("Address not found: " + label, current);
        return;
    }

    // Set the program counter to the branch
    current = branches[label];

    // Set the message
    message = "Jumped to " + label;
    
    // Increase the instruction count of jump
    instruction_count[jump]++;
}

// Function to execute addi instruction
void Core::executeAddi(std::vector<unsigned int> &params)
{
    // Pre conditions to check the possibility of execution of the instruction
    if (writeBusy)
    {// If write is busy then the instruction cannot be performed, so stall
        current--;
        return;
    }
    else if (loadQu[params[0]] != nullptr)
    {// If the register to be loaded is in DRAM load qu then stall and assert the waiting signal of the corresponding register
        waitReg[params[0]] = true;
        current--;
        return;
    }

    // If the register to be written has a request in loadQu then delete the request
    if (loadQu[params[1]] != nullptr)
    {
        loadQu[params[1]] = nullptr;
        pendingRequests--;
    }

    // The number to be added
    unsigned int giv = params[2];
    int num = (int)(giv << 16);
    num = num >> 16;

    // Value of register file params[0]
    int a = (int) register_file[params[0]];

    // Check the overflow
    detectOFadd(a, num, current, core_num);
    
    // Set the value of the distination register
    register_file[params[1]] = (unsigned int)(a + num);

    // Set the message
    message = "Add immediate instruction executed, " +
                num_to_reg[params[1]] + " = " + num_to_reg[params[0]] + " + " + std::to_string(num) +
                " = " + std::to_string((int)register_file[params[1]]);

    // Increase addi instruction count
    instruction_count[addi]++;

    waitReg[params[0]] = false;
}

// *********************************************************
// ****************** R-Type instructions ******************
// *********************************************************
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

    int a = (int) register_file[params[0]];
    int b = (int) register_file[params[1]];
    detectOFadd(a, b, current, core_num);
    register_file[params[2]] = (unsigned int)(a + b);
    message = "Add instruction executed, " +
                num_to_reg[params[2]] + " = " + num_to_reg[params[0]] + " + " + num_to_reg[params[1]] + " = " +
                std::to_string((int)register_file[params[2]]);
    instruction_count[add]++;

    waitReg[params[0]] = waitReg[params[1]] = false;
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

    int a = (int) register_file[params[0]];
    int b = (int) register_file[params[1]];
    detectOFsub(a, b, current, core_num);
    register_file[params[2]] = (unsigned int)(a - b);
    message = "Subtract instruction executed, " +
                num_to_reg[params[2]] + " = " + num_to_reg[params[0]] + " - " + num_to_reg[params[1]] + " = " +
                std::to_string((int)register_file[params[2]]);
    instruction_count[sub]++;

    waitReg[params[0]] = waitReg[params[1]] = false;
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

    int a = (int) register_file[params[0]];
    int b = (int) register_file[params[1]];
    detectOFmul(a, b, current, core_num);
    register_file[params[2]] = (unsigned int)(a * b);
    message = "Multiply instruction executed, " +
                num_to_reg[params[2]] + " = " + num_to_reg[params[0]] + " * " + num_to_reg[params[1]] + " = " +
                std::to_string((int)register_file[params[2]]);
    instruction_count[mul]++;

    waitReg[params[0]] = waitReg[params[1]] = false;
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

    register_file[params[2]] = ((int) register_file[params[0]]) <  ((int) register_file[params[1]]);
    message = "Set on less than instruction executed, " +
                num_to_reg[params[2]] + " = " + num_to_reg[params[0]] + " < " + num_to_reg[params[1]] + " = " + 
                std::to_string(register_file[params[2]]);
    instruction_count[slt]++;

    waitReg[params[0]] = waitReg[params[1]] = false;
}

// ****************** Branch instructions ******************
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

    waitReg[params[0]] = waitReg[params[1]] = false;
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

    waitReg[params[0]] = waitReg[params[1]] = false;
}

// Load word instruction
void Core::executeLw(std::vector<unsigned int> &params)
{
    // Value that will be forwarded from saveQu to the load register
    static unsigned int forward_value = -1;

    // True if forwarding possible
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

    // If the params[2] in loadQu then wait
    if (loadQu[params[2]] != nullptr)
    {
        waitReg[params[2]] = true;
        waitMem = address;
        current--;
        return;
    }

    // Forwarding
    if (saveQu[address % saveQuBufferLength] != nullptr)
    {// If memory to be loaded from is to be written

        // Check if the request to be written on the same address and the write is not busy
        if (saveQu[address % saveQuBufferLength]->address == address && !writeBusy)
        {
            // Write the value directly from the wait buffer
            register_file[params[0]] = saveQu[address % saveQuBufferLength]->reg;

            // Set message
            message = "Forwarded save instruction at address " + std::to_string(address) + " to load the value in register " + num_to_reg[params[0]];
            
            // Increase instruction count
            instruction_count[lw]++;
            return;
        }
        else if (saveQu[address % saveQuBufferLength]->address == address && writeBusy)
        {// If write is busy and at the same address, then write in next cycle
            
            // Set the forward value and and forward signal
            forward_value = saveQu[address % saveQuBufferLength]->reg;
            forward = true;
            current--;
            return;
        }
    }
    else if (forward)
    {// If to be forward then forward
        register_file[params[0]] = forward_value;
        message = "Forwarded save instruction at address " + std::to_string(address) + " to load the value in register " + num_to_reg[params[0]];
        instruction_count[lw]++;
        
        // Reset forward signals
        forward_value = -1;
        forward = false;
        return;
    }

    // Request generation
    addRequest(new Request(true, address, params[0]));
    pendingRequests++;
    message = "Load word instruction generated for DRAM from memory address " +
                std::to_string(address - base_address) +
                " to the register " + num_to_reg[params[0]];

    waitReg[params[2]] = false;
}

void Core::executeSw(std::vector<unsigned int> &params)
{
    // Address calculation and validation
    params[1] = params[1] << 16;
    int params1 = (int)params[1];
    params1 = params1 >> 16;
    int address = (params1 + (int)register_file[params[2]]) + base_address;
    if (address % 4 != 0)
    {
        throwRunTimeError("Invalid memory address: should be aligned with 4", current);
    }

    // Validation
    if (saveQu[address % saveQuBufferLength] != nullptr)
    {// If same address point busy then check
        if (saveQu[address % saveQuBufferLength]->address != address)
        {// If different address busy then stall
            current--;
            return;
        }
    }
    
    // If any of the input register is busy then stall
    if (loadQu[params[0]] != nullptr || loadQu[params[2]] != nullptr)
    {
        waitReg[params[0]] = waitReg[params[2]] = true;
        current--;
        return;
    }

    // Request generation
    addRequest(new Request(false, address, register_file[params[0]]));
    pendingRequests++;
    message = "Save word instruction generated for DRAM to memory address " +
                std::to_string(address - base_address) +
                " from the register " + num_to_reg[params[0]];

    waitReg[params[0]] = waitReg[params[2]] = false;
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
    printRegisterFile(register_file, core_num);

    std::cout << "Number of times each instruction is executed: " << std::endl << std::endl;
    for (std::pair<InstructionType, long long int> p: instruction_count){
        std::cout << instruction_type_string[(int) p.first] << "\t" << p.second << std::endl;
    }
}

void Core::addRequest(Request *req)
{
    if (req->load)
    {
        if (loadQu[req->reg] != nullptr)
        {// Already request present then overwrite
            
            // Decrease number of pending request
            pendingRequests--;

            // Clean the heap space (Good practice)
            delete loadQu[req->reg];
        }
        loadQu[req->reg] = req;
    }
    else
    {
        if (saveQu[req->address % saveQuBufferLength] != nullptr)
        {// Already request present then overwrite
            
            // Decrease number of pending request
            pendingRequests--;

            // Clean the heap space (Good practice)
            delete saveQu[req->address % saveQuBufferLength];
        }
        saveQu[req->address % saveQuBufferLength] = req;
    }

}

void Core::setBestRequest()
{
    int loadBest = -1;
    int saveBest = -1;
    
    int loadNorm = -1;
    int saveNorm = -1;

    // Load best
    for (int i = 0; i < 32; i++)
    {
        if (loadQu[i] == nullptr)
        {
            continue;
        }
        loadNorm = i;
        if (loadQu[i]->row == DRAM::activeRow && waitReg[i])
        {
            loadBest = i;
            break;
        }
        else if (loadQu[i]->row == DRAM::activeRow)
        {
            loadBest = i;
        }
        else if (waitReg[i] && loadBest == -1)
        {
            loadBest = i;
        }
    }

    // Save best
    for (int i = 0; i < Core::saveQuBufferLength; i++)
    {
        if (saveQu[i] == nullptr)
        {
            continue;
        }
        saveNorm = i;
        if (saveQu[i]->row == DRAM::activeRow && waitMem == saveQu[i]->address)
        {
            saveBest = i;
            break;
        }
        else if (saveQu[i]->row == DRAM::activeRow)
        {
            saveBest = i;
        }
        else if (waitMem == saveQu[i]->address && saveBest == -1)
        {
            saveBest = i;
        }
    }

    if (loadBest == -1 && saveBest == -1)
    {
        if(loadNorm != -1)
        {
            bestReq[0] = 1;
            bestReq[1] = loadNorm;
        }
        else if (saveNorm != -1)
        {
            bestReq[0] = 0;
            bestReq[1] = saveNorm;
        } 
        else
        {
            bestReq[0] = bestReq[1] = -1;
        }
    }
    else if (loadBest == -1)
    {
        bestReq[0] = 0;
        bestReq[1] = saveBest;
    }
    else if (saveBest == -1)
    {
        bestReq[0] = 1;
        bestReq[1] = loadBest;
    }
    else
    {
        Request *l = loadQu[loadBest];
        Request *s = saveQu[saveBest];

        if (l->row == DRAM::activeRow && waitReg[l->reg])
        {
            bestReq[0] = 1;
            bestReq[1] = loadBest;
        }
        else if (s->row == DRAM::activeRow && waitMem == s->address)
        {
            bestReq[0] = 0;
            bestReq[1] = saveBest;
        }
        else if (l->row == DRAM::activeRow)
        {
            bestReq[0] = 1;
            bestReq[1] = loadBest;
        }
        else if (s->row == DRAM::activeRow)
        {
            bestReq[0] = 0;
            bestReq[1] = saveBest;
        }
        else if (waitReg[l->reg])
        {
            bestReq[0] = 1;
            bestReq[1] = loadBest;
        }
        else if (waitMem == s->address)
        {
            bestReq[0] = 0;
            bestReq[1] = saveBest;
        }
        else
        {
            bestReq[0] = bestReq[1] = -1;
        }
    }
}

Request* Core::getNextRequest()
{
    if (bestReq[0] = -1)
    {
        if (pendingRequests > 0)
        {
            std::cout << "f" << std::endl;
        }
        return nullptr;
    }
    else
    {
        if (bestReq[0] == 1)
        {
            return loadQu[bestReq[1]];
        }
        else
        {
            return saveQu[bestReq[1]];
        }
    }
}
