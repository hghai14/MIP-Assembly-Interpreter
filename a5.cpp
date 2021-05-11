#include "a5.hpp"

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
DRAM_Req *DRAM::activeRequest;

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
    if (activeRequest->req->load)
    {
        message = "Load request to register " + Core::num_to_reg[activeRequest->req->reg] + 
                    " from address " + std::to_string(activeRequest->req->address);
    }
    else
    {
        message = "Save request to adrress " + std::to_string(activeRequest->req->address) + " of value " + 
                    std::to_string(activeRequest->req->reg) + 
                    " from register " + Core::num_to_reg[activeRequest->req->reg];
    }

    message += ": ";

    if (writeLeft > 0)
    { // If number of writeback left
        writeLeft--;
        message += "Row " + std::to_string(activeRow) + " writeback";
    }
    else if (rowLeft > 0)
    { // If number of row access left
        activeRow = activeRequest->req->row;
        rowLeft--;
        message += "Row " + std::to_string(activeRow) + " access";
    }
    else if (colLeft > 0)
    { // If number of column access left
        colLeft--;
        message += "Column " + std::to_string(activeRequest->req->address % 1024) + " access";
    }

    message += " --- Core " + std::to_string(activeRequest->core->core_num);

    if (rowLeft + writeLeft + colLeft == 1)
    {
        activeRequest->core->writeBusy = true;
    }

    if (colLeft == 0)
    { // Request if completed

        // Perform request
        Request *r = activeRequest->req;
        Core *c = activeRequest->core;

        if (r->load)
        {
            c->register_file[r->reg] = memory[r->address / 4];
            c->loadQu[r->reg] = nullptr;
        }
        else
        {
            memory[r->address / 4] = r->reg;
            c->saveQu[r->address % Core::saveQuBufferLength] = nullptr;
        }

        delete r;
        delete activeRequest;

        c->writeBusy = false;
        c->pendingRequests--;
        c->waitMem = -1;

        // Set the busy to false
        busy = false;
    }
}

bool DRAM::execute()
{
    message = "Free";
    if (!busy)
    {
        // Get the next best request
        activeRequest = getNextRequest();

        if (activeRequest == nullptr)
        {
            return false;
        }

        writeLeft = rowLeft = 0;

        // Set the wait cycles left
        if (activeRequest->req->row != activeRow && activeRow != -1)
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

    process();

    return true;
}

Request *getBestReq(Core *c, int *bestReq)
{
    if (bestReq[0] == 0)
    {
        return c->saveQu[bestReq[1]];
    }
    else if (bestReq[0] == 1)
    {
        return c->loadQu[bestReq[1]];
    }
    else
    {
        return nullptr;
    }
}

DRAM_Req *DRAM::getNextRequest()
{
    Request *best = nullptr;
    Core *best_c = nullptr;
    bool waitReg = true;
    bool waitMem = true;

    int total = 0;

    for (Core *c : cores)
    {
        total += c->pendingRequests;
        Request *r = getBestReq(c, c->bestReq);
        if (r == nullptr)
        {
            continue;
        }

        if (r->row == activeRow)
        {
            best = r;
            best_c = c;
            break;
        }
        else if (c->waitReg[r->reg])
        {
            best = r;
            best_c = c;
            waitReg = false;
        }
        else if (c->waitMem == r->address && waitReg)
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

    if (best == nullptr)
    {
        return nullptr;
    }
    else{
        DRAM_Req *res = new DRAM_Req();
        res->req = best;
        res->core = best_c;
        return res;
    }

}
