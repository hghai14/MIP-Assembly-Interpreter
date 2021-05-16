#include "a5.hpp"

DRAM_Req DRAM_Req::null = DRAM_Req();
Request Request::null = Request();

// Row and column access delay
int DRAM::ROW_ACCESS_DELAY = -1;
int DRAM::COL_ACCESS_DELAY = -1;

std::vector<Core *> DRAM::swap_queue;

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
                    " from address " + std::to_string(activeRequest.req.address - activeRequest.core->base_address);
    }
    else
    {
        message = "Save request to adrress " + std::to_string(activeRequest.req.address - activeRequest.core->base_address) + " of value " + 
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
        activeRequest.req.busy = false;

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

        // Set the busy to false
        busy = false;
    }
}

bool DRAM::execute()
{
    message = "Free";

    bool f = false;

    // std::cout << (tempRequest != DRAM_Req::null) << std::endl;
    
    if (!busy && tempRequest != DRAM_Req::null)
    {
        f = true;

        bool same = activeRequest.core == tempRequest.core;
        
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

            if (same)
            {
                c->row_miss++;

                int i = 0;
                while (swap_queue[i] != c)
                {
                    i++;
                }

                int swaps = 0;

                while (i < swap_queue.size()-1)
                {
                    if (swap_queue[i]->row_miss > swap_queue[i+1]->row_miss)
                    {
                        std::swap(swap_queue[i], swap_queue[i+1]);
                        i++;
                        swaps++;
                    }
                    else
                    {
                        break;
                    }
                }
            }
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
    // std::cout << tempRequest.req.load << " " << tempRequest.req.reg << " " << tempRequest.req.valid << std::endl;

    if (tempRequest != DRAM_Req::null)
    {
        f = true;
    }

    return f;
}

DRAM_Req DRAM::getNextRequest()
{
    Request best = Request::null;
    Core *best_c = nullptr;
    bool waitReg = true;
    bool sameRow = true;

    Request row_miss_request = Request::null;
    Core *row_miss_c = nullptr;
    int min_row_miss = INT32_MAX;

    for (int i = 0; i < swap_queue.size(); i++)
    {
        Request r = swap_queue[i]->getNextRequest();

        if (!r.valid)
        {
            continue;
        }

        if (swap_queue[i]->row_miss < min_row_miss)
        {
            row_miss_request = r;
            min_row_miss = swap_queue[i]->row_miss;
            row_miss_c = swap_queue[i];
        }
    }

    for (Core *c : cores)
    {
        Request r = c->getNextRequest();
        
        c->setBestRequest();
        
        if (!r.valid)
        {
            continue;
        }

        if (r.row == activeRow && c->waitReg[r.reg])
        {
            best = r;
            best_c = c;
            break;
        }
        else if (r.row == activeRow)
        {
            best = r;
            best_c = c;
            sameRow = false;
            waitReg = false;
        }
        else if (c->waitReg[r.reg] && sameRow)
        {
            best = r;
            best_c = c;
            waitReg = false;
        }
        else if (waitReg)
        {
            best = r;
            best_c = c;
        }
    }

    if (waitReg)
    {
        best = row_miss_request;
        best_c = row_miss_c;
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
