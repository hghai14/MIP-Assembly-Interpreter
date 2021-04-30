#include "a5.hpp"

int DRAM::ROW_ACCESS_DELAY = -1;
int DRAM::COL_ACCESS_DELAY = -1;

int DRAM::activeRow = -1;
bool DRAM::busy = false;

std::string DRAM::message = "";
int DRAM::writeLeft = 0;
int DRAM::rowLeft = 0;
int DRAM::colLeft = 0;
int DRAM::mrmWaitLeft = 0;

DRAM_Req* DRAM::activeRequest;

Request::Request(bool load, unsigned int address, unsigned int reg)
{
    this->load = load;
    this->address = address;
    this->reg = reg;

    this->row = address / 1024;

}

void DRAM::process()
{
    if (writeLeft > 0)
    {
        writeLeft--;
        message = "Writeback";
    }
    else if (rowLeft > 0)
    {
        rowLeft--;
        message = "Row access";
    }
    else if (colLeft > 0)
    {
        colLeft--;
        message = "Col access";
    }

    if (colLeft == 0)
    {
        busy = false;
    }

}

bool DRAM::execute()
{
    if (mrmWaitLeft > 0)
    {
        mrmWaitLeft--;
    }

    if (busy)
    {
        process();
        return false;
    }
}

DRAM_Req* DRAM::getNextRequest()
{

}

