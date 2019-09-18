#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <string>
#include <vector>
#define SYSERR      1
#define ARGCOUNT    9
using namespace std;

unsigned int call_count;
class Block {
    public:    
    bool is_dirty=false;
    bool is_valid=false;
    unsigned int tag_id;
    unsigned int timestamp;
};

class Set{
    public:
    Set(int associativity){
        for(int i=0;i<associativity;i++){
            blocks.push_back(Block());
        }
    }
    vector<Block> blocks;
};

class Cache {
    private:
    int block_size;
    int cache_size;        
    int assoc;
    Cache* next_level;
    vector<Set> sets;

    int num_reads           =   0;
    int num_writes          =   0;
    int num_read_miss       =   0;
    int num_write_miss      =   0;
    int num_write_backs     =   0;
    int num_memory_access   =   0;

    int number_of_sets      =   0;
    int number_of_blocks    =   0;

    public:
    Cache(int block_size, int cache_size, int associativity): 
        block_size(block_size), cache_size(cache_size), assoc(associativity){
            number_of_blocks=cache_size/block_size;
            number_of_sets=number_of_blocks/associativity;
            for(int i=0;i<number_of_sets;i++){
                sets.push_back(Set(associativity));
            }            
        }
    void DisplayStats(){
        cout<<"===== Simulation Results =====\n";
        cout<<"a. number of L1 reads:			"<<num_reads<<"\n";
        cout<<"b. number of L1 read misses:		"<<num_read_miss<<"\n";
        cout<<"c. number of L1 writes:			"<<num_writes<<"\n";
        cout<<"d. number of L1 write misses:		"<<num_write_miss<<"\n";
        cout<<"e. L1 miss rate:			"<<((float)num_read_miss+(float)num_write_miss)/((float)num_reads+(float)num_writes)<<"\n";
        cout<<"f. number of writebacks from L1 memory:	"<<num_write_backs<<"\n";
        cout<<"g. total memory traffic:		"<<num_memory_access<<"\n";
    }

    bool ReadFromAddress(unsigned int address){        
        int set_index = address%number_of_sets;        
        unsigned int tag = address/((unsigned int)assoc*(unsigned int)block_size);
        vector<Block> target_set=sets[set_index].blocks;   
        num_reads++;
        unsigned int max_count;
        bool is_found=false;
        bool invalid_block_found=false;
        vector<Block>::iterator lru_block;
        vector<Block>::iterator invalid_block;
        for(vector<Block>::iterator it=target_set.begin();it != target_set.end();it++){
            if(it->is_valid==1 && it->tag_id == tag){
                is_found=true;
                it->timestamp=call_count++;
            }
            if(it->timestamp < lru_block->timestamp){
                lru_block=it;
            }
            if(!it->is_valid && !invalid_block_found){
                invalid_block=it;
                invalid_block_found=true;
            }
        }
        if(!is_found){

            num_read_miss++;
            num_memory_access++;

            
            if(invalid_block_found){
                invalid_block->tag_id=tag;
                invalid_block->is_valid=true;
                invalid_block->timestamp=call_count++;
            }else{
                if(lru_block->is_dirty){
                    //write back to lower level cache
                    num_write_backs++;
                    num_memory_access++;
                }
                lru_block->tag_id=tag;
                lru_block->is_valid=true;
                lru_block->timestamp=call_count++;
            }
            
        }
        return true;
                
    }
    bool WriteToAddress(unsigned int address){
        num_writes++;
        int set_index = address%number_of_sets;        
        unsigned int tag = address/((unsigned int)assoc*(unsigned int)block_size);
        vector<Block> target_set=sets[set_index].blocks;        
        unsigned int max_count;
        bool is_found=false;
        bool invalid_block_found=false;
        vector<Block>::iterator lru_block;
        vector<Block>::iterator invalid_block;
        for(vector<Block>::iterator it=target_set.begin();it != target_set.end();it++){
            if(it->is_valid==1 && it->tag_id == tag){
                is_found=true;
                it->timestamp=call_count++;
                it->is_dirty=true;
            }
            if(it->timestamp < lru_block->timestamp){
                lru_block=it;
            }
            if(!it->is_valid && !invalid_block_found){
                invalid_block=it;
                invalid_block_found=true;
            }
        }
        if(!is_found){

            num_write_miss++;
            //get block value from lower level memory
            //
            num_memory_access++;

            
            if(invalid_block_found){
                invalid_block->tag_id=tag;
                invalid_block->is_valid=true;
                invalid_block->timestamp=call_count++;
                invalid_block->is_dirty=true;
            }else{
                if(lru_block->is_dirty){
                    //write back to lower level cache
                    num_write_backs++;
                    num_memory_access++;
                }
                lru_block->tag_id=tag;
                lru_block->is_valid=true;
                lru_block->timestamp=call_count++;
                lru_block->is_dirty=true;
            }
            
        }
        return true;
    }

};

int main(int argc, char* _argv[])
{
    if(argc != ARGCOUNT ){
        cout<<"Improper Arguments \n";
        cout<<"Example Usage: sim_cache <BLOCKSIZE> <L1_SIZE> <L1_ASSOC> <L2_SIZE> <L2_ASSOC> <L2_DATA_BLOCKS> <L2_ADDR_TAGS> <trace_file> \n";
        return SYSERR;
    }
    int block_size      =   atoi(_argv[1]);
    int l1_size         =   atoi(_argv[2]);
    int l1_assoc        =   atoi(_argv[3]);
    // int l2_size         =   atoi(_argv[4]);
    // int l2_assoc        =   atoi(_argv[5]);
    // int l2_data_blocks  =   atoi(_argv[6]);
    // int l2_addr_tags    =   atoi(_argv[7]);
    char* trace_file      =   _argv[8];
    ifstream infile;
    infile.open(trace_file);
    if(!infile.is_open()){
        cerr<<"Unable to open "<<trace_file<<"!!!";
        return SYSERR;
    }
    string str;
    Cache cache = Cache(block_size,l1_size,l1_assoc);
    while(!infile.eof()){
        getline(infile,str);        
        char first_char=str[0];
        if(first_char=='r'){
            str[0]='0';
            str[1]='X';
            unsigned int addr = stoi(str);
            cache.ReadFromAddress(addr);
        }else if(first_char=='w'){
            str[0]='0';
            str[1]='X';
            unsigned int addr = stoi(str);
            cache.WriteToAddress(addr);
        }else{
            cout<<"Unexpected value in trace file!!! \n";
        }
    }
    infile.close();
    cache.DisplayStats();
    
    return 0;
}