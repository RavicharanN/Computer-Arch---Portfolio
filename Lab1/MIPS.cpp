#include<iostream>
#include<string>
#include<vector>
#include<bitset>
#include<fstream>

using namespace std;

#define ADDU (1)
#define SUBU (3)
#define AND (4)
#define OR  (5)
#define NOR (7)

// Memory size.
// In reality, the memory size should be 2^32, but for this lab and space reasons,
// we keep it as this large number, but the memory is still 32-bit addressable.
#define MemSize (65536)

class RF
{
  public:
    bitset<32> ReadData1, ReadData2; 
    RF()
    { 
      Registers.resize(32);  
      Registers[0] = bitset<32> (0);  
    }

    void ReadWrite(bitset<5> RdReg1, bitset<5> RdReg2, bitset<5> WrtReg, bitset<32> WrtData, bitset<1> WrtEnable)
    {   
      /**
       * @brief Reads or writes data from/to the Register.
       *
       * This function is used to read or write data from/to the register, depending on the value of WrtEnable.
       * Put the read results to the ReadData1 and ReadData2.
       */
      // TODO: implement!
      int rdRegIdx1 =  RdReg1.to_ulong();  // Index of read register 1 in the 32 registerArr
      int rdRegIdx2 =  RdReg2.to_ulong();  // Index of read register 2 in the 32 registerArr
      int wrtRegIdx =  WrtReg.to_ulong();  // Index of write register

      ReadData1 = Registers[rdRegIdx1];
      ReadData2 = Registers[rdRegIdx2];
      // If write enable is set, writeData to the writeRegister
      if (WrtEnable.all())
      {
        Registers[wrtRegIdx] = WrtData;
      }
    }

    void OutputRF()
    {
      ofstream rfout;
      rfout.open("RFresult.txt",std::ios_base::app);
      if (rfout.is_open())
      {
        rfout<<"A state of RF:"<<endl;
        for (int j = 0; j<32; j++)
        {        
          rfout << Registers[j]<<endl;
        }

      }
      else cout<<"Unable to open file";
      rfout.close();

    }     
  private:
    vector<bitset<32> >Registers;
};


class ALU
{
  public:
    bitset<32> ALUresult;
    bitset<32> ALUOperation (bitset<3> ALUOP, bitset<32> oprand1, bitset<32> oprand2)
    {   
      /**
       * @brief Implement the ALU operation here.
       *
       * ALU operation depends on the ALUOP, which are definded as ADDU, SUBU, etc. 
       */
      // TODO: implement!

      int aluOpInt = ALUOP.to_ullong();
      int addRes;
      // Carry out the operation based on ALUOp
      switch (aluOpInt)
      {
        case ADDU: 
        {
          addRes = oprand1.to_ulong() + oprand2.to_ulong();
          ALUresult = bitset<32>(addRes);
          break;
        }
          

        case SUBU:
        {
          addRes = oprand1.to_ulong() - oprand2.to_ulong();
          ALUresult = bitset<32>(addRes);
          break;
        }
          

        case AND: 
        {
          ALUresult = oprand1 & oprand2;
          break;
        }
        
        case OR:
        {
          ALUresult = oprand1 | oprand2;
          break;
        }
          
        case NOR:
        {
          ALUresult = ~(oprand1 | oprand2);
          break;
        }
          

        default:
          cout << "Instruction not defined!";
          break;
      }

      return ALUresult;
    }            
};


class INSMem
{
  public:
    bitset<32> Instruction;
    INSMem()
    {       IMem.resize(MemSize); 
      ifstream imem;
      string line;
      int i=0;
      imem.open("imem.txt");
      if (imem.is_open())
      {
        while (getline(imem,line))
        {      
          IMem[i] = bitset<8>(line);
          i++;
        }

      }
      else cout<<"Unable to open file";
      imem.close();

    }

    bitset<32> ReadMemory (bitset<32> ReadAddress) 
    {    
      // TODO: implement!
      /**
       * @brief Read Instruction Memory (IMem).
       *
       * Read the byte at the ReadAddress and the following three byte,
       * and return the read result. 
       */

      int byteIdx = ReadAddress.to_ulong();  // Give the byte index of the intrsudction

      // Concat the current 8 bits (at byteIdx) and the consecutive 26 bits (3 bytes) to get the instruction
      string bitsetInstStr = "";
      for (int i = byteIdx; i < byteIdx + 3; i++)
      {
          bitsetInstStr += IMem[byteIdx].to_string();
      }

      Instruction = bitset<32>(bitsetInstStr);
      return Instruction;     
    }     

  private:
    vector<bitset<8> > IMem;
};

class DataMem    
{
  public:
    bitset<32> readdata;  
    DataMem()
    {
      DMem.resize(MemSize); 
      ifstream dmem;
      string line;
      int i=0;
      dmem.open("dmem.txt");
      if (dmem.is_open())
      {
        while (getline(dmem,line))
        {      
          DMem[i] = bitset<8>(line);
          i++;
        }
      }
      else cout<<"Unable to open file";
      dmem.close();

    }  
    bitset<32> MemoryAccess (bitset<32> Address, bitset<32> WriteData, bitset<1> readmem, bitset<1> writemem) 
    {    
      /**
       * @brief Reads/writes data from/to the Data Memory.
       *
       * This function is used to read/write data from/to the DataMem, depending on the readmem and writemem.
       * First, if writemem enabled, WriteData should be written to DMem, clear or ignore the return value readdata,
       * and note that 32-bit - will occupy 4 continious Bytes in DMem. 
       * If readmem enabled, return the DMem read result as readdata.
       */
      // TODO: implement!

      int addIdx = Address.to_ulong();
      if (readmem.any())
      {
        string bitsetInstStr = "";
        for (int i = addIdx; i < addIdx + 3; i++)
        {
            bitsetInstStr += DMem[addIdx + i].to_string();
        }

        return readdata = bitset<32>(bitsetInstStr);
      }

      if (writemem.any())
      {
        // Clear the read data when write is enabled
        readdata.reset();

        // Extract each byte out from the 32 bit data and write to DMemory
        for (int i = 0; i < 4; i++)
        {
          string byteStr = (WriteData >> (3-i)*8).to_string().substr(24, 8);
          DMem[addIdx + i] = bitset<8>(byteStr);
        }
      }

      return readdata;     
    }   

    void OutputDataMem()
    {
      ofstream dmemout;
      dmemout.open("dmemresult.txt");
      if (dmemout.is_open())
      {
        for (int j = 0; j< 1000; j++)
        {     
          dmemout << DMem[j]<<endl;
        }

      }
      else cout<<"Unable to open file";
      dmemout.close();

    }             

  private:
    vector<bitset<8> > DMem;

};  



int main()
{
  RF myRF;
  ALU myALU;
  INSMem myInsMem;
  DataMem myDataMem;

  bitset<32> PC;

  while (1)  // TODO: implement!
  {
    // Fetch: fetch an instruction from myInsMem.
    // decode(Read RF): get opcode and other signals from instruction, decode instruction
    // Execute: after decoding, ALU may run and return result
    // Read/Write Mem: access data memory (myDataMem)
    // Write back to RF: some operations may write things to RF
    // ================= END OF BOILERPLATE =========================================

    bitset<32> instruction = myInsMem.ReadMemory(PC);
    bitset<5> opcode = bitset<5>(instruction.to_string().substr(0, 6));
    bool dontUpdatePC = false; // Set to true for branches and jumps

    // If current instruction is "11111111111111111111111111111111", then break; (exit the while loop)
    if (instruction.all())
      break;

    if (!opcode.any())                            // R Type
    {
      bitset<5> rsAddress  = bitset<5>(instruction.to_string().substr(6, 5));   // op1 
      bitset<5> rtAddress  = bitset<5>(instruction.to_string().substr(11, 5));  // op2
      bitset<5> rdAddress  = bitset<5>(instruction.to_string().substr(16, 5));  // rd <- rs + rt
      bitset<3> aluOp = bitset<3>(instruction.to_string().substr(29, 3));

      myRF.ReadWrite(rsAddress, rtAddress, rdAddress, 0, 0);                    // Get the values at rs & rt for computation 
      bitset<32> reg1 = myRF.ReadData1;
      bitset<32> reg2 = myRF.ReadData2;                                         // Feed these values to ALU
      
      myALU.ALUOperation(aluOp, reg1, reg2);
      bitset<32> writeData = myALU.ALUresult;
      myRF.ReadWrite(rsAddress, 0, rdAddress, writeData, 1);                    // Carry out the ALU operation and feed the res to RF
    }
    else if (opcode == 00010 || opcode == 00011)  // J Type
    {
      if (opcode == 00010)
      {
        bitset<26> jumpAddress = bitset<26>(instruction.to_string().substr(6, 26));
        PC = (((PC.to_ulong() + 4) & 0xf0000000)) + (jumpAddress.to_ulong() << 2);
        dontUpdatePC = true;
      }
    }
    else if (opcode != 00010 && opcode != 00011)  // I Type <- not R and not J                                  
    {
      bitset<5> rsAddress  = bitset<5>(instruction.to_string().substr(6, 5));       // op1 
      bitset<5> rtAddress  = bitset<5>(instruction.to_string().substr(11, 5));      // This becomes the write for I type
      bitset<16> immediate = bitset<16>(instruction.to_string().substr(16, 16));
      bitset<32> signExtImmediate = bitset<32>(immediate.to_ulong() & 0xFFFFFFFF);  // Sign extend immediate is used as the second op for I Type
      
      myRF.ReadWrite(rsAddress, 0, rtAddress, 0, 0);
      bitset<32> op1 = myRF.ReadData1;                                              // op1 here is rs
      bitset<32> op2 = myRF.ReadData2;

      switch (opcode.to_ulong())
      {
        case 9: // 001001 addiu
        {
          myRF.ReadWrite(rsAddress, 0, 0, 0, 0);
          bitset<32> op1 = myRF.ReadData1;                                          // RS is the first operand for I Type
          myALU.ALUOperation(ADDU, op1, signExtImmediate);
          myRF.ReadWrite(rsAddress, 0, rtAddress, myALU.ALUresult, 1);              // Write as R[rt] <- R[rs] + signExtImm
          break;
        }
        
        case 35: // 100011 lw
        {
          bitset<32> memoryAddress = bitset<32>(op1.to_ulong() + signExtImmediate.to_ulong());
          myDataMem.MemoryAccess(memoryAddress, 0, 1, 0);                           // M[R[rs] + signExtImmediate] 
          myRF.ReadWrite(0, 0, rtAddress, myDataMem.readdata, 1);                   // Value extracted in the line above is written to Rt 
          break;
        }
        
        case 43: // 101011 sw
        {
          bitset<32> memoryAddress = bitset<32>(op1.to_ulong() + signExtImmediate.to_ulong());
          myDataMem.MemoryAccess(memoryAddress, op2, 0, 1);                         // M[R[rs] + signExtImmediate] = R[rt]
          break;
        }
        
        case 4: // 000100 beq
        {
          if (op1 ==  op2)
          {
            PC = ((PC.to_ulong() + 4) & 0xf0000000) + (signExtImmediate.to_ulong() << 2);
            dontUpdatePC = true;
          }
          break;
        }

        default:
          break;

      }
    }
    
    if (!dontUpdatePC)
    {
      PC = PC.to_ulong() + 4;                                                         // Update program counter by 4
    }
    

    /**** You don't need to modify the following lines. ****/
    myRF.OutputRF(); // dump RF;    
  }
  myDataMem.OutputDataMem(); // dump data mem

  return 0;
}
