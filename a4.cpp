#include "a4.hpp"

std::vector<DRAM*> dram_qu;
int busy[32];
std::set<int> waiting;
std::set<unsigned int> busy_mem;

bool optimize;

DRAM* cur = nullptr;

long long row_updates_count = 0;

AccessType::AccessType(std::string message, int param, int cycles){
    // Row access from row 
    // 5
    // 10
    this->message = message;
    this->param = param;
    this->cycles = cycles;
}

AccessType::~AccessType(){

}

// true for load
// false for store

DRAM::DRAM(bool type, unsigned int param, unsigned int address){
    this->type = type;
    this->param = param;
    this->address = address;
    this->row_num = address / 1024;
    this->col_num = address % 1024;
}

DRAM::~DRAM(){

}

void DRAM::next(){
    std::cout << "DRAM request: " << 
        (type ? 
            ("Load word from address " + std::to_string(address) + " to register " + std::to_string(param)) : 
             "Save word " + std::to_string(param) + " to address " + std::to_string(address)) 
        << ": " << qu.front().message << qu.front().param << std::endl;
    qu.front().cycles--;
    if (qu.front().cycles == 0){
        qu.erase(qu.begin());
    }
}

int DRAM::ROW_ACCESS_DELAY = 10;
int DRAM::COL_ACCESS_DELAY = 2;
int DRAM::active_row = -1;
bool DRAM::buffer_updated = 0;

void printData(long long cycle_num, std::string message){
    
    std::cout << std::endl << "Cycle " << cycle_num+1 << ": " << std::endl;

    if (message != ""){
        std::cout << message << std::endl;
    }

}

void configQueue(DRAM* req){
    bool deleted = dram_qu.empty() && (cur == nullptr);

    if (cur != nullptr){
        cur->next();
        if (cur->qu.empty()){
            if (cur->type){
               busy[cur->param]--;
            }
            
            if (cur->type){
                register_file[cur->param] = memory[cur->address / 4];
            }else{
                if (memory[cur->address / 4] != cur->param){
                    DRAM::buffer_updated = 1;
                }
                memory[cur->address / 4] = cur->param;
                busy_mem.erase(cur->address);
            }
            delete cur;
            deleted = true;
            cur = nullptr;
        }
    }

    if (req != nullptr){

        if(optimize){
            for (int i = 0; i < dram_qu.size(); ){
                if (!dram_qu[i]->type){
                    if (dram_qu[i]->address == req->address){
                        dram_qu.erase(dram_qu.begin()+i);
                    }else{
                        i++;
                    }
                }else{
                    i++;
                }
            }
        }

        dram_qu.push_back(req);
    }

    if (optimize){
        int curr_row = DRAM::active_row;

        int i = 0;
        int j = 0;

        while (j < dram_qu.size()){
            if (dram_qu[j]->row_num == curr_row) {
                std::swap(dram_qu[i], dram_qu[j]);
                i++;
                j++;
            }else {
                j++;
            }
        }

        j = 0;

        if (i == 0){
            i = dram_qu.size();
        }
        
        while (j < i){
            if (dram_qu[j]->type && waiting.find(dram_qu[j]->param) != waiting.end()) {
                std::swap(dram_qu[0], dram_qu[j]);
                break;
            }else{
                j++;
            }
        }
    }


    if (deleted && !dram_qu.empty()){
        cur = dram_qu[0];
        dram_qu.erase(dram_qu.begin());
        if (cur->row_num != DRAM::active_row){
            if (DRAM::buffer_updated){
                cur->qu.push_back(AccessType("Row buffer writeback: Row ", DRAM::active_row, DRAM::ROW_ACCESS_DELAY));
                // row_updates_count++;
            }
            DRAM::active_row = cur->row_num;
            cur->qu.push_back(AccessType("Row buffer updated from: Row ", cur->row_num, DRAM::ROW_ACCESS_DELAY));
            row_updates_count++;       
        }
        cur->qu.push_back(AccessType("Column access at column ", cur->col_num, DRAM::COL_ACCESS_DELAY));
        DRAM::buffer_updated = 0;
    }

    std::cout << std::endl;

}

void delete_redundant(unsigned int a){

    if (!optimize){
        return;
    }

    int i = 0;
    while (i < dram_qu.size()){
        if (dram_qu[i]->type && dram_qu[i]->param == a){
            dram_qu.erase(dram_qu.begin()+i);
            busy[a] = 0;
        }else{
            i++;
        }
    }

}

/*

-------
lw $t0, addr
addi $t0, $zero, 5

*/