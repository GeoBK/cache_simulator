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
    int tag_selector=0;     
};

bool CompareTimestamps(Sector a, Sector b){
    return a.timestamp>b.timestamp;
}

class Sector{
    public:
    Sector(int data_blocks,int addr_tags){
        for(int i=0;i<data_blocks;i++){
            blocks.push_back(Block());
        }
        for(int i=0;i<addr_tags;i++){
            tags.push_back(0);
        }
    }
    vector<Block> blocks;
    vector<int> tags;
    unsigned int timestamp;
};

class Set{
    public:
    Set(int associativity, int data_blocks, int addr_tags){
        for(int i=0;i<associativity;i++){
            sectors.push_back(Sector(data_blocks,addr_tags));
        }        
    }   
    vector<Sector> sectors;
};


class Cache {
    private:
    int block_size;
    int cache_size;        
    int assoc;
    int data_blocks;
    int addr_tags;
    Cache* next_level       =   nullptr;
    vector<Set> sets;

    int num_reads                   =   0;
    int num_writes                  =   0;
    int num_read_miss               =   0;
    int num_write_miss              =   0;
    int num_write_backs             =   0;
    int num_memory_access           =   0;

    int number_of_sets              =   0;    
    int total_sectors_in_cache      =   0;   

    public:
    Cache(int block_size, int cache_size, int associativity, int data_blocks, int addr_tags): 
        block_size(block_size), cache_size(cache_size), assoc(associativity), data_blocks(data_blocks), addr_tags(addr_tags){
            total_sectors_in_cache = cache_size / (block_size*data_blocks);            
            number_of_sets = total_sectors_in_cache/associativity;
            for(int i=0;i<number_of_sets;i++){
                sets.push_back(Set(associativity,data_blocks,addr_tags));
            }
        }

    void AttachNextLevelCache(Cache *c){
        next_level=c;
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

    Block FetchBlock(unsigned int address,char mode){
        if(mode=='r'){
            num_reads++;
        }else if(mode == 'w'){
            num_writes++;
        }

        unsigned int c = address/block_size;  
        unsigned int c0 =   c%data_blocks;  
        unsigned int c1 =   (c/data_blocks)%number_of_sets;        
        unsigned int c2 =   (c/(data_blocks*number_of_sets))%addr_tags;
        unsigned int c3 =   c/(data_blocks*number_of_sets*addr_tags);
        bool is_found=false;
        bool invalid_block_found=false;
        vector<Sector>::reverse_iterator lru_block=sets[c1].sectors.rbegin();
        vector<Sector>::reverse_iterator invalid_block;
        for(vector<Sector>::reverse_iterator it=sets[c1].sectors.rbegin();it != sets[c1].sectors.rend();it++){            
            if(it->blocks[c0].is_valid && it->blocks[c0].tag_selector == c2 && it->tags[c2] == c3){
                is_found=true;
                it->timestamp=call_count++;
                return it->blocks[0];
            }
            if(it->timestamp < lru_block->timestamp){
                lru_block=it;
            }
            if(!it->blocks[c0].is_valid && !invalid_block_found){
                invalid_block=it;
                invalid_block_found=true;
            }
        }
        if(!is_found){  
            num_memory_access++;
            if(mode=='r'){
                num_read_miss++;
            }else if(mode == 'w'){
                num_write_miss++;
            }
            if(next_level != nullptr){
                next_level->ReadFromAddress(address);
            }          
            if(invalid_block_found){
                invalid_block->blocks[c0].is_valid=true;
                invalid_block->blocks[c0].is_dirty=false;
                invalid_block->blocks[c0].tag_selector=c2;
                invalid_block->tags[c2]=c3;
                invalid_block->timestamp=call_count++;
                return invalid_block->blocks[c0];
                
            }else{
                // blocks associated with the old address tag need to be invalidated
                int addr_tag_to_be_evicted = lru_block->blocks[c0].tag_selector;
                int i=0;
                for(vector<Block>::iterator bl_it=lru_block->blocks.begin();bl_it!=lru_block->blocks.end();bl_it++){
                    if(bl_it->tag_selector==addr_tag_to_be_evicted){                        
                        if(bl_it->is_valid && bl_it->is_dirty){
                            num_write_backs++;
                            num_memory_access++;
                            if(next_level != nullptr){
                                int old_addr = (((lru_block->tags[addr_tag_to_be_evicted]*addr_tags+addr_tag_to_be_evicted)*number_of_sets+c1)*data_blocks+i)*block_size;
                                next_level->WriteToAddress(old_addr);
                            }
                        }
                        bl_it->is_valid=false;
                    }
                    i++;
                }
                lru_block->blocks[c0].is_valid=true;
                lru_block->blocks[c0].is_dirty=false;
                lru_block->blocks[c0].tag_selector=c2;
                lru_block->tags[c2]=c3;
                lru_block->timestamp=call_count++; 
                return lru_block->blocks[c0];
            }            
        }

    }

    bool ReadFromAddress(unsigned int address){         
        FetchBlock(address,'r');
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
        
        Block fetchedBlock = FetchBlock(address,'w');
        fetchedBlock.is_dirty=true;
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
    
    void PrintDataCacheContent(){               
        int i=0;
        for(vector<Set>::iterator it=sets.begin();it != sets.end();it++){
            cout<<"set\t"<<i<<":";
            sort(it->sectors.begin(),it->sectors.end(),CompareTimestamps);
            for(vector<Sector>::iterator sec_it=it->sectors.begin(); sec_it!=it->sectors.end(); sec_it++){ 
                for(vector<Block>::iterator bl_it= sec_it->blocks.begin(); bl_it!=sec_it->blocks.end();bl_it++){
                    char dirty_char;
                    char valid_char;
                    if(bl_it->is_dirty){
                        dirty_char='D';
                    }else{
                        dirty_char='N';
                    }
                    if(bl_it->is_valid){
                        valid_char='V';
                    }else{
                        valid_char='I';
                    }
                    printf("\t%d,%c,%c\t",bl_it->tag_selector,valid_char,dirty_char);
                } 
                printf("||");          
            }
            cout<<"\n";
            i++;
        }

    }

    void PrintAddressCacheContent(){
        int i=0;
        for(vector<Set>::iterator it=sets.begin();it != sets.end();it++){
            cout<<"set\t"<<i<<":";
            sort(it->sectors.begin(),it->sectors.end(),CompareTimestamps);
            for(vector<Sector>::iterator sec_it=it->sectors.begin(); sec_it!=it->sectors.end(); sec_it++){ 
                for(vector<int>::iterator addr_it= sec_it->tags.begin(); addr_it!=sec_it->tags.end();addr_it++){
                    printf("\t%d\t",*addr_it);
                } 
                printf("||");               
            }
            cout<<"\n";
            i++;
        }

    }

    void PrintCacheContent(){        
        int i=0;
        for(vector<Set>::iterator it=sets.begin();it != sets.end();it++){
            cout<<"set\t"<<i<<":";
            sort(it->sectors.begin(),it->sectors.end(),CompareTimestamps);
            for(vector<Sector>::iterator sec_it=it->sectors.begin(); sec_it!=it->sectors.end(); sec_it++){
                char dirty_char;
                for(vector<Block>::iterator bl_it=sec_it->blocks.begin();bl_it!=sec_it->blocks.end();bl_it++){
                    if(bl_it->is_dirty){
                        dirty_char='D';
                    }else{
                        dirty_char='N';
                    }
                    printf("\t%0x %c\t||",sec_it->tags[bl_it->tag_selector]*addr_tags+bl_it->tag_selector,dirty_char);
                }
                
                

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
    Cache L1_cache = Cache(block_size,l1_size,l1_assoc,1,1);
    Cache L2_cache = Cache(block_size,l2_size,l2_assoc,l2_data_blocks,l2_addr_tags);
    L1_cache.AttachNextLevelCache(&L2_cache);
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
                L1_cache.ReadFromAddress(addr);
            }else if(first_char=='w'){
                str[0]='0';
                str[1]='X';
                unsigned int addr = stoul(str,nullptr,16);
                L1_cache.WriteToAddress(addr);
            }else{
                cout<<"Unexpected value in trace file!!! \n";
            }
        }        
    }
    infile.close();
    cout<< "----------------------L1--------------------------\n ";
    L1_cache.PrintCacheContent();
    cout<<"\n";
    L1_cache.DisplayStats();    
    cout<< "----------------------L2--------------------------\n ";
    L2_cache.PrintCacheContent();
    cout<<"\n";
    L2_cache.DisplayStats();   
    return 0;
}