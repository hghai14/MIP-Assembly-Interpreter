#include "a5.hpp"

DRAM_Req DRAM_Req::null = DRAM_Req();
Request Request::null = Request();

// Row and column access delay
int DRAM::ROW_ACCESS_DELAY = -1;
int DRAM::COL_ACCESS_DELAY = -1;

// Active row of DRAM
int DRAM::activeRow = -1;

// Boolean variable to check if the DRAM is busy
bool DRAM::busy = false;

// DRAM message
std::string DRAM::message = "";

// Number of cycles left for following instructions to complete
int DRAM::writeLeft = 0;
int DRAM::rowLeft = 0;
int DRAM::colLeft = 0;

// Memory request manager cycles left
int DRAM::mrmWaitLeft = 0;

// Active request at DRAM
DRAM_Req DRAM::activeRequest = DRAM_Req::null;
DRAM_Req DRAM::tempRequest = DRAM_Req::null;

// Request constructor
Request::Request(bool load, unsigned int address, unsigned int reg)
{
    // 1 if the request is load request
    this->load = load;

    // Memory address of the request
    this->address = address;

    // Register number in case of load and value in case of store
    this->reg = reg;

    // Row accessed by the request
    this->row = address / 1024;
}

// DRAM process of execution of request
void DRAM::process()
{
    // Detailed message
    if (activeRequest.req.load)
    {
        message = "Load request to register " + Core::num_to_reg[activeRequest.req.reg] + 
                    " from address " + std::to_string(activeRequest.req.address);
    }
    else
    {
        message = "Save request to adrress " + std::to_string(activeRequest.req.address) + " of value " + 
                    std::to_string(activeRequest.req.reg);
    }

    message += ": ";

    if (writeLeft > 0)
    { // If number of writeback left
        writeLeft--;
        message += "Row " + std::to_string(activeRow) + " writeback";
    }
    else if (rowLeft > 0)
    { // If number of row access left
        activeRow = activeRequest.req.row;
        rowLeft--;
        message += "Row " + std::to_string(activeRow) + " access";
    }
    else if (colLeft > 0)
    { // If number of column access left
        colLeft--;
        message += "Column " + std::to_string(activeRequest.req.address % 1024) + " access";
    }

    message += " --- Core " + std::to_string(activeRequest.core->core_num);

    if (colLeft == 0)
    { // Request if completed

        // Perform request
        Request r = activeRequest.req;
        Core *c = activeRequest.core;

        if (r.load)
        {
            c->register_file[r.reg] = memory[r.address / 4];
            c->instruction_count[lw]++;
        }
        else
        {
            memory[r.address / 4] = r.reg;
            c->instruction_count[sw]++;
        }

        c->pendingRequests--;
        activeRequest.core->writeBusy = true;

        // Set the busy to false
        busy = false;
    }
}

bool DRAM::execute()
{
    message = "Free";

    bool f = false;

    if (!busy && activeRequest != DRAM_Req::null)
    {
        Request r = activeRequest.req;
        Core *c= activeRequest.core;

        c->writeBusy = false;
        c->busyMem = -1;

        if (r.load && !c->loadQu[r.reg].valid && c->loadQu[r.reg].busy &&
            c->loadQu[r.reg].address == r.address)
        {
            c->loadQu[r.reg].busy = false;
        }
        else if (!r.load && !c->saveQu[r.address % Core::saveQuBufferLength].valid &&
                c->saveQu[r.address % Core::saveQuBufferLength].busy &&
                c->saveQu[r.address % Core::saveQuBufferLength].address == r.address && 
                c->saveQu[r.address % Core::saveQuBufferLength].reg == r.reg)
        {
            c->saveQu[r.address % Core::saveQuBufferLength].busy = false;
        }
    }

    // std::cout << (tempRequest != DRAM_Req::null) << std::endl;
    
    if (!busy && tempRequest != DRAM_Req::null)
    {
        f = true;
        
        activeRequest = tempRequest;

        Request r = activeRequest.req;
        Core *c= activeRequest.core;

        writeLeft = rowLeft = 0;

        if (r.load && c->loadQu[r.reg].valid&&
            c->loadQu[r.reg].address == r.address)
        {
            c->loadQu[r.reg].valid = false;
        }
        else if (!r.load && c->saveQu[r.address % Core::saveQuBufferLength].valid &&
                c->saveQu[r.address % Core::saveQuBufferLength].address == r.address && 
                c->saveQu[r.address % Core::saveQuBufferLength].reg == r.reg)
        {
            c->saveQu[r.address % Core::saveQuBufferLength].valid = false;
            c->busyMem = r.address;
        }

        // Set the wait cycles left
        if (activeRequest.req.row != activeRow && activeRow != -1)
        {
            writeLeft = rowLeft = ROW_ACCESS_DELAY;
        }
        else if (activeRow == -1)
        {
            rowLeft = ROW_ACCESS_DELAY;
        }

        colLeft = COL_ACCESS_DELAY;

        // Set busy true
        busy = true;
    }
    
    if (busy)
    {
        f = true;
        process();
    }

    tempRequest = getNextRequest();

    if (tempRequest != DRAM_Req::null)
    {
        f = true;
    }

    // std::cout << tempRequest.req.load << " " << tempRequest.req.reg << " " << tempRequest.req.valid << std::endl;

    return f;
}

DRAM_Req DRAM::getNextRequest()
{
    Request best = Request::null;
    Core *best_c = nullptr;
    bool waitReg = true;
    bool waitMem = true;

    for (Core *c : cores)
    {
        Request r = c->getNextRequest();
        if (!r.valid)
        {
            continue;
        }

        if (r.row == activeRow)
        {
            best = r;
            best_c = c;
            break;
        }
        else if (c->waitReg[r.reg])
        {
            best = r;
            best_c = c;
            waitReg = false;
        }
        else if (c->waitMem == r.address && waitReg)
        {
            best = r;
            best_c = c;
            waitMem = false;
        }
        else if (waitMem && waitReg)
        {
            best = r;
            best_c = c;
        }
    }

    if (!best.valid)
    {
        return DRAM_Req::null;
    }
    else
    {
        DRAM_Req res;
        res.core = best_c;
        res.req = best;
        return res;
    }

}
