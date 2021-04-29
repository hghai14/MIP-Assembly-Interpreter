#include "helper.hpp"

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
void printRegisterFile(unsigned int register_file[], unsigned int core_num)
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
    exit(-1);
}

void throwRunTimeError(const std::string &message, unsigned int core_num)
{
    std::cerr << "Runtime error occured in core " << core_num << std::endl;
    std::cerr << message << std::endl;
    exit(-1);
}

// Functions to throw compilation error
void throwError(const std::string &message, int &lineNum, int &column, unsigned int core_num)
{
    std::cerr << "Syntax error: At line " << lineNum << ", column " << column + 1 << " in core " << core_num << std::endl;
    std::cerr << message << std::endl;
    exit(-1);
}

void throwError(const std::string &message, int &lineNum, unsigned int core_num)
{
    std::cerr << "Syntax error: At line " << lineNum << " in core " << core_num << std::endl;
    std::cerr << message << std::endl;
    exit(-1);
}

void printOverFlowMessage(std::string &s, unsigned int &current, unsigned int core_num)
{
    std::cerr << s << std::endl;
    std::cerr << "At instruction " << current << " in core " << core_num << std::endl;
}

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
InstructionType getInstructionType(unsigned int opcode, unsigned int &current, unsigned int core_num)
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