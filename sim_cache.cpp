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
    unsigned int tag_selector=0;     
};

class Tag {
    public:
    unsigned int tag_id;
    bool is_valid = false;
};

class Sector{
    public:
    Sector(int data_blocks,int addr_tags){
        for(int i=0;i<data_blocks;i++){
            blocks.push_back(Block());
        }
        for(int i=0;i<addr_tags;i++){
            tags.push_back(Tag());
        }
    }
    vector<Block> blocks;
    vector<Tag> tags;
    bool is_valid=false;
    unsigned int timestamp=0;
};

bool CompareTimestamps(Sector a, Sector b){
    return a.timestamp>b.timestamp;
}

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
    unsigned int block_size;
    unsigned int cache_size;        
    unsigned int assoc;
    
    Cache* next_level       =   nullptr;
    vector<Set> sets;

    void evictSector(vector<Sector>::reverse_iterator sector_to_replace,unsigned int c0,unsigned int c1,unsigned int c2, unsigned int c3, char mode){
        // blocks associated with the old address tag need to be invalidated
        unsigned int ind_addr_tag_to_be_evicted = c2;            
        unsigned int i=0;
        for(vector<Block>::iterator bl_it=sector_to_replace->blocks.begin();bl_it!=sector_to_replace->blocks.end();bl_it++){
            if((bl_it->tag_selector==ind_addr_tag_to_be_evicted && c3!=sector_to_replace->tags[bl_it->tag_selector].tag_id) || (i == c0)){    
                //                    
                if(bl_it->is_valid)
                {
                    if(bl_it->is_dirty)
                    {
                        num_write_backs++;
                        num_memory_access++;
                        unsigned int old_addr = (((sector_to_replace->tags[ind_addr_tag_to_be_evicted].tag_id*addr_tags+ind_addr_tag_to_be_evicted)*number_of_sets+c1)*data_blocks+i)*block_size;
                        if(next_level != nullptr){                        
                            next_level->WriteToAddress(old_addr);
                        }
                    }
                    bl_it->tag_selector=0;;
                    bl_it->is_valid=false;
                    bl_it->is_dirty=false;
                }                
            }
            i++;
        }        
        sector_to_replace->blocks[c0].is_valid=true;
        sector_to_replace->tags[c2].is_valid=true;
        sector_to_replace->blocks[c0].is_dirty=false;
        if(mode=='w'){
            sector_to_replace->blocks[c0].is_dirty= true;
        }
        sector_to_replace->blocks[c0].tag_selector=c2;
        sector_to_replace->tags[c2].tag_id=c3;
        sector_to_replace->timestamp=call_count++;        
    }

    public:
    unsigned int data_blocks;
    unsigned int addr_tags;
    int num_reads                   =   0;
    int num_writes                  =   0;
    int num_read_miss               =   0;
    int num_write_miss              =   0;
    int num_write_backs             =   0;
    int num_memory_access           =   0;

    int sector_miss                 =   0;
    int cache_block_miss            =   0;

    unsigned int number_of_sets              =   0;    
    unsigned int total_sectors_in_cache      =   0;   

    int cache_level                 =   0;

    
    
    Cache(int block_size, int cache_size, int associativity, int data_blocks, int addr_tags, int level): 
        block_size(block_size), cache_size(cache_size), assoc(associativity), data_blocks(data_blocks), addr_tags(addr_tags), cache_level(level){
            total_sectors_in_cache = cache_size / (block_size*data_blocks);            
            number_of_sets = total_sectors_in_cache/associativity;
            for(unsigned int i=0;i<number_of_sets;i++){
                sets.push_back(Set(associativity,data_blocks,addr_tags));
            }
        }

    void AttachNextLevelCache(Cache *c){
        next_level=c;
    }    

    void FetchBlock(unsigned int address,char mode){
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
        bool is_tag_found = false;
        bool is_block_found = false;

        bool invalid_block_found=false;
        bool invalid_tag_block_found=false;
        vector<Sector>::reverse_iterator lru_block=sets[c1].sectors.rbegin();
        vector<Sector>::reverse_iterator invalid_block;
        vector<Sector>::reverse_iterator invalid_tag_block;
        
        for(vector<Sector>::reverse_iterator it=sets[c1].sectors.rbegin();it != sets[c1].sectors.rend();it++)
        {
            for(vector<Block>::iterator bl_it=it->blocks.begin(); bl_it != it->blocks.end(); bl_it++){
                if(bl_it->is_valid){
                    is_tag_found=true;
                }
            }
            if((it->tags[c2].tag_id == c3 && it->tags[c2].is_valid)){
                
                if(it->blocks[c0].is_valid && it->blocks[c0].tag_selector == c2 ){
                    is_found=true;
                    it->timestamp=call_count++;
                    if(mode=='w'){
                        it->blocks[c0].is_dirty= true;
                    }
                                   
                }                                
            }
            if(it->blocks[c0].is_valid && it->blocks[c0].tag_selector == c2){
                is_block_found=true;
            }            

            //Third would be lru
            if(it->timestamp < lru_block->timestamp){
                lru_block=it;
            }
            //If block is not found first priority to replace would be for matching tag but invalid block
            if(it->tags[c2].tag_id==c3 && it->tags[c2].is_valid && !it->blocks[c0].is_valid){
                invalid_block=it;
                invalid_block_found=true;
            }
            //Second best option would be invalid tag
            if(!it->tags[c2].is_valid  && !invalid_tag_block_found){
                invalid_tag_block=it;
                invalid_tag_block_found=true;
            }
        }
        if(cache_level==1){
            printf("------------------------------------------------\n");
            if(mode=='r'){
                printf("# %d: read %0x\n",line,address);
                printf("L1 read: %0x (tag %0x, index %d)\n", address, c3, c1);
            }else if (mode=='w'){
                printf("# %d: write %0x\n",line,address);
                printf("L1 write: %0x (tag %0x, index %d)\n", address, c3, c1);
            }            
            if(is_found){
                printf("L1 hit \n");
            }else{
                printf("L1 miss \n");
            } 
        }else if(cache_level==2){
            if(mode=='r'){
                printf("L2 read: %0x (C0 %0x, C1 %0x, C2 %0x, C3 %0x) \n",address,c0,c1,c2,c3);
            }else if(mode == 'w'){
                printf("L2 write:  (C0 %0x, C1 %0x, C2 %0x, C3 %0x) \n",c0,c1,c2,c3);
            }
            
            if(is_found){
                printf("L2 hit \n");
            }else{
                printf("L2 miss \n");
            } 
        }
        if(!is_block_found){
            cache_block_miss++;
        }
        if(!is_tag_found){
                sector_miss++;                       
        }
        if(!is_found){  
            num_memory_access++;
            if(mode=='r'){
                num_read_miss++;
            }else if(mode == 'w'){
                num_write_miss++;
            }

            if(invalid_block_found){
                evictSector(invalid_block,c0,c1,c2,c3,mode);

            }else if(invalid_tag_block_found){
                evictSector(invalid_tag_block,c0,c1,c2,c3,mode);
                
            }else{
                evictSector(lru_block,c0,c1,c2,c3,mode);
            }
            if(next_level != nullptr){
                next_level->ReadFromAddress(address);
            }            
        }
        if(cache_level==1){
            printf("L1 update LRU\n");
        }else
        {
            printf("L2 update LRU\n");
        }        
    }


    bool ReadFromAddress(unsigned int address){         
        FetchBlock(address,'r');        
        return true;
                
    }
    bool WriteToAddress(unsigned int address){        
        FetchBlock(address,'w');              
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
                    printf("\t%0x,%c,%c\t",bl_it->tag_selector,valid_char,dirty_char);
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
                for(vector<Tag>::iterator addr_it= sec_it->tags.begin(); addr_it!=sec_it->tags.end();addr_it++){
                    printf("\t%0x\t",addr_it->tag_id);
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
                    printf("\t%0x %c\t||",sec_it->tags[bl_it->tag_selector].tag_id*addr_tags+bl_it->tag_selector,dirty_char);
                }
            }
            cout<<"\n";
            i++;
        }

    }

};

void DisplayStats(Cache l1, Cache l2){ 
    if(l2.addr_tags!=1 || l2.data_blocks!=1){
        cout<<"\n";
    }       
    cout<<"===== Simulation Results =====\n";
    cout<<"a. number of L1 reads:			"<<l1.num_reads<<"\n";
    cout<<"b. number of L1 read misses:		"<<l1.num_read_miss<<"\n";
    cout<<"c. number of L1 writes:			"<<l1.num_writes<<"\n";
    cout<<"d. number of L1 write misses:		"<<l1.num_write_miss<<"\n";
    printf("e. L1 miss rate:			%0.4f\n",((float)l1.num_read_miss+(float)l1.num_write_miss)/((float)l1.num_reads+(float)l1.num_writes));        
    cout<<"f. number of writebacks from L1 memory:	"<<l1.num_write_backs<<"\n";
    cout<<"g. number of L2 reads:			"<<l2.num_reads<<"\n";
    cout<<"h. number of L2 read misses:		"<<l2.num_read_miss<<"\n";
    cout<<"i. number of L2 writes:			"<<l2.num_writes<<"\n";
    cout<<"j. number of L2 write misses:		"<<l2.num_write_miss<<"\n";
    if(l2.data_blocks==1 && l2.addr_tags==1){
        printf("k. L2 miss rate:			%0.4f\n",(float)l2.num_read_miss/(float)l2.num_reads);        
        cout<<"l. number of writebacks from L2 memory:	"<<l2.num_write_backs<<"\n";
        cout<<"m. total memory traffic:		"<<l2.num_memory_access<<"\n";
    }else{
        cout<<"k. number of L2 sector misses:		"<<l2.sector_miss<<"\n";
        cout<<"l. number of L2 cache block misses:		"<< l2.num_read_miss + l2.num_write_miss - l2.sector_miss <<"\n";
        printf("m. L2 miss rate:			%0.4f\n",(float)l2.num_read_miss/(float)l2.num_reads);        
        cout<<"n. number of writebacks from L2 memory:	"<<l2.num_write_backs<<"\n";
        cout<<"o. total memory traffic:		"<<l2.num_memory_access<<"\n";
    }
    
    
}

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
    Cache L1_cache = Cache(block_size,l1_size,l1_assoc,1,1,1);
    Cache L2_cache = Cache(block_size,l2_size,l2_assoc,l2_data_blocks,l2_addr_tags,2);
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

    cout<< "\n===== L1 contents =====\n ";
    L1_cache.PrintCacheContent();
    cout<<"\n";
    
    if(l2_data_blocks==1 && l2_addr_tags==1){
        cout<<"===== L2 contents =====\n";
        cout<<"";
        L2_cache.PrintCacheContent();
        cout<<"\n";
         
    }else{        
        cout<<"===== L2 Address Array contents =====\n";
        L2_cache.PrintAddressCacheContent();
        cout<<"\n===== L2 Data Array contents =====\n";
        L2_cache.PrintDataCacheContent();

    }
    DisplayStats(L1_cache,L2_cache);  
    
    
    return 0;
}