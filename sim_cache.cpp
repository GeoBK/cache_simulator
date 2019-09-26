#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#define SYSERR      1
#define ARGCOUNT    9
using namespace std;

unsigned int call_count;
unsigned int line=0;
class Block {
    public:    
    bool is_dirty=false;
    bool is_valid=false;
    unsigned int tag_id;
    unsigned int timestamp;
};

bool CompareTimestamp(Block a, Block b){
    return a.timestamp>b.timestamp;
}

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
        printf("e. L1 miss rate:			%1.4f\n",((float)num_read_miss+(float)num_write_miss)/((float)num_reads+(float)num_writes));        
        cout<<"f. number of writebacks from L1 memory:	"<<num_write_backs<<"\n";
        cout<<"g. total memory traffic:		"<<num_memory_access<<"\n";
    }

    bool ReadFromAddress(unsigned int address){        
        int set_index = (address/block_size)%number_of_sets;        
        unsigned int tag = address/((unsigned int)number_of_sets*(unsigned int)block_size);
           
        num_reads++;        
        bool is_found=false;
        bool invalid_block_found=false;
        vector<Block>::reverse_iterator lru_block=sets[set_index].blocks.rbegin();
        vector<Block>::reverse_iterator invalid_block;
        for(vector<Block>::reverse_iterator it=sets[set_index].blocks.rbegin();it != sets[set_index].blocks.rend();it++){
            if(it->is_valid && it->tag_id == tag){
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
                invalid_block->is_dirty=false;
            }else{
                if(lru_block->is_dirty){
                    //write back to lower level cache
                    num_write_backs++;
                    num_memory_access++;
                }
                lru_block->tag_id=tag;
                lru_block->is_valid=true;
                lru_block->timestamp=call_count++;
                lru_block->is_dirty=false;
            }
            
        }
        // printf("------------------------------------------------\n");
        // printf("# %d: read %0x\n",line,address);
        // printf("L1 read: %0x (tag %0x, index %d)\n", address, tag, set_index);
        // if(is_found){
        //     printf("L1 hit \n");
        // }else{
        //     printf("L1 miss \n");
        // }        
        // printf("L1 update LRU\n");
        return true;
                
    }
    bool WriteToAddress(unsigned int address){
        num_writes++;
        int set_index = (address/block_size)%number_of_sets;        
        unsigned int tag = address/((unsigned int)number_of_sets*(unsigned int)block_size);
                
        bool is_found=false;
        bool invalid_block_found=false;
        vector<Block>::reverse_iterator lru_block = sets[set_index].blocks.rbegin();
        vector<Block>::reverse_iterator invalid_block;
        for(vector<Block>::reverse_iterator it=sets[set_index].blocks.rbegin();it != sets[set_index].blocks.rend();it++){
            if(it->is_valid && it->tag_id == tag){
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
        // printf("------------------------------------------------\n");
        // printf("# %d: write %0x\n",line,address);
        // printf("L1 write: %0x (tag %0x, index %d)\n", address, tag, set_index);
        // if(is_found){
        //     printf("L1 hit \n");
        // }else{
        //     printf("L1 miss \n");
        // }   
        // printf("L1 set dirty\n");
        // printf("L1 update LRU\n");
        return true;
    }
    
    void PrintCacheContent(){
        cout<<"\n";
        cout<<"===== L1 contents =====\n";
        int i=0;
        for(vector<Set>::iterator it=sets.begin();it != sets.end();it++){
            cout<<"set\t"<<i<<":";
            sort(it->blocks.begin(),it->blocks.end(),CompareTimestamp);
            for(vector<Block>::iterator bl_it=it->blocks.begin(); bl_it!=it->blocks.end(); bl_it++){                
                char dirty_char;
                if(bl_it->is_dirty){
                    dirty_char='D';
                }else{
                    dirty_char='N';
                }
                printf("\t%0x %c\t||",bl_it->tag_id,dirty_char);
                
            }
            cout<<"\n";
            i++;
        }

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
    int l2_size         =   atoi(_argv[4]);
    int l2_assoc        =   atoi(_argv[5]);
    int l2_data_blocks  =   atoi(_argv[6]);
    int l2_addr_tags    =   atoi(_argv[7]);
    char* trace_file      =   _argv[8];
    ifstream infile;
    infile.open(trace_file);
    if(!infile.is_open()){
        cerr<<"Unable to open "<<trace_file<<"!!!";
        return SYSERR;
    }
    string str;
    Cache cache = Cache(block_size,l1_size,l1_assoc);
    printf("  ===== Simulator configuration =====\n");
    printf("  BLOCKSIZE:                        %d\n",block_size);
    printf("  L1_SIZE:                          %d\n",l1_size);
    printf("  L1_ASSOC:                         %d\n",l1_assoc);
    printf("  L2_SIZE:                          %d\n",l2_size);
    printf("  L2_ASSOC:                         %d\n",l2_assoc);
    printf("  L2_DATA_BLOCKS:                   %d\n",l2_data_blocks);
    printf("  L2_ADDRESS_TAGS:                  %d\n",l2_addr_tags);
    printf("  trace_file:                       %s\n",trace_file);
    while(!infile.eof()){
        line++;
        getline(infile,str);     
        if(str!=""){
            char first_char=str[0];
            if(first_char=='r'){
                str[0]='0';
                str[1]='X';
                unsigned int addr = stoul(str,nullptr,16);
                cache.ReadFromAddress(addr);
            }else if(first_char=='w'){
                str[0]='0';
                str[1]='X';
                unsigned int addr = stoul(str,nullptr,16);
                cache.WriteToAddress(addr);
            }else{
                cout<<"Unexpected value in trace file!!! \n";
            }
        }        
    }
    infile.close();
    cache.PrintCacheContent();
    cout<<"\n";
    cache.DisplayStats();    
    return 0;
}