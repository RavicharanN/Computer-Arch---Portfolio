#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <assert.h>
#include <limits>

using std::cout;
using std::endl;
using std::string;
using std::vector;
using std::unordered_map;

string inputtracename = "trace.txt";
string outputtracename = inputtracename.substr(0, inputtracename.length() - 4) + ".out.txt";
string hardwareconfigname = "config.txt";

enum Operation
{
	OP_ADD,
	OP_SUB,
	OP_MULT,
	OP_DIV,
	OP_LOAD,
	OP_STORE
};

enum RSType
{
	LOAD,
	STORE,
	ADD,
	MULT,
};

enum State
{
	NA,
	Issued,
	Executing,
	Complete,
	Written
};

RSType getRSTypeForOp(Operation op)
{
	if (op == OP_LOAD)
		return LOAD;
	
	if (op == OP_STORE)
		return STORE;
	
	if (op == OP_ADD || op == OP_SUB)
		return ADD;
	
	return MULT;
}

string getRSTypeString(RSType type)
{
	if (type == LOAD)
		return "Load";
	
	if (type == STORE)
		return "Store";
	
	if (type == ADD)
		return "Add";
	
	return "Mult";
}

// The execute cycle of each operation: ADD, SUB, MULT, DIV, LOAD, STORE
const int OperationCycle[6] = {2, 2, 10, 40, 2, 2};

struct HardwareConfig
{
	int LoadRSsize;	 // number of load reservation stations
	int StoreRSsize; // number of store reservation stations
	int AddRSsize;	 // number of add reservation stations
	int MultRSsize;	 // number of multiply reservation stations
	int FRegSize;	 // number of fp registers
};

// We use the following structure to record the time of each instruction
struct InstructionStatus
{
	int cycleIssued;								// When placed in RS (might still be waiting on thigs to be availalbe)
	int cycleExecuted; 								// execution completed
	int cycleWriteResult;							// Push to CDB (can wawit here if multiple instrctions finish at the same time)
	int curCycle;
};

struct Instruction
{
	Operation op;
	RSType rsType;
	string rs_key;
	int dest_idx;
	int s1_idx;
	int s2_idx;
	State state;
	int run_time;				// Init when reading based on op
	int cur_rs_idx;
};

// Register Result Status structure
struct RegisterResultStatus
{
	string ReservationStationName;
	bool dataReady;
};

/*********************************** ↓↓↓ Todo: Implement by you ↓↓↓ ******************************************/

class RegisterResultStatuses
{
	public:
		vector<RegisterResultStatus> _registers;
		RegisterResultStatuses(int numFpRegs)
		{
			_registers.resize(numFpRegs);
			for (int i = 0; i < _registers.size(); i++)
			{
				_registers[i].ReservationStationName = "";
				_registers[i].dataReady = false;
			}
		}

		string _printRegisterResultStatus() const
		{
			std::ostringstream result;
			for (int idx = 0; idx < _registers.size(); idx++)
			{
				result << "F" + std::to_string(idx) << ": ";
				result << _registers[idx].ReservationStationName << ", ";
				result << "dataRdy: " << (_registers[idx].dataReady ? "Y" : "N") << ", ";
				result << "\n";
			}
			return result.str();
		}

	private:
		
};

// Define your Reservation Station structure
struct ReservationStation
{
	RSType type;
	Operation op;
	bool busy;
	int fp_reg_idx;
	std::vector<int> val;
	vector<string> q;
};
class ReservationStations
{
	public:
		// {"ld": [], "st": [], "add": [], "mult" : []}
		unordered_map<string, vector  <ReservationStation> > _stations;

		ReservationStations(HardwareConfig config)
		{
			InitRSByType(config.LoadRSsize, RSType::LOAD);				// Init Load Buffers
			InitRSByType(config.StoreRSsize, RSType::STORE);			// Init Store Buffers
			InitRSByType(config.AddRSsize, RSType::ADD);				// Init Add Stations
			InitRSByType(config.MultRSsize, RSType::MULT);				// Init Mult Stations
		}

	private:
		void InitRSByType(int size, RSType type)
		{
			vector<ReservationStation> rs_vec;
			string key = getRSTypeString(type);
			for (int i  = 0; i < size; i++)
			{
				ReservationStation rs;	
				rs.busy = false;
				rs.type = type;
				rs.fp_reg_idx = -1;

				if (key == "Add" || key == "Mult")
				{
					int cnt = 2;
					while(cnt--)
					{
						rs.val.push_back(0);
						rs.q.push_back("");
					}
					
				}

				rs_vec.push_back(rs);
			}

			_stations[key] = rs_vec;
		}
};

bool canFetch(vector<Instruction>& fetched_inst)
{
	// Fetch is no istructions are fetched 
	// or the last instruction is issued (no na)
	if (fetched_inst.size() == 0)
		return true;
	
	Instruction lastInst = fetched_inst[fetched_inst.size() - 1];
	return lastInst.state != State::NA;
}

// This updates RS and RRS
int populate_stations(ReservationStations& rs, RSType rsType, int src1, int src2, int dest, RegisterResultStatuses& rrs)
{
	string key = getRSTypeString(rsType);
	for (int i = 0; i < rs._stations[key].size(); i++)
	{
		if	(rs._stations[key][i].busy == false)
		{
			rs._stations[key][i].busy = true;						// ---- DOUBLE CHECLK THIS LOGIC FOR STORE !!!!!!!!!!!!
			if (rsType == RSType::LOAD || rsType == RSType::STORE)
				rs._stations[key][i].busy = true;					//  Redundant code this block is not needed	
			else
			{
				// Check source 1 is free
				RegisterResultStatus reg_stat_s1 = rrs._registers[src1];
				RegisterResultStatus reg_stat_s2 = rrs._registers[src2];

				// This condition means it should be waiting
				if ((reg_stat_s1.ReservationStationName != "" && !reg_stat_s1.dataReady))				// Recheck once !! - mostly right
				{
					rs._stations[key][i].q[0] = rrs._registers[src1].ReservationStationName;
				}
				else
					rs._stations[key][i].val[0] = 1;
				
				// Check source 2 is free
				if ((reg_stat_s2.ReservationStationName != "" && !reg_stat_s2.dataReady) )
					rs._stations[key][i].q[1] = rrs._registers[src2].ReservationStationName;
				else
					rs._stations[key][i].val[1] = 1;
			}			

			// Update RRS -  Avoid updatig for store
			if (rsType != RSType::STORE)
			{
				rrs._registers[dest].dataReady = false;
				rrs._registers[dest].ReservationStationName = key + std::to_string(i);
			}
			
			return i;
		}
	}

	return -1;
}


void call_cdb(ReservationStations& rs, RegisterResultStatuses& rrs, int dest, string rs_name)
{
	// Update rwegister status
	if (rrs._registers[dest].ReservationStationName == rs_name)
	{
		rrs._registers[dest].dataReady = true;
	}
	
	// Update res stations wherever needed
	vector<string> keys;
	keys.push_back("Add");
	keys.push_back("Mult");

	for (string key : keys)
	{
		for (int i = 0; i < rs._stations[key].size(); i++)
		{
			if (rs._stations[key][i].q[0] == rs_name)
				rs._stations[key][i].q[0] = "";
			
			if (rs._stations[key][i].q[1] == rs_name)
				rs._stations[key][i].q[1] = "";
		}
	}
	
}

void printStuff(RegisterResultStatuses& rrs, vector<Instruction>& all_inst, vector<InstructionStatus>& all_inst_status, ReservationStations& rs)
{
	// Instructions and status
	cout << "Inst and statuses \n";
	for (int i = 0; i < all_inst_status.size(); i++)
	{
		cout << getRSTypeString(all_inst[i].rsType) << " " << all_inst[i].rs_key << " " << all_inst[i].op << endl;
		cout << "D, S1, S2 " << all_inst[i].dest_idx  << " " << all_inst[i].s1_idx  << " " << all_inst[i].s2_idx  << "\n";
		cout << all_inst[i].run_time << " " << all_inst_status[i].curCycle << all_inst_status[i].cycleIssued << all_inst_status[i].cycleExecuted << all_inst_status[i].cycleWriteResult << endl;
		cout << "======================= \n";
	}

	cout << "\n \n";
	cout << "RRS";
	rrs._printRegisterResultStatus();
}

/*
print the instruction status, the reservation stations and the register result status
@param filename: output file name
@param instructionStatus: instruction status
*/
void PrintResult4Grade(const string &filename, const vector<InstructionStatus> &instructionStatus)
{
	std::ofstream outfile(filename, std::ios_base::app); // append result to the end of file
	outfile << "Instruction Status:\n";
	for (int idx = 0; idx < instructionStatus.size(); idx++)
	{
		outfile << "Instr" << idx << ": ";
		outfile << "Issued: " << instructionStatus[idx].cycleIssued << ", ";
		outfile << "Completed: " << instructionStatus[idx].cycleExecuted << ", ";
		outfile << "Write Result: " << instructionStatus[idx].cycleWriteResult << ", ";
		outfile << "\n";
	}
	outfile.close();
}

/*
print the register result status each 5 cycles
@param filename: output file name
@param registerResultStatus: register result status
@param thiscycle: current cycle
*/
void PrintRegisterResultStatus4Grade(const string &filename,
									 const RegisterResultStatuses &registerResultStatus,
									 const int thiscycle)
{
	if (thiscycle % 5 != 0)
		return;
	std::ofstream outfile(filename, std::ios_base::app); // append result to the end of file
	outfile << "Cycle " << thiscycle << ":\n";
	outfile << registerResultStatus._printRegisterResultStatus() << "\n";
	outfile.close();
}

// Function to simulate the Tomasulo algorithm
void simulateTomasulo(RegisterResultStatuses& rrs, vector<Instruction>& all_inst, vector<InstructionStatus>& all_inst_status, ReservationStations& rs)
{
	int thiscycle = 1; // start cycle: 1
	vector<Instruction> fetched_inst;

	while (thiscycle < 100000000)
	{
		// Fetch new instruction if its possible
		if ((fetched_inst.size() < all_inst.size()) && canFetch(fetched_inst))
		{
			int idx_to_fetch = fetched_inst.size();
			fetched_inst.push_back(all_inst[idx_to_fetch]);
		}
	
		bool inst_pending = false;
		bool cdb_free = true;

		// Loop through all the fetch_instructions and update states, RSs
		for (int i  = 0; i < fetched_inst.size(); i++)
		{
			if (fetched_inst[i].state == State::Written)
				continue;
			
			inst_pending = true;

			// Complete comes first because this ensures the first instruction gets CDB
			if (fetched_inst[i].state == State::Complete)
			{
				// Check if CDB is free this cycle
				// ---- If free--------
				if (cdb_free)
				{
					cdb_free = false;

					all_inst_status[i].cycleWriteResult = thiscycle;
					fetched_inst[i].state = State::Written;										// Change state to written
					
					// If store nothing is waiting
					// if load 
					string rs_name = fetched_inst[i].rs_key + std::to_string(fetched_inst[i].cur_rs_idx);
	
					call_cdb(rs, rrs, fetched_inst[i].dest_idx, rs_name);

					// Update RS
					rs._stations[fetched_inst[i].rs_key][fetched_inst[i].cur_rs_idx].busy = false;
				}
				else
					continue;
			}

			// If it was issued in cycle - 1, its starts executing now 
			if (fetched_inst[i].state == State::Executing)
			{
				all_inst_status[i].curCycle--;
				if (all_inst_status[i].curCycle == 0)
				{
					fetched_inst[i].state = State::Complete;
					all_inst_status[i].cycleExecuted = thiscycle;
				}
			}

			if (fetched_inst[i].state == State::NA)
			{
				int free_station_idx = populate_stations(rs, fetched_inst[i].rsType, fetched_inst[i].s1_idx, fetched_inst[i].s2_idx, fetched_inst[i].dest_idx, rrs);
				if (free_station_idx != -1)								// Station free; Change state to issued; Keep track of which RS
				{
					fetched_inst[i].state = State::Issued;
					fetched_inst[i].cur_rs_idx = free_station_idx;
					all_inst_status[i].cycleIssued = thiscycle;
				}
			}

			if (fetched_inst[i].state == State::Issued)
			{
				// Check if ready to begin executing
				string rs_key = fetched_inst[i].rs_key;
				int rs_idx = fetched_inst[i].cur_rs_idx;

				if (rs_key == "Add" || rs_key == "Mult")
				{
					if (rs._stations[rs_key][rs_idx].q[0] == "" 
						&& rs._stations[rs_key][rs_idx].q[1] == "")
					{
						fetched_inst[i].state = State::Executing;
						all_inst_status[i].curCycle = fetched_inst[i].run_time;
					}
				}
				else if (rs_key == "Store") {
					int cur_dest_idx = fetched_inst[i].dest_idx;

					if (rrs._registers[cur_dest_idx].ReservationStationName == ""
						|| (rrs._registers[cur_dest_idx].ReservationStationName != "" && rrs._registers[cur_dest_idx].dataReady == true))
					{
						fetched_inst[i].state = State::Executing;
						all_inst_status[i].curCycle = fetched_inst[i].run_time;
					}
				}
				else{
					fetched_inst[i].state = State::Executing;
					all_inst_status[i].curCycle = fetched_inst[i].run_time;
				}

				// ELSE WAIT TO BE EXECUTED
			}
		}

		if (!inst_pending)
			break;

		// At the end of this cycle, we need this function to print all registers status for grading
		PrintRegisterResultStatus4Grade(outputtracename, rrs, thiscycle);

		++thiscycle;
	}
};

void ReadAndPopulateInstructions(vector<Instruction> &all_inst, vector<InstructionStatus> &all_inst_status)
{
	std::ifstream inst_stream;
	string line;

	inst_stream.open(inputtracename);
	if (inst_stream.is_open())
	{
		while (std::getline(inst_stream,line))
		{      
			std::istringstream iss(line);
			string opString, dest_reg, src1, src2;
			iss >> opString >> dest_reg >> src1 >> src2;

			Instruction ist;
			InstructionStatus status;

			if (opString == "LOAD" || opString == "STORE")
			{
				ist.op = (opString == "LOAD"
					? Operation::OP_LOAD 
					: Operation::OP_STORE);

				ist.rsType = (opString == "LOAD" ? RSType::LOAD : RSType::STORE);
			}
			else if (opString == "ADD" || opString == "SUB")
			{
				ist.op = (opString == "ADD" ? Operation::OP_ADD : Operation::OP_SUB);
				ist.rsType = RSType::ADD;
				ist.s1_idx = stoi(src1.substr(1));
				ist.s2_idx = stoi(src2.substr(1));
			}
			else
			{
				ist.op = (opString == "MULT" ? Operation::OP_MULT : Operation::OP_DIV);
				ist.rsType = RSType::MULT;
				ist.s1_idx = stoi(src1.substr(1));
				ist.s2_idx = stoi(src2.substr(1));
			}

			ist.rs_key = getRSTypeString(ist.rsType);
			ist.dest_idx = stoi(dest_reg.substr(1));

			ist.run_time = OperationCycle[ist.op];
			ist.state = State::NA;

			all_inst.push_back(ist);
			all_inst_status.push_back(status);
		}                   
	}
	else 
		cout<<"Unable to open file";

	inst_stream.close(); 
}

int main(int argc, char **argv)
{
	if (argc > 1)
	{
		hardwareconfigname = argv[1];
		inputtracename = argv[2];
	}

	HardwareConfig hardwareConfig;
	std::ifstream config;
	config.open(hardwareconfigname);
	config >> hardwareConfig.LoadRSsize;  // number of load reservation stations
	config >> hardwareConfig.StoreRSsize; // number of store reservation stations
	config >> hardwareConfig.AddRSsize;	  // number of add reservation stations
	config >> hardwareConfig.MultRSsize;  // number of multiply reservation stations
	config >> hardwareConfig.FRegSize;	  // number of fp registers
	config.close();

	vector<Instruction> all_inst;
	vector<InstructionStatus> all_inst_status;
	RegisterResultStatuses registerResultStatus(hardwareConfig.FRegSize);
	ReservationStations stations(hardwareConfig);

	ReadAndPopulateInstructions(all_inst, all_inst_status);
	// printStuff(registerResultStatus, all_inst, all_inst_status, stations);

/*********************************** ↓↓↓ Todo: Implement by you ↓↓↓ ******************************************/

	// Read instructions from a file (replace 'instructions.txt' with your file name)
	// ...

	// Simulate Tomasulo:
	simulateTomasulo(registerResultStatus, all_inst, all_inst_status, stations);

/*********************************** ↑↑↑ Todo: Implement by you ↑↑↑ ******************************************/

	// At the end of the program, print Instruction Status Table for grading
	PrintResult4Grade(outputtracename, all_inst_status);

	return 0;
}
