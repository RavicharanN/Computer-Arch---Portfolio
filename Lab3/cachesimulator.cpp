/*
Cache Simulator
Level one L1 and level two L2 cache parameters are read from file (block size, line per set and set per cache).
The 32 bit address is divided into tag bits (t), set index bits (s) and block offset bits (b)
s = log2(#sets)   b = log2(block size in bytes)  t=32-s-b
32 bit address (MSB -> LSB): TAG || SET || OFFSET

Tag Bits   : the tag field along with the valid bit is used to determine whether the block in the cache is valid or not.
Index Bits : the set index field is used to determine which set in the cache the block is stored in.
Offset Bits: the offset field is used to determine which byte in the block is being accessed.
*/

#include <algorithm>
#include <bitset>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <vector>
using namespace std;
//access state:
#define NA 0 // no action
#define RH 1 // read hit
#define RM 2 // read miss
#define WH 3 // Write hit
#define WM 4 // write miss
#define NOWRITEMEM 5 // no write to memory
#define WRITEMEM 6   // write to memory

struct config
{
    int L1blocksize;
    int L1setsize;
    int L1size;
    int L2blocksize;
    int L2setsize;
    int L2size;
};

struct CacheBlock
{
    // we don't actually need to allocate space for data, because we only need to simulate the cache action
    // or else it would have looked something like this: vector<number of bytes> Data;
    int valid;
    int dirty;
    long long tag;
};

// You can design your own data structure for L1 and L2 cache; just an example here
class cache
{
    vector<vector<CacheBlock>> L1, L2;

public:
    unsigned long idx_bit_l1, idx_bit_l2;                // Identify sets
    unsigned long block_bits_l1, block_bits_l2;         // Identify block offsets
    unsigned long tag_bits_l1, tag_bits_l2;             // Identify  and check ways

    cache(config cacheconfig)
    {
        block_bits_l1 = log2(cacheconfig.L1blocksize);
        block_bits_l2 = log2(cacheconfig.L2blocksize);

        // Handle for fully associative cache
        int setSizeL1 = cacheconfig.L1setsize;
        if (setSizeL1 == 0)
            setSizeL1 = cacheconfig.L1size*1024/cacheconfig.L1blocksize;

        int setSizeL2 = cacheconfig.L2setsize;
        if (setSizeL2 == 0)
            setSizeL2 = cacheconfig.L2size*1024/cacheconfig.L2blocksize;
        
        idx_bit_l1 = log2(cacheconfig.L1size*1024 / (setSizeL1 * cacheconfig.L1blocksize));
        idx_bit_l2 = log2(cacheconfig.L2size*1024 / (setSizeL2 * cacheconfig.L2blocksize));

        tag_bits_l1 = 32 - block_bits_l1 - idx_bit_l1;
        tag_bits_l2 = 32 - block_bits_l2 - idx_bit_l2;

        int num_sets1 = pow(2, idx_bit_l1);
        int num_sets2 = pow(2, idx_bit_l2);
        
        // Set Params for L1 
        L1.resize(num_sets1);
        for (int i = 0; i < num_sets1; i++)
        {
            L1[i].resize(setSizeL1);
        }

        L2.resize(num_sets2);
        for (int i = 0; i < num_sets2; i++)
        {
            L2[i].resize(setSizeL2);
        }
    }
    
    vector<vector<CacheBlock>> getL1()
    {
        return this->L1;
    }

    vector<vector<CacheBlock>> getL2()
    {
        return this->L2;
    }

    // auto write(auto addr){
    //     /*
    //     step 1: select the set in our L1 cache using set index bits
    //     step 2: iterate through each way in the current set
    //         - If Matching tag and Valid Bit High -> WriteHit!
    //                                                 -> Dirty Bit High
    //     step 3: Otherwise? -> WriteMiss!

    //     return WH or WM
    //     */
    // }

    // auto writeL2(auto addr){
    //     /*
    //     step 1: select the set in our L2 cache using set index bits
    //     step 2: iterate through each way in the current set
    //         - If Matching tag and Valid Bit High -> WriteHit!
    //                                              -> Dirty Bit High
    //     step 3: Otherwise? -> WriteMiss!

    //     return {WM or WH, WRITEMEM or NOWRITEMEM}
    //     */
    // }

    // auto readL1(auto addr){
    //     /*
    //     step 1: select the set in our L1 cache using set index bits
    //     step 2: iterate through each way in the current set
    //         - If Matching tag and Valid Bit High -> ReadHit!
    //     step 3: Otherwise? -> ReadMiss!

    //     return RH or RM
    //     */
    // }

    // auto readL2(auto addr){
    //     /*
    //     step 1: select the set in our L2 cache using set index bits
    //     step 2: iterate through each way in the current set
    //         - If Matching tag and Valid Bit High -> ReadHit!
    //                                              -> copy dirty bit
    //     step 3: otherwise? -> ReadMiss! -> need to pull data from Main Memory
    //     step 4: find a place in L1 for our requested data
    //         - case 1: empty way in L1 -> place requested data
    //         - case 2: no empty way in L1 -> evict from L1 to L2
    //                 - case 2.1: empty way in L2 -> place evicted L1 data there
    //                 - case 2.2: no empty way in L2 -> evict from L2 to memory

    //     return {RM or RH, WRITEMEM or NOWRITEMEM}
    //     */
    // }
};

// /*********************************** ↑↑↑ Todo: Implement by you ↑↑↑ ******************************************/


int main(int argc, char *argv[])
{

    config cacheconfig;
    ifstream cache_params;
    string dummyLine;
    cache_params.open(argv[1]);
    while (!cache_params.eof())                   // read config file
    {
        cache_params >> dummyLine;                // L1:
        cache_params >> cacheconfig.L1blocksize;  // L1 Block size
        cache_params >> cacheconfig.L1setsize;    // L1 Associativity
        cache_params >> cacheconfig.L1size;       // L1 Cache Size
        cache_params >> dummyLine;                // L2:
        cache_params >> cacheconfig.L2blocksize;  // L2 Block size
        cache_params >> cacheconfig.L2setsize;    // L2 Associativity
        cache_params >> cacheconfig.L2size;       // L2 Cache Size
    }
    ifstream traces;
    ofstream tracesout;
    string outname;
    outname = string(argv[2]) + ".out";
    traces.open(argv[2]);
    tracesout.open(outname.c_str());
    string line;
    string accesstype;     // the Read/Write access type from the memory trace;
    string xaddr;          // the address from the memory trace store in hex;
    unsigned int addr;     // the address from the memory trace store in unsigned int;
    bitset<32> accessaddr; // the address from the memory trace store in the bitset;


/*********************************** ↓↓↓ Todo: Implement by you ↓↓↓ ******************************************/
    
    if (cacheconfig.L1blocksize!=cacheconfig.L2blocksize){
        printf("please test with the same block size\n");
        return 1;
    }

    cache cache(cacheconfig);
    vector<vector<CacheBlock>> L1 = cache.getL1();
    vector<vector<CacheBlock>> L2 = cache.getL2();
    unsigned long idx_bit_l1 = cache.idx_bit_l1, idx_bit_l2 = cache.idx_bit_l2;
    unsigned long tag_bits_l1 = cache.tag_bits_l1, tag_bits_l2 = cache.tag_bits_l2;
    vector<int> counterL1 (pow(2, idx_bit_l1), 0);
    vector<int> counterL2 (pow(2, idx_bit_l2), 0);

    if (traces.is_open() && tracesout.is_open())
    {
        while (getline(traces, line))
        { // read mem access file and access Cache

            istringstream iss(line);
            if (!(iss >> accesstype >> xaddr)){
                break;
            }
            stringstream saddr(xaddr);
            saddr >> std::hex >> addr;

            accessaddr = bitset<32>(addr);
            int L1AcceState = NA; // L1 access state variable, can be one of NA, RH, RM, WH, WM;
            int L2AcceState = NA; // L2 access state variable, can be one of NA, RH, RM, WH, WM;
            int MemAcceState = NOWRITEMEM;

            int way_tagL1 = (bitset<32>(accessaddr.to_string().substr(0, tag_bits_l1))).to_ulong();
            int way_tagL2 = (bitset<32>(accessaddr.to_string().substr(0, tag_bits_l2))).to_ulong();
            int set_idxL1 = (bitset<32>(accessaddr.to_string().substr(tag_bits_l1, idx_bit_l1))).to_ulong();
            int set_idxL2 = (bitset<32>(accessaddr.to_string().substr(tag_bits_l2, idx_bit_l2))).to_ulong();

            // Can only got in a set
            // Id way is full evict 

            // access the L1 and L2 Cache according to the trace;
            if (accesstype.compare("R") == 0)  // a Read request
            {
                // READ L1
                // Check for L1 hit in set_idxL1
                for (int i = 0; i < L1[set_idxL1].size(); i++)
                {
                    if (L1[set_idxL1][i].tag == way_tagL1 && L1[set_idxL1][i].valid == 1)
                    {
                        // L1 HIT
                        L1AcceState = RH;
                        break;
                    }
                    L1AcceState = RM;
                }

                // At this point we know if its L1 hit or miss based on access state
                // Decide for L2 after this, if miss check L2
                if (L1AcceState == RM)
                {
                    // Search L2 ways 
                    for (int i = 0; i < L2[set_idxL2].size(); i++)
                    {
                        // L2 Hit 
                        if (L2[set_idxL2][i].tag == way_tagL2 && L2[set_idxL2][i].valid == 1)
                        {
                            L2AcceState = RH;
                            L2[set_idxL2][i].valid = 0; // First set valid bit on L2 low when hit 

                            // Block is moved to L1 - How? Iterate and find empty way 
                            bool emptyWayExists = false;
                            for (int j = 0; j < L1[set_idxL1].size(); j++)
                            {
                                if (L1[set_idxL1][j].valid == 0)    // Empty way exists - move tag and copy dirtybit
                                {
                                    emptyWayExists = true;
                                    L1[set_idxL1][j].tag = way_tagL1;                                                       // Question - This takes waytag1
                                    L1[set_idxL1][j].valid = 1;
                                    L1[set_idxL1][j].dirty = L2[set_idxL2][i].dirty;
                                    break;
                                }
                            }
                            
                            // If L1 is full? - What to do? Evict
                            if (!emptyWayExists)
                            {
                                // Eviction: counterL1[set_idxL1]
                                int to_be_evicted_block_idx = counterL1[set_idxL1];
                                int to_be_copied_dirty = L1[set_idxL1][to_be_evicted_block_idx].dirty;

                                // Update Counter
                                counterL1[set_idxL1] += 1;
                                if (counterL1[set_idxL1] == L1[set_idxL1].size())
                                    counterL1[set_idxL1] == 0;
                                
                                
                                // For address for the block that needs to be evicted
                                string evic_block_addr = "";
                                // evic_block_addr += block_tag + block_idx + block_byte_offset
                                bitset<32> l1_tag_bits = bitset<32>(log2(L1[set_idxL1][to_be_evicted_block_idx].tag));
                                bitset<32> l1_idx_bits = bitset<32>(log2(set_idxL1));
                                bitset<32> l1_block_bits = bitset<32>(log2(cacheconfig.L1blocksize));

                                // TODO : CONVERT TO EVIC BLOCK ADD

                                // This is the loc where the evicted block needs to go in L2
                                int new_tagL2 = (bitset<32>(evic_block_addr.substr(0, tag_bits_l2))).to_ulong();
                                int new_idx_setL2 = (bitset<32>(evic_block_addr.substr(tag_bits_l2, idx_bit_l2))).to_ulong();

                                // cWrite to L2 and copy dirty bit if empty block found
                                bool L2_empty_way_exists = false;
                                for (int j = 0; j < L2[new_idx_setL2].size(); j++)
                                {
                                    if (L2[new_idx_setL2][j].valid == 0)
                                    {
                                        emptyWayExists = true;
                                        L2[new_idx_setL2][j].valid = 1;
                                        L2[new_idx_setL2][j].dirty = to_be_copied_dirty;
                                        L2[new_idx_setL2][j].tag = new_tagL2;
                                    }
                                }

                                // new_idx_setL2 is the current set - we evict from here
                                //  VALIDATE all dirty bit conditions
                                if (!emptyWayExists)
                                {
                                    int l2_to_be_evic_blck = counterL2[new_idx_setL2];

                                    // If L2 evicted bit is dirty : write to main mem (check if l1 is copying)
                                    if ( L2[new_idx_setL2][l2_to_be_evic_blck].dirty == 1)
                                        MemAcceState = WRITEMEM;
                                    
                                    L2[new_idx_setL2][l2_to_be_evic_blck].tag = new_tagL2;
                                    L2[new_idx_setL2][l2_to_be_evic_blck].dirty = to_be_copied_dirty;

                                    // Update counter (after write to mem and/or )
                                    counterL2[new_idx_setL2] += 1;
                                    if (counterL2[new_idx_setL2] == L2[new_idx_setL2].size())
                                        counterL2[new_idx_setL2] == 0;
                                }
                            }
                        }
                        break;
                    }

                    L2AcceState = RM;
                }

                // Both miss - fetch from memoery to L1
                if (L2AcceState == RM)
                {
                    MemAcceState = NOWRITEMEM;

                    // Fetch to L1 - Check ways
                    bool emptyWayExists = false;
                    for (int j = 0; j < L1[set_idxL1].size(); j++)
                    {
                        if (L1[set_idxL1][j].valid == 0)    // Empty way exists
                        {
                            emptyWayExists = true;
                            L1[set_idxL1][j].tag = way_tagL1;                                                       // Question - This takes waytag1
                            L1[set_idxL1][j].valid = 1;
                            break;
                        }
                    }

                    if (!emptyWayExists)
                    {
                        // Step 1: Evict Round robin counter
                        // Step 2: set evicted_block.valid = 1, evicted_bloc.tag = tag
                        // Step 3: Move to L2 - Check L2 full (how? - same as L1)
                        // Step 4: L2 eviction questoons - dirty bit                                                    // Question

                        // Eviction: counterL1[set_idxL1]
                        int to_be_evicted_block_idx = counterL1[set_idxL1];
                        int to_be_copied_dirty = L1[set_idxL1][to_be_evicted_block_idx].dirty;

                        // Update Counter
                        counterL1[set_idxL1] += 1;
                        if (counterL1[set_idxL1] == L1[set_idxL1].size())
                            counterL1[set_idxL1] == 0;
                        
                        
                        // For address for the block that needs to be evicted
                        string evic_block_addr = "";
                        // evic_block_addr += block_tag + block_idx + block_byte_offset
                        bitset<32> l1_tag_bits = bitset<32>(log2(L1[set_idxL1][to_be_evicted_block_idx].tag));
                        bitset<32> l1_idx_bits = bitset<32>(log2(set_idxL1));
                        bitset<32> l1_block_bits = bitset<32>(log2(cacheconfig.L1blocksize));

                        // This is the loc where the evicted block needs to go in L2
                        int new_tagL2 = (bitset<32>(evic_block_addr.substr(0, tag_bits_l2))).to_ulong();
                        int new_idx_setL2 = (bitset<32>(evic_block_addr.substr(tag_bits_l2, idx_bit_l2))).to_ulong();

                        // cWrite to L2 and copy dirty bit if empty block found
                        bool L2_empty_way_exists = false;
                        for (int j = 0; j < L2[new_idx_setL2].size(); j++)
                        {
                            if (L2[new_idx_setL2][j].valid == 0)
                            {
                                emptyWayExists = true;
                                L2[new_idx_setL2][j].valid = 1;
                                L2[new_idx_setL2][j].dirty = to_be_copied_dirty;
                                L2[new_idx_setL2][j].tag = new_tagL2;
                            }
                        }

                        // new_idx_setL2 is the current set - we evict from here
                        //  VALIDATE all dirty bit conditions
                        if (!emptyWayExists)
                        {
                            int l2_to_be_evic_blck = counterL2[new_idx_setL2];

                            // If L2 evicted bit is dirty : write to main mem (check if l1 is copying)
                            if ( L2[new_idx_setL2][l2_to_be_evic_blck].dirty == 1)
                                MemAcceState = WRITEMEM;
                            
                            L2[new_idx_setL2][l2_to_be_evic_blck].tag = new_tagL2;
                            L2[new_idx_setL2][l2_to_be_evic_blck].dirty = to_be_copied_dirty;

                            // Update counter (after write to mem and/or )
                            counterL2[new_idx_setL2] += 1;
                            if (counterL2[new_idx_setL2] == L2[new_idx_setL2].size())
                                counterL2[new_idx_setL2] == 0;
                        }
                    }
                }
            }
            else
            { // a Write request
                // Implement by you:
                //   write access to the L1 Cache, or L2 / main MEM,
                //   update the access state variable;
                //   return: L1AcceState L2AcceState

                // For example:
                // L1AcceState = cache.writeL1(addr);
                // if (L1AcceState == WM){
                //     L2AcceState, MemAcceState = cache.writeL2(addr);
                // }
               
                // else if(){...}

                // Check L1 write
                for (int i = 0; i < L1[set_idxL1].size(); i++)
                {
                    // Check if tag exists - then hit : set sirty bit to 1
                    if (L1[set_idxL1][i].tag == way_tagL1 && L1[set_idxL1][i].valid == 1)
                    {
                        L1[set_idxL1][i].dirty = 1;
                        L1AcceState = WH;
                        L2AcceState = NA;
                        break;
                    }
                    L1AcceState = WM;
                }

                if (L1AcceState == WM)
                {
                    for (int i = 0; i < L2[set_idxL2].size(); i++)
                    {
                        if (L2[set_idxL2][i].tag == way_tagL2 && L2[set_idxL2][i].valid == 1)
                        {
                            L2[set_idxL2][i].dirty = 1;
                            L2AcceState = WH;
                            break;
                        }
                        L2AcceState = WM;
                    }
                }

                if (L2AcceState == WM)
                {
                    MemAcceState = WRITEMEM;
                }
            }
/*********************************** ↑↑↑ Todo: Implement by you ↑↑↑ ******************************************/



            // Grading: don't change the code below.
            // We will print your access state of each cycle to see if your simulator gives the same result as ours.
            tracesout << L1AcceState << " " << L2AcceState << " " << MemAcceState << endl; // Output hit/miss results for L1 and L2 to the output file;
        }
        traces.close();
        tracesout.close();
    }
    else
        cout << "Unable to open trace or traceout file ";

    return 0;
}
