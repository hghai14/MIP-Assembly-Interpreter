#include "a5.hpp"

int DRAM::ROW_ACCESS_DELAY = -1;
int DRAM::COL_ACCESS_DELAY = -1;

int DRAM::cur = 0;
bool DRAM::busy = false;

std::string DRAM::message = "";
int DRAM::writeLeft = 0;
int DRAM::rowLeft = 0;
int DRAM::colLeft = 0;
int DRAM::mrmWaitLeft = 0;
int DRAM::pendingTotal = 0;

Request* DRAM::quBuf[64];
Request* DRAM::activeRequest;

Request::Request(Core* core, bool load, unsigned int address, int reg)
{
    this->core = core;
    this->load = load;
    this->address = address;
    this->reg = reg;
}

bool DRAM::execute()
{
    if (mrmWaitLeft > 0)
    {
        mrmWaitLeft--;
        message = "Memory request manager working";
        return false;
    }

    if (!busy)
    {
        activeRequest = getNextRequest();
        if (activeRequest != nullptr)
        {
            writeLeft = 0;
            rowLeft = 10;
            colLeft = 2;
            busy = true;
        }
        if (pendingTotal > 0)
        {
            message = "Memory request manager started working";
        }
        else
        {
            message = "Memory request manager sacnned buffer";
        }
        return false;
    }

    if (writeLeft > 0)
    {
        writeLeft--;
        message = "Writing back";
    }
    else if (rowLeft > 0)
    {
        rowLeft--;
        message = "Loading row to buffer";
    }
    else if (colLeft > 0)
    {
        colLeft--;
        message = "Column access";
    }
    else
    {
        message = "";
    }

    if (colLeft + rowLeft + writeLeft == 1)
    {
        activeRequest->core->writeBusy = true;
    }

    if (colLeft + rowLeft + writeLeft  == 0)
    {
        activeRequest->core->pendingRequests--;
        activeRequest->core->writeBusy = false;
        busy = false;
    }

    return false;

}

Request* DRAM::getNextRequest()
{

    if (pendingTotal == 0)
    {
        mrmWaitLeft = 0;
        return nullptr;
    }

    mrmWaitLeft = 5;

    for (int i = 0; i < 64; i++)
    {
        if (quBuf[i] != nullptr)
        {
            cur = i;
            Request* temp = quBuf[i];
            quBuf[i] = nullptr;
            pendingTotal--;
            return temp;
        }
    }

    return nullptr;
}

bool DRAM::addRequest(Request *req)
{
    if (cur == -1)
    {
        return false;
    }

    quBuf[cur] = req;

    pendingTotal++;

    cur = -1;

    for (int i = 0; i < 64; i++)
    {
        if (quBuf[i] == nullptr)
        {
            cur = i;
            break;
        }
    }

    return false;
}
