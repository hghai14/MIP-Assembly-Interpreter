#ifndef HELPER
#define HELPER

#include "main.hpp"

std::string decToHex(unsigned int n);

void printRegisterFile(unsigned int register_file[], unsigned int core_num);
void throwRunTimeError(const std::string &message, unsigned int current, unsigned int core_num);
void throwRunTimeError(const std::string &message, unsigned int core_num);
void throwError(const std::string &message, int &lineNum, int &column, unsigned int core_num);
void throwError(const std::string &message, int &lineNum, unsigned int core_num);
void printOverFlowMessage(std::string &s, unsigned int &current, unsigned int core_num);

void detectOFadd(int a, int b, unsigned int current, unsigned int core_num);
void detectOFmul(int a, int b, unsigned int current, unsigned int core_num);
void detectOFsub(int a, int b, unsigned int current, unsigned int core_num);

InstructionType getInstructionType(unsigned int opcode, unsigned int &current, unsigned int core_num);

bool isSpace(const char &c);
bool isAlpha(const char &c);
bool isNum(const char &c);
void toNextNonSpace(std::string &s, int &i);

std::vector<std::string> getTokens(std::string &s, int &i, int &lineNum);
std::vector<std::string> getLoadSaveTokens(std::string &s, int &i, int &lineNum, unsigned int core_num);


#endif