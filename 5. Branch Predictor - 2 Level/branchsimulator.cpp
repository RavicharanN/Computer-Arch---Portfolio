#include <iostream>
#include <fstream>
#include <sstream>
#include <math.h>
#include <vector>
#include <bitset>

using namespace std;
const int STRONG_NOT_TAKEN = 0, WEAK_NOT_TAKEN  = 1, WEAK_TAKEN = 2, STRONG_TAKEN = 3;

string extract_bits(int value, int num_bits)
{
	bitset<32> binary = value;
	return binary.to_string().substr(32 - num_bits, num_bits);
}

void update_bht(vector<int> &bht, int branch_taken, int idx, int w)
{
	string bits = (bitset<32>(bht[idx] << 1)).to_string().substr(32 - w, w-1);
	bits += to_string(branch_taken);
	bht[idx] = bitset<32>(bits).to_ulong();

	return;
}

int get_prediction(int pht_val)
{
	int prediction = 0;
	if (pht_val == WEAK_TAKEN || pht_val == STRONG_TAKEN)
		prediction = 1;
	
	return prediction;
}

void update_pht(vector<int> &pht, int idx, int was_branch_taken)
{
	int prediction = get_prediction(pht[idx]);
	bool is_pred_true = (was_branch_taken == prediction);

	switch (pht[idx])
	{
		case STRONG_NOT_TAKEN:
			if (!is_pred_true)
				pht[idx] = WEAK_NOT_TAKEN;
			break;

		case WEAK_NOT_TAKEN:
			if (!is_pred_true)
				pht[idx] = WEAK_TAKEN;
			else
				pht[idx] = STRONG_NOT_TAKEN;
			break;
		
		case WEAK_TAKEN:
			if (is_pred_true)
				pht[idx] = STRONG_TAKEN;
			else
				pht[idx] = WEAK_NOT_TAKEN;
			break;
		
		case STRONG_TAKEN:
			if (!is_pred_true)
				pht[idx] = WEAK_TAKEN;
			break;
		
		default:
			break;
	}
}

int main (int argc, char** argv) {
	ifstream config;
	config.open(argv[1]);

	int m, w, h;
	config >> m;
	config >> h;
	config >> w; 
	config.close();

	ofstream out;
	string out_file_name = string(argv[2]) + ".out";
	out.open(out_file_name.c_str());
	
	ifstream trace;
	trace.open(argv[2]);
	string line;

	vector<int> bht(pow(2, h), 0);
	vector<int> pht(pow(2, m), WEAK_TAKEN);

	// TODO: Implement a two-level branch predictor 
	while (!trace.eof()) {
        getline(trace, line);

        if (line.empty()) {
            continue;  // Skip empty lines
        }

        istringstream iss(line);
        string PCHex;
        int branchTaken;

        // Read the hexadecimal address and bit from the line
        iss >> PCHex >> branchTaken;

        // Convert the hexadecimal address to an integer
       	unsigned int PCInt;
        stringstream ss;
        ss << std::hex << PCHex;
        ss >> PCInt;

		bitset<32> PC = PCInt;
		cout << "Given " << PC.to_string() << "  " << branchTaken << endl;

		int h_bits =  bitset<32>(PC.to_string().substr(30 - h, h)).to_ulong();

		int w_val = bht[h_bits];
		bitset<32> w_bits = w_val;

		cout << "H = " << h << " " << PC.to_string().substr(30 - h, h) << endl;
		cout << "W = " << w << " " << "Val: " << extract_bits(w_val, w) << endl;

		int m_w_bits = bitset<32>(PC.to_string().substr(30 - (m-w), m-w)).to_ulong();

		string first_half = extract_bits(m_w_bits, m - w);
		string second_half = extract_bits(w_val, w);

		cout << "PHT Look up" << first_half << second_half << endl;
		int pht_idx = (bitset<32>(first_half + second_half)).to_ulong();

		int prediction = get_prediction(pht[pht_idx]);
		cout << "PHT Val, pred, branch action, t/f: " << pht[pht_idx] << " " << prediction << " " << branchTaken << " " << (branchTaken == prediction) << endl;
		out << prediction << endl;

		update_pht(pht, pht_idx, branchTaken);
		cout << "Updated PHT " << pht[pht_idx] << endl; 

		update_bht(bht, branchTaken, h_bits, w);
	}
	 
	trace.close();	
	out.close();
}

// Path: branchsimulator_skeleton_23.cpp