#include<iostream>
#include<string>
#include<vector>
#include<bitset>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

using namespace std;

#define MemSize (65536)

class PhyMem  
{
  public:
    bitset<32> readdata;  
    PhyMem()
    {
      DMem.resize(MemSize); 
      ifstream dmem;
      string line;
      int i=0;
      dmem.open("pt_initialize.txt");
      if (dmem.is_open())
      {
        while (getline(dmem,line))
        {      
          DMem[i] = bitset<8>(line);
          i++;
        }
      }
      else cout<<"Unable to open page table init file";
      dmem.close();

    }  
    bitset<32> getValAtIdx(int addIdx) 
    {    
      string bitsetInstStr = "";
      for (int i = addIdx; i <= addIdx + 3; i++)
      {
          bitsetInstStr += DMem[i].to_string();
      }

      return bitset<32>(bitsetInstStr);    
    }           

  private:
    vector<bitset<8> > DMem;

};  

string get_hex(long val, bool is32bit)
{
  std::stringstream hex_stream;
  if (is32bit)
    hex_stream << "0x" << std::hex << std::setw(8) << std::setfill('0') << val;
  else
    hex_stream << "0x" << std::hex << val;
  std::string val_hex = hex_stream.str();
  return val_hex;
}

int main(int argc, char *argv[])
{
    PhyMem myPhyMem;

    ifstream traces;
    ifstream PTB_file;
    ofstream tracesout;

    string outname;
    outname = "pt_results.txt";

    traces.open(argv[1]);
    PTB_file.open(argv[2]);
    tracesout.open(outname.c_str());

    //Initialize the PTBR
    bitset<12> PTBR;
    PTB_file >> PTBR;

    string line;
    bitset<14> virtualAddr;
    int page_size = 4;

    /*********************************** ↓↓↓ Todo: Implement by you ↓↓↓ ******************************************/

    // Read a virtual address form the PageTable and convert it to the physical address - CSA23
    if(traces.is_open() && tracesout.is_open())
    {
        while (getline(traces, line))
        {
            //TODO: Implement!
            
            virtualAddr = bitset<14>(line);
            bitset<4> opt_bits = bitset<4> (virtualAddr.to_string().substr(0, 4));    // OPT bits in VAddr
            bitset<4> ipt_bits = bitset<4> (virtualAddr.to_string().substr(4, 4));    // IPT bits in Vaddr

            // Access the outer page table 
            int opt_idx = PTBR.to_ulong() + (opt_bits).to_ulong()*page_size;          // idx where opt is
            bitset<32> opt_addr = myPhyMem.getValAtIdx(opt_idx);                      // 32 bits of opt
            if (opt_addr[0] == 0)
            {
              tracesout << "0, 0, 0x000, 0x00000000" << endl;
              continue;
            }
            
            bitset<12> frame = opt_addr.to_ulong() >> 20;                             // iptstart =  12 msb of opt
            int ipt_idx = frame.to_ulong() + (ipt_bits).to_ulong()*page_size;         // ipt_add = ipt_start + 4*(IPT bits in Vaddr)

            // If outer page table valid bit is 1, access the inner page table 
            bitset<32> ipt_data = myPhyMem.getValAtIdx(ipt_idx);                               // 32 bits from ipt_add in DMem
            if (ipt_data[0] == 0)
            {
              tracesout << "1, 0, 0x000, 0x00000000" << endl;
              continue;
            }

            
            string phy_addr = ipt_data.to_string().substr(0, 6);
            phy_addr += virtualAddr.to_string().substr(8, 6);

            bitset<12> phy_add_bits = bitset<12>(phy_addr);
            bitset<32> val_at_mem = myPhyMem.getValAtIdx(phy_add_bits.to_ulong());

            string phy_addr_hex = get_hex(phy_add_bits.to_ulong(), false);
            string val_hex = get_hex(val_at_mem.to_ulong(), true);

            tracesout << "1, 1, " + phy_addr_hex + ", " + val_hex << endl;
            //Return valid bit in outer and inner page table, physical address, and value stored in the physical memory.
            // Each line in the output file for example should be: 1, 0, 0x000, 0x00000000
        }

        traces.close();
        tracesout.close();
    }

    else
        cout << "Unable to open trace or traceout file ";

    return 0;
}
