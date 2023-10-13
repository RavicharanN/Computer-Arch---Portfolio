#include<iostream>
#include<string>
#include<vector>
#include<bitset>
#include<fstream>
using namespace std;

#define MemSize 1000 // memory size, in reality, the memory size should be 2^32, but for this lab csa23, for the space resaon, we keep it as this large number, but the memory is still 32-bit addressable.

#define ADDU (1)
#define SUBU (3)
#define AND (4)
#define OR  (5)
#define NOR (7)

struct IFStruct {
    bitset<32>  PC;
    bool        nop;
    IFStruct() : PC(0), nop(0) {} 
};

struct IDStruct {
    bitset<32>  Instr;
    bool        nop;  
    IDStruct() : Instr(0), nop(1) {}
};

struct EXStruct {
    bitset<32>  Read_data1;
    bitset<32>  Read_data2;
    bitset<16>  Imm;
    bitset<5>   Rs;
    bitset<5>   Rt;
    bitset<5>   Wrt_reg_addr;
    bool        is_I_type;
    bool        rd_mem;
    bool        wrt_mem; 
    bool        alu_op;     //1 for addu, lw, sw, 0 for subu 
    bool        wrt_enable;
    bool        nop;
    EXStruct() : Read_data1(0), Read_data2(0), Imm(0), Rs(0), Rt(0), Wrt_reg_addr(0), is_I_type(0), rd_mem(0), alu_op(0), wrt_enable(0), nop(1) {}
};

struct MEMStruct {
    bitset<32>  ALUresult;
    bitset<32>  Store_data;
    bitset<5>   Rs;
    bitset<5>   Rt;    
    bitset<5>   Wrt_reg_addr;
    bool        rd_mem;
    bool        wrt_mem; 
    bool        wrt_enable;    
    bool        nop;    
    MEMStruct() : ALUresult(0), Store_data(0), Rs(0), Rt(0), Wrt_reg_addr(0), rd_mem(0), wrt_mem(0), wrt_enable(0), nop(1) {}
};

struct WBStruct {
    bitset<32>  Wrt_data;
    bitset<5>   Rs;
    bitset<5>   Rt;     
    bitset<5>   Wrt_reg_addr;
    bool        wrt_enable;
    bool        nop;  
    WBStruct() : Wrt_data(0), Rs(0), Rt(0), Wrt_reg_addr(0), wrt_enable(0), nop(1) {}   
};

struct stateStruct {
    IFStruct    IF;
    IDStruct    ID;
    EXStruct    EX;
    MEMStruct   MEM;
    WBStruct    WB;
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

      int aluOpInt = ALUOP.to_ulong();
      int addRes;
      // Carry out the operation based on ALUOp
      switch (aluOpInt)
      {
        case ADDU: 
        {
          cout << "Add" << endl;
          addRes = oprand1.to_ulong() + oprand2.to_ulong();
          ALUresult = bitset<32>(addRes);
          break;
        }

        case SUBU:
        {
          cout << "Sub" << endl;
          addRes = oprand1.to_ulong() - oprand2.to_ulong();
          ALUresult = bitset<32>(addRes);
          break;
        }

        case AND: 
        {
          cout << "and" << endl;
          ALUresult = oprand1 & oprand2;
          break;
        }
        
        case OR:
        {
          cout << "or" << endl;
          ALUresult = oprand1 | oprand2;
          break;
        }
          
        case NOR:
        {
          cout << "nor" << endl;
          ALUresult = ~(oprand1 | oprand2);
          break;
        }

        default:
          break;
      }

      return ALUresult;
    }            
};

class RF
{
    public: 
        bitset<32> Reg_data;
     	RF()
    	{ 
			Registers.resize(32);  
			Registers[0] = bitset<32> (0);  
        }
	
        bitset<32> readRF(bitset<5> Reg_addr)
        {   
            Reg_data = Registers[Reg_addr.to_ulong()];
            return Reg_data;
        }
    
        void writeRF(bitset<5> Reg_addr, bitset<32> Wrt_reg_data)
        {
            Registers[Reg_addr.to_ulong()] = Wrt_reg_data;
        }
		 
		void outputRF()
		{
			ofstream rfout;
			rfout.open("RFresult.txt",std::ios_base::app);
			if (rfout.is_open())
			{
				rfout<<"State of RF:\t"<<endl;
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

class INSMem
{
	public:
        bitset<32> Instruction;
        INSMem()
        {       
			IMem.resize(MemSize); 
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
                  
		bitset<32> readInstr(bitset<32> ReadAddress) 
		{    
			string insmem;
			insmem.append(IMem[ReadAddress.to_ulong()].to_string());
			insmem.append(IMem[ReadAddress.to_ulong()+1].to_string());
			insmem.append(IMem[ReadAddress.to_ulong()+2].to_string());
			insmem.append(IMem[ReadAddress.to_ulong()+3].to_string());
			Instruction = bitset<32>(insmem);		//read instruction memory
			return Instruction;     
		}     
      
    private:
        vector<bitset<8> > IMem;     
};
      
class DataMem    
{
    public:
        bitset<32> ReadData;  
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
		
        bitset<32> readDataMem(bitset<32> Address)
        {	
			string datamem;
            datamem.append(DMem[Address.to_ulong()].to_string());
            datamem.append(DMem[Address.to_ulong()+1].to_string());
            datamem.append(DMem[Address.to_ulong()+2].to_string());
            datamem.append(DMem[Address.to_ulong()+3].to_string());
            ReadData = bitset<32>(datamem);		//read data memory
            return ReadData;               
		}
            
        void writeDataMem(bitset<32> Address, bitset<32> WriteData)            
        {
            DMem[Address.to_ulong()] = bitset<8>(WriteData.to_string().substr(0,8));
            DMem[Address.to_ulong()+1] = bitset<8>(WriteData.to_string().substr(8,8));
            DMem[Address.to_ulong()+2] = bitset<8>(WriteData.to_string().substr(16,8));
            DMem[Address.to_ulong()+3] = bitset<8>(WriteData.to_string().substr(24,8));  
        }   
                     
        void outputDataMem()
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

void printState(stateStruct state, int cycle)
{
    ofstream printstate;
    printstate.open("stateresult.txt", std::ios_base::app);
    if (printstate.is_open())
    {
        printstate<<"State after executing cycle:\t"<<cycle<<endl; 
        
        printstate<<"IF.PC:\t"<<state.IF.PC.to_ulong()<<endl;        
        printstate<<"IF.nop:\t"<<state.IF.nop<<endl; 
        
        printstate<<"ID.Instr:\t"<<state.ID.Instr<<endl; 
        printstate<<"ID.nop:\t"<<state.ID.nop<<endl;
        
        printstate<<"EX.Read_data1:\t"<<state.EX.Read_data1<<endl;
        printstate<<"EX.Read_data2:\t"<<state.EX.Read_data2<<endl;
        printstate<<"EX.Imm:\t"<<state.EX.Imm<<endl; 
        printstate<<"EX.Rs:\t"<<state.EX.Rs<<endl;
        printstate<<"EX.Rt:\t"<<state.EX.Rt<<endl;
        printstate<<"EX.Wrt_reg_addr:\t"<<state.EX.Wrt_reg_addr<<endl;
        printstate<<"EX.is_I_type:\t"<<state.EX.is_I_type<<endl; 
        printstate<<"EX.rd_mem:\t"<<state.EX.rd_mem<<endl;
        printstate<<"EX.wrt_mem:\t"<<state.EX.wrt_mem<<endl;        
        printstate<<"EX.alu_op:\t"<<state.EX.alu_op<<endl;
        printstate<<"EX.wrt_enable:\t"<<state.EX.wrt_enable<<endl;
        printstate<<"EX.nop:\t"<<state.EX.nop<<endl;        

        printstate<<"MEM.ALUresult:\t"<<state.MEM.ALUresult<<endl;
        printstate<<"MEM.Store_data:\t"<<state.MEM.Store_data<<endl; 
        printstate<<"MEM.Rs:\t"<<state.MEM.Rs<<endl;
        printstate<<"MEM.Rt:\t"<<state.MEM.Rt<<endl;   
        printstate<<"MEM.Wrt_reg_addr:\t"<<state.MEM.Wrt_reg_addr<<endl;              
        printstate<<"MEM.rd_mem:\t"<<state.MEM.rd_mem<<endl;
        printstate<<"MEM.wrt_mem:\t"<<state.MEM.wrt_mem<<endl; 
        printstate<<"MEM.wrt_enable:\t"<<state.MEM.wrt_enable<<endl;         
        printstate<<"MEM.nop:\t"<<state.MEM.nop<<endl;        

        printstate<<"WB.Wrt_data:\t"<<state.WB.Wrt_data<<endl;
        printstate<<"WB.Rs:\t"<<state.WB.Rs<<endl;
        printstate<<"WB.Rt:\t"<<state.WB.Rt<<endl;        
        printstate<<"WB.Wrt_reg_addr:\t"<<state.WB.Wrt_reg_addr<<endl;
        printstate<<"WB.wrt_enable:\t"<<state.WB.wrt_enable<<endl;        
        printstate<<"WB.nop:\t"<<state.WB.nop<<endl; 
    }
    else cout<<"Unable to open file";
    printstate.close();
}

unsigned long shiftbits(bitset<32> inst, int start)
{
    unsigned long ulonginst;
    return ((inst.to_ulong()) >> start);
}

bitset<32> getSignExtendImmed(string immediate)
{
  char signBit = immediate[0];
  if (signBit == '0')
  {
    return bitset<32>("0000000000000000" + immediate);
  }

  return bitset<32>("1111111111111111" + immediate);
}
 

int main()
{   
    RF myRF;
    INSMem myInsMem;
    DataMem myDataMem;
    ALU myALU;
    stateStruct state, nextState;
    int cycle = 0;
	
    bitset<32> PC;
    bitset<32> branchAddress = bitset<32>(0);
    bool isStall = false;
    bool isBranch = false;

    while (1) {
       cout << "ASDSAda" << endl;
        /* --------------------- WB stage --------------------- */
        if (!state.WB.nop)
        {
            cout << cycle << "IF";
            if (state.WB.wrt_enable)   // LW instruction is written back here to RF
                myRF.writeRF(state.WB.Wrt_reg_addr, state.WB.Wrt_data);
        }

        /* --------------------- MEM stage --------------------- */
        if (state.MEM.nop)
        {
            nextState.WB.nop = true;
        }
        if (!state.MEM.nop)
        {
            nextState.WB.nop = false;
            cout << cycle << "MEM";
            nextState.WB.Wrt_data = state.MEM.ALUresult;

            if (state.MEM.wrt_mem) // sw, no need to update write_data
            {
                // Handle MEM - MEM forwarding (CHECK THIS)
                if (!state.WB.nop && (state.MEM.Rt == state.WB.Wrt_reg_addr) && state.WB.wrt_enable)
                    state.MEM.Store_data = state.WB.Wrt_data;

                myDataMem.writeDataMem(state.MEM.ALUresult, state.MEM.Store_data);
            }
            else if (state.MEM.rd_mem)
                nextState.WB.Wrt_data = myDataMem.readDataMem(state.MEM.ALUresult);

            nextState.WB.Rs = state.MEM.Rs;
            nextState.WB.Rt = state.MEM.Rt;
            nextState.WB.Wrt_reg_addr = state.MEM.Wrt_reg_addr;
            nextState.WB.wrt_enable = state.MEM.wrt_enable;
        }

        /* --------------------- EX stage --------------------- */
        if (state.EX.nop)
        {
            nextState.MEM.nop = true;
        }
        if (!state.EX.nop)
        {
            nextState.MEM.nop = false;
            cout << cycle << "EX";
            // We only make use the ALU operand vals from the decode step and forward the result to mem 
            bitset<32> op1 = state.EX.Read_data1;
            bitset<32> op2 = (state.EX.is_I_type ? getSignExtendImmed(state.EX.Imm.to_string()) : state.EX.Read_data2);
            // NOTE: (In the ABOVE LINE) : Both lw and sw use (rs + signExtImm) as inputs, so use that as op2

            nextState.MEM.Store_data = state.EX.Read_data2;         // Only relevant for sw

            // Handle EX EX forwarding
            if (state.MEM.wrt_enable)    // if register file gets upadted, handle this in the first half; 
            {
                // EX - EX forwarding, previous rs is used for 
                if (state.EX.Rs == state.MEM.Wrt_reg_addr)
                    op1 = state.MEM.ALUresult;
                
                if (state.EX.Rt == state.MEM.Wrt_reg_addr && !state.EX.wrt_mem && !state.EX.rd_mem)
                    op2 = state.MEM.ALUresult;
            }

            // Handle MEM - EX forwarding
            if (state.WB.wrt_enable)  // (CHECK THIS - Handle tests) 
            {
                if (state.EX.Rs == state.WB.Wrt_reg_addr)
                    op1 = state.WB.Wrt_data;
                
                if (state.EX.Rt == state.WB.Wrt_reg_addr && !state.EX.is_I_type)
                    op2 = state.WB.Wrt_data;
            }

            if (state.EX.alu_op)
                nextState.MEM.ALUresult = op1.to_ulong() + op2.to_ulong();
            else 
                nextState.MEM.ALUresult = op1.to_ulong() - op2.to_ulong();

            //  Directly forward to mem
            if (state.EX.wrt_mem && state.EX.Rt == state.WB.Wrt_reg_addr)
                nextState.MEM.Store_data = state.WB.Wrt_data;

            // All these flags set in EX will be carried forward
            nextState.MEM.Rs = state.EX.Rs;
            nextState.MEM.Rt = state.EX.Rt;
            nextState.MEM.Wrt_reg_addr = state.EX.Wrt_reg_addr;
            nextState.MEM.wrt_enable = state.EX.wrt_enable;
            nextState.MEM.rd_mem = state.EX.rd_mem;
            nextState.MEM.wrt_mem = state.EX.wrt_mem;
        }
          
        /* --------------------- ID stage --------------------- */
        if (state.ID.nop)
        {
            nextState.EX.nop = true;
        }
        else
        {
            nextState.EX.nop = false;
            // cout << cycle << "ID";
            bitset<32> instruction = state.ID.Instr;
            bitset<6> opcode = bitset<6>((instruction.to_string().substr(0, 6)));
            bitset<5> rsAddress  = bitset<5>(instruction.to_string().substr(6, 5));   // op1 
            bitset<5> rtAddress  = bitset<5>(instruction.to_string().substr(11, 5));  // op2
            bitset<5> rdAddress  = bitset<5>(instruction.to_string().substr(16, 5));  // rd <- rs + rt
            bitset<6> aluOp = bitset<6>(instruction.to_string().substr(26, 6));
            bitset<16> imm = bitset<16>((instruction.to_string().substr(16, 16)));

            // Block to valeus for the EX stage.
            nextState.EX.Rs = rsAddress;
            nextState.EX.Rt = rtAddress;
            nextState.EX.Read_data1 = myRF.readRF(nextState.EX.Rs);
            nextState.EX.Read_data2 = myRF.readRF(nextState.EX.Rt);
            nextState.EX.Imm = imm;

            // if (!opcode.any()) :  R-Type 
            // else : Itype set to truwe
            nextState.EX.is_I_type = opcode != 0 && opcode != 2; // Is I type if its not R type or J Type
            nextState.EX.alu_op = 1; //  If aluop is 3 (SUBU) then val is 0, 1 for the rest
            nextState.EX.rd_mem = (opcode.to_ulong() == 35);
            nextState.EX.wrt_mem = (opcode.to_ulong() == 43);
            nextState.EX.wrt_enable = false;

            // =======
            if (nextState.EX.is_I_type)  // I Type 
            {
                bitset<16> immediate = bitset<16>(instruction.to_string().substr(16, 16));
                bitset<32> signExtendImmed =  getSignExtendImmed(immediate.to_string());
                nextState.EX.Imm = immediate;
                nextState.EX.Wrt_reg_addr = nextState.EX.Rt; // Rt
                if (opcode == 0x23)                          // lw,
                    nextState.EX.wrt_enable = true;
                if (opcode == 0x04) // beq in this lab, treat it as bne
                    if (nextState.EX.Read_data2 != nextState.EX.Read_data1)
                    {
                        isBranch = true;
                        bitset<32> newBranchAdd = PC.to_ulong() + 4; 
                        branchAddress = newBranchAdd.to_ulong() + (signExtendImmed.to_ulong() << 2);
                        // remove + 4 is very important
                        // bitset<32> signext = getSignExtendImmed(nextState.EX.Imm.to_string());
                        // branchAddress = bitset<32>(
                        //     state.IF.PC.to_ulong() + (bitset<32>((bitset<30>(shiftbits(signext, 0))).to_string<char, std::string::traits_type, std::string::allocator_type>() + "00")).to_ulong());
                    }
            }
            else                // R Type
            {
                nextState.EX.Wrt_reg_addr = rdAddress;
                nextState.EX.alu_op = aluOp == 0x23 ? 0 : 1;
                nextState.EX.wrt_enable = true;
                

                // if (opcode.to_ulong() == 35) // LW
                // {
                //     nextState.EX.Wrt_reg_addr = rtAddress;   // CHECK THIS -----------
                //     nextState.EX.wrt_enable = true;
                //     nextState.EX.rd_mem = true;
                //     nextState.EX.wrt_mem = false;
                // }
                // else if (opcode.to_ulong() == 43) // SW 
                // {
                //     nextState.EX.wrt_enable = false;
                //     nextState.EX.rd_mem = false;
                //     nextState.EX.wrt_mem = true;
                // }
                // else if (opcode.to_ulong() == 4) // treating beq it as bne
                // {
                //     if (nextState.EX.Read_data2 != nextState.EX.Read_data1)
                //     {
                //         bitset<32> signExtImm = getSignExtendImmed(immediate.to_string());
                //         bitset<32> newBranchAdd = PC.to_ulong() + 4; 
                //         newBranchAdd = newBranchAdd.to_ulong() + (signExtendImmed.to_ulong() << 2);
                //         nextState.IF.PC = newBranchAdd;
                //         nextState.ID = state.ID;
                //         nextState.ID.nop = true;
                //         nextState.EX.nop = true;
                //     }
                // }
                
            }
            // NOT MY CODE
            if (state.EX.wrt_enable) // check if stall is needed
            {
                // when lw, sw, rt is to be forwarded; when r type, rs, rt both to be checked
                if (!state.EX.nop && (state.EX.Wrt_reg_addr == nextState.EX.Rs || (!nextState.EX.is_I_type && state.EX.Wrt_reg_addr == nextState.EX.Rt)))
                    isStall = true;
            }
            // NOTE MY CODE

            // TODO :  HANDLE STALLSSSSSS -------------
            // if (!state.EX.nop && state.EX.wrt_enable)
            // {
            //     if ((state.EX.Wrt_reg_addr == rsAddress) ||
            //      ((rtAddress == state.EX.Wrt_reg_addr) && !state.EX.is_I_type))
            //     {
            //         nextState.IF = state.IF;
            //         nextState.ID = state.ID;
            //         nextState.EX.nop = true;
            //     }
            // }
        }
        
        /* --------------------- IF stage --------------------- */
        // if (!state.IF.nop)
        // {
        //     cout << cycle << "IF";
        //     nextState.ID.nop = false;
        //     nextState.ID.Instr = myInsMem.readInstr(state.IF.PC);
        //     nextState.IF.PC = state.IF.PC.to_ullong() + 4;
             
        // }
        // else
        //     nextState.ID.nop = true;
        
        // -------- NOT MY CODE ----- 
        if (state.IF.nop)
        {
            nextState.ID.nop = true;
        }
        else
        {
            nextState.ID.nop = false;
            nextState.ID.Instr = myInsMem.readInstr(state.IF.PC);
            nextState.IF.PC = state.IF.PC.to_ulong() + 4;
        }
        // ------- NOT MY CODE -----
        
        if (isBranch)
        {
            cout << "Branch from " << state.IF.PC.to_ulong() << " to " << branchAddress.to_ulong() << endl;
            isBranch = false;
            nextState.IF.PC = branchAddress;
            nextState.ID = state.ID;
            nextState.ID.nop = true;
            nextState.EX.nop = true;
        }

        // ----- NOT MY CODE
        if (isStall)
        {
            cout << "Stalled" << endl;
            isStall = false;
            nextState.IF = state.IF;
            nextState.ID = state.ID;
            nextState.EX.nop = true;
        }
        // ----- NOT MY CODE
        
        if (nextState.ID.Instr.all())
        {
            nextState.IF.nop = true;
            nextState.ID.nop = true;
            nextState.IF.PC = state.IF.PC;
        }

        /* ------------------- END OF IF stage ------------------*/
             
        if (state.IF.nop && state.ID.nop && state.EX.nop && state.MEM.nop && state.WB.nop)
            break;
        
        printState(nextState, cycle); //print states after executing cycle 0, cycle 1, cycle 2 ... 
        cycle++;
       
        state = nextState; /*** The end of the cycle and updates the current state with the values calculated in this cycle. csa23 ***/ 

    }
    
    myRF.outputRF(); // dump RF;	
	myDataMem.outputDataMem(); // dump data mem 
	
	return 0;
}
