/**
 * @file ass3.cpp
 *
 * @brief This file reads the given MIPS instructions and executes them
 *
 * @ingroup COL216
 *
 * @authors VNS Aditya, Alladi Ajay
 *
 */

/* Semantics:- 
 * add reg1, reg2, reg3
 * sub reg1, reg2, reg3
 * mul reg1, reg2, reg3
 * beq reg1, reg2, line no.
 * bne reg1, reg2, line no.
 * slt reg1, reg2, reg3
 * j line no.
 * lw reg1, offset(reg2)
 * sw reg1, offset(reg2)
 * addi reg1, reg2, reg3
*/

#include <iostream>
#include <fstream> 
#include <bits/stdc++.h>
#include <experimental/filesystem>
#include <sstream>
#include <string>
#include <fstream> 
#include <boost/program_options.hpp>

using namespace std;
namespace po = boost::program_options;


string operations[10] = {"add", "sub", "mul", "beq", "bne", "slt", "j", "lw", "sw", "addi"};
string registers[32] = {"$zero", "$at", "$v0", "$v1", "$a0", "$a1", "$a2", "$a3", 
						"$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7", 
						"$s0", "$s1", "$s2", "$s3", "$s4", "$s5", "$s6", "$s7",
						"$t8", "$t9", "$k0", "$k1", "$gp", "$sp", "$fp", "$ra"};
string registers_final[32] = {"zero", "at", "v0", "v1", "a0", "a1", "a2", "a3", 
						"t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7", 
						"s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7",
						"t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra"};
int reg_values[32];
int prev_reg_values[32];
int data_address = 0;
int clock_cycles = 0;
int prev_clock_cycles = 0;
int memory[(int)pow(2.0, 20.0)];
int row_buffer[1024];
int cur_row = -1;
string cur_register = "None";
string cur_register_2 = "None";
string cur_dram = "None";
bool on_hold = false;
int max_cycle;
string onhold_output;
vector<string> all_ins;
int buffer_updates;

struct instruct{
	// ins_id of add: 1; sub: 2; mul: 3; beq: 4; bne: 5; slt: 6; j:7; lw: 8; sw: 9; addi: 10
	int ins_id = 0;
	string reg[3] = {"None", "None", "None"};
	int val = 0;
	string num_val = "NULL";
};

int inArray(string word, string arr[], int size);
int execute(vector<instruct> PC, int line_no, int data_memory, bool is_ins[], int ROW_ACCESS_DELAY, int COL_ACCESS_DELAY);
int compute(instruct ins, int line_no, bool is_ins[], int ROW_ACCESS_DELAY, int COL_ACCESS_DELAY);
int Add(string reg1, string reg2, string reg3, int line_no);
int Sub(string reg1, string reg2, string reg3, int line_no);
int Mul(string reg1, string reg2, string reg3, int line_no);
int Beq(string reg1, string reg2, int val);
int Bne(string reg1, string reg2, int val);
int Slt(string reg1, string reg2, string reg3, int line_no);
int Lw(string reg1, string reg2, int offset, int line_no, int ROW_ACCESS_DELAY, int COL_ACCESS_DELAY);
int Sw(string reg1, string reg2, int offset, int line_no, int ROW_ACCESS_DELAY, int COL_ACCESS_DELAY);
int Addi(string reg1, string reg2, int val, int line_no);
void print_register(bool is_ins[], int temp);
instruct getInstruct(vector<instruct> PC, int line_no);
bool check_1(instruct ins, int line_no);
bool check_2(instruct ins, int line_no);
bool check_3(instruct ins, int line_no);
void initialize();
int isZero(int r);
void print_memory();
bool is_independent(string reg[], int len, int id);
string getLine(vector<string> all_ins, int line_no);

int main(int argc, char* argv[]){
	string file_name;
	vector<instruct> PC;
	fstream input_file;
	int ROW_ACCESS_DELAY;
	std::istringstream ss2(argv[2]);
	if (!(ss2 >> ROW_ACCESS_DELAY)) {
		cout << "Invalid number: " << argv[2] << '\n';
		return -1;
	} 
	int COL_ACCESS_DELAY;
	std::istringstream ss1(argv[3]);
	if (!(ss1 >> COL_ACCESS_DELAY)) {
		cout << "Invalid number: " << argv[3] << '\n';
		return -1;
	} 
	input_file.open(argv[1], ios::in);
	string str;
	string word;
	
	int line_num = 0;

	if(ROW_ACCESS_DELAY < 1){
		cout << "Invalid ROW_ACCESS_DELAY value" << endl;
		return -1;
	}
	if(COL_ACCESS_DELAY < 1){
		cout << "Invalid COL_ACCESS_DELAY value" << endl;
		return -1;
	}
	while(getline(input_file, str, '\n')){
		stringstream ss(str);
		instruct line;
		int i = 0;
		int val_start = 0;
		while (ss >> word){
			if (word[word.length() - 1] == ','){
				word.pop_back();
			}
			if(i == 0){
				int ins_id = inArray(word, operations, 10);
				if (ins_id == 0){
					cout << "Invalid instruction: " << word << " in line "<< line_num + 1 << endl;
					return -1;
				}
				else{
					line.ins_id = ins_id;
				}
			}
			else if(i >= 1 && i < 4){
				if (inArray(word, registers, 32) != 0){
					line.reg[i - 1] = word;
					i++;
					continue;
				}
				string temp = word;
				char* end;
				char* c = const_cast<char*>(word.c_str());
				int val = strtol(c, &end, 10);
				if (*end) {
					word = end;
					if(word[0] != '('){
						cout << "Invalid Syntax in line: " << line_num + 1 << endl;
						return -1;
					}
					if(word[word.size() - 1] != ')'){
						cout << "Invalid Syntax in line: " << line_num + 1 << endl;
						return -1;
					}
					word = word.substr(1, word.size() - 2);
					if(inArray(word, registers, 32) != 0){
						line.reg[i - 1] = word;
						if(val_start >= 1){
							cout << "Invalid instruction: " << temp << " in line "<< line_num + 1 << endl;
							return -1;
						}
						line.val = val;
						line.num_val = "yes";
						val_start++;
					}
					else{
    					cout << "Invalid instruction: " << temp << " in line "<< line_num + 1 << endl;
    					return -1;
    				}
				}
				else {
					if(val_start >= 1){
						cout << "Invalid instruction: " << temp << " in line "<< line_num + 1 << endl;
						return -1;
					}
	    			line.val = val;
	    			line.num_val = "yes";
	   				val_start++;
				}
			}
			else if(i >= 4){
				cout << "Invalid instruction: " << word <<" in line "<< line_num << endl;
				return -1;
			}			
			i++;
		}
		data_address = data_address + 4;
		PC.push_back(line);
		all_ins.push_back(str);
		line_num++;
	}
	bool is_ins[line_num + 1];
	for(int k = 0; k < line_num + 1; k++){
		is_ins[k] = true;
	}
	int print_reg = execute(PC, 0, data_address, is_ins, ROW_ACCESS_DELAY, COL_ACCESS_DELAY);
	if(print_reg == -1){
		return -1;
	}
	for(int p = 0; p < 1024; p++){
		memory[cur_row*1024 + p] = row_buffer[p];
	}
	cout << "cycle " << clock_cycles + 1<< "-" << clock_cycles + ROW_ACCESS_DELAY << ": " << "write back row buffer to DRAM" << endl;
	cout << endl;
	clock_cycles = clock_cycles + ROW_ACCESS_DELAY;
	cout << "Clock Cycles: " << clock_cycles << endl;
	cout << endl;
	print_memory();
	cout << "Total Buffer Updates: " << buffer_updates << endl;
	return 0;
}

int inArray(string word, string arr[], int size){
	for (int i = 0; i < size; i++){
		int x = word.compare(arr[i]);
		if (x == 0){
			return i + 1;
		}
	}
	return 0;
}

int execute(vector<instruct> PC, int line_no, int data_address, bool is_ins[], int ROW_ACCESS_DELAY, int COL_ACCESS_DELAY){
	int temp;
	while(line_no < (data_address/4)){
		temp = line_no;
		instruct ins = getInstruct(PC, line_no);
		line_no = compute(ins, line_no, is_ins, ROW_ACCESS_DELAY, COL_ACCESS_DELAY);
		if (line_no == -1){
			return -1;
		}
		if(ins.ins_id == 8 || ins.ins_id == 9){
			continue;
		}
		if(is_ins[temp] != false){
			if(clock_cycles - prev_clock_cycles == 1){
				cout << "cycle " << clock_cycles << ": ";
			}
			else{
				cout << "cycle " << prev_clock_cycles + 1 << "-" << clock_cycles << ": ";
			}
			
		}
		
		print_register(is_ins, temp);
		//print_data(is_ins, temp);
		for (int j = 0; j < 32; j++){
			prev_reg_values[j] = reg_values[j];
		}
		prev_clock_cycles = clock_cycles;
	}	
	if(on_hold == true){
		cout << onhold_output;
		clock_cycles = max_cycle;
		on_hold = false;
	}
	return 0;
}
bool is_independent(string reg[], int len, int id){
	if(len == 0){
		return true;
	}
	if(cur_dram == "lw"){
		if(id == 1 || id == 2 || id == 3 || id == 6 || id == 10){
			for(int i = 0; i < len; i++){
				if(cur_register == reg[i]){
					return false;
				}
			}
			if(cur_register_2 == reg[0]){
				return false;
			}
		}
		else if(id == 4 || id == 5){
			for(int i = 0; i < len; i++){
				if(cur_register == reg[i]){
					return false;
				}
			}
		}
	}
	else if(cur_dram == "sw"){
		if(id == 1 || id == 2 || id == 3 || id == 6 || id == 10){
			if(cur_register == reg[0] || cur_register_2 == reg[0]){

				return false;
			}
		}
		else if(id == 4 || id == 5){
			return true;
		}
	}
	return true;
}
int compute(instruct ins, int line_no, bool is_ins[], int ROW_ACCESS_DELAY, int COL_ACCESS_DELAY){
	if(ins.ins_id == 1){
		bool is_ind = is_independent(ins.reg, 3, ins.ins_id);
		if(is_ind == false || clock_cycles == max_cycle){
			cout << onhold_output;
			clock_cycles = max_cycle;
			on_hold = false;
			cur_register = "None";
			cur_register_2 = "None";
			cur_dram = "None";
			prev_clock_cycles = clock_cycles;
		}
		clock_cycles++;
		bool go = check_1(ins, line_no);
		if(go == true){
			int x = Add(ins.reg[0], ins.reg[1], ins.reg[2], line_no);
			if (x == -1){
				return -1;
			}
			return line_no + 1;
		}
		else{
			return -1;
		}
	}

	else if(ins.ins_id == 2){
		bool is_ind = is_independent(ins.reg, 3, ins.ins_id);
		if(is_ind == false || clock_cycles == max_cycle){
			cout << onhold_output;
			clock_cycles = max_cycle;
			on_hold = false;
			cur_register = "None";
			cur_register_2 = "None";
			cur_dram = "None";
			prev_clock_cycles = clock_cycles;
		}
		clock_cycles++;
		bool go = check_1(ins, line_no);
		if(go == true){
			int x = Sub(ins.reg[0], ins.reg[1], ins.reg[2], line_no);
			if (x == -1){
				return -1;
			}
			return line_no + 1;
		}
		else{
			return -1;
		}
	}

	else if(ins.ins_id == 3){
		bool is_ind = is_independent(ins.reg, 3, ins.ins_id);
		if(is_ind == false || clock_cycles == max_cycle){
			cout << onhold_output;
			clock_cycles = max_cycle;
			on_hold = false;
			cur_register = "None";
			cur_register_2 = "None";
			cur_dram = "None";
			prev_clock_cycles = clock_cycles;
		}
		clock_cycles++;
		bool go = check_1(ins, line_no);
		if(go == true){
			int x = Mul(ins.reg[0], ins.reg[1], ins.reg[2], line_no);
			if (x == -1){
				return -1;
			}
			return line_no + 1;
		}
		else{
			return -1;
		}
	}

	else if(ins.ins_id == 4){
		bool is_ind = is_independent(ins.reg, 2, ins.ins_id);
		if(is_ind == false || clock_cycles == max_cycle){
			cout << onhold_output;
			clock_cycles = max_cycle;
			on_hold = false;
			cur_register = "None";
			cur_register_2 = "None";
			cur_dram = "None";
			prev_clock_cycles = clock_cycles;
		}
		clock_cycles++;
		bool go = check_2(ins, line_no);
		if(go == true){
			int x = Beq(ins.reg[0], ins.reg[1], ins.val);
			if(x == 0){
				return 1 + line_no;
			}
			else{
				if(ins.val - 1 < 1){
					cout << "Can't go to the mentioned line from the branch, line " << line_no + 1<< endl;
					return -1;
				}
				return ins.val - 1;
			}
		}
		else{
			return -1;
		}
	}

	else if(ins.ins_id == 5){
		bool is_ind = is_independent(ins.reg, 2, ins.ins_id);
		if(is_ind == false || clock_cycles == max_cycle){
			cout << onhold_output;
			clock_cycles = max_cycle;
			on_hold = false;
			cur_register = "None";
			cur_register_2 = "None";
			cur_dram = "None";
			prev_clock_cycles = clock_cycles;
		}
		clock_cycles++;
		bool go = check_2(ins, line_no);
		if(go == true){
			int x = Bne(ins.reg[0], ins.reg[1], ins.val);
			if(x == 0){
				return 1 + line_no;
			}
			else{
				if(ins.val - 1 < 1){
					cout << "Can't go to the mentioned line from the branch, line " << line_no + 1<< endl;
					return -1;
				}
				return ins.val - 1;
			}
		}
		else{
			return -1;
		}
	}

	else if(ins.ins_id == 6){
		bool is_ind = is_independent(ins.reg, 3, ins.ins_id);
		if(is_ind == false || clock_cycles == max_cycle){
			cout << onhold_output;
			clock_cycles = max_cycle;
			on_hold = false;
			cur_register = "None";
			cur_register_2 = "None";
			cur_dram = "None";
			prev_clock_cycles = clock_cycles;
		}
		clock_cycles++;
		bool go = check_1(ins, line_no);
		if(go == true){
			int x = Slt(ins.reg[0], ins.reg[1], ins.reg[2], line_no);
			if (x == -1){
				return -1;
			}
			return line_no + 1;
		}
		else{
			return -1;
		}
	}

	else if(ins.ins_id == 7){
		bool is_ind = is_independent(ins.reg, 0, ins.ins_id);
		if(is_ind == false || clock_cycles == max_cycle){
			cout << onhold_output;
			clock_cycles = max_cycle;
			on_hold = false;
			cur_register = "None";
			cur_register_2 = "None";
			cur_dram = "None";
			prev_clock_cycles = clock_cycles;
		}
		clock_cycles++;
		bool go = check_3(ins, line_no);
		if(go == true){
			if(ins.val - 1 < 1){
				cout << "Can't go to the mentioned line from jump, line " << line_no + 1 << endl;
				return -1;
			}
			return ins.val - 1;
		}
		else{
			return -1;
		}
	}

	else if(ins.ins_id == 8){
		if(on_hold == true){
			cout << onhold_output;
			on_hold = false;
			clock_cycles = max_cycle;
			cur_register = "None";
			cur_register_2 = "None";
			cur_dram = "None";
			prev_clock_cycles = clock_cycles;
		}
		
		bool go = check_2(ins, line_no);
		if(go == true){
			int valid_addr = Lw(ins.reg[0], ins.reg[1], ins.val, line_no, ROW_ACCESS_DELAY, COL_ACCESS_DELAY);
			if(valid_addr == -1){
				return -1;
			}
			return line_no + 1;
		}
		else{
			return -1;
		}
	}

	else if(ins.ins_id == 9){
		if(on_hold == true){
			cout << onhold_output;
			on_hold = false;
			clock_cycles = max_cycle;
			cur_register = "None";
			cur_register_2 = "None";
			cur_dram = "None";
			prev_clock_cycles = clock_cycles;
		}
		
		bool go = check_2(ins, line_no);
		if(go == true){
			int valid_addr = Sw(ins.reg[0], ins.reg[1], ins.val, line_no, ROW_ACCESS_DELAY, COL_ACCESS_DELAY);
			if(valid_addr == -1){
				return -1;
			}
			return line_no + 1;
		}
		else{
			return -1;
		}
	}

	else if(ins.ins_id == 10){
		bool is_ind = is_independent(ins.reg, 2, ins.ins_id);
		if(is_ind == false || clock_cycles == max_cycle){
			cout << onhold_output;
			clock_cycles = max_cycle;
			on_hold = false;
			cur_register = "None";
			cur_register_2 = "None";
			cur_dram = "None";
			prev_clock_cycles = clock_cycles;
		}
		clock_cycles++;
		bool go = check_2(ins, line_no);
		if(go == true){
			int x = Addi(ins.reg[0], ins.reg[1], ins.val, line_no);
			if (x == -1){
				return -1;
			}
			return line_no + 1;
		}
		else{
			return -1;
		}
	}
	else{
		is_ins[line_no] = false;
		return line_no + 1;
	}
}

int Add(string reg1, string reg2, string reg3, int line_no){
	int r1 = inArray(reg1, registers, 32) - 1;
	int r2 = inArray(reg2, registers, 32) - 1;
	int r3 = inArray(reg3, registers, 32) - 1;
	if(isZero(r1) == -1){
		cout << "Zero register can't be changed, line: " << line_no + 1 << endl; 
		return -1;
	}
	reg_values[r1] = reg_values[r2] + reg_values[r3];
	return 0;
}

int Sub(string reg1, string reg2, string reg3, int line_no){
	int r1 = inArray(reg1, registers, 32) - 1;
	int r2 = inArray(reg2, registers, 32) - 1;
	int r3 = inArray(reg3, registers, 32) - 1;
	if(isZero(r1) == -1){
		cout << "Zero register can't be changed, line: " << line_no + 1 << endl; 
		return -1;
	}
	reg_values[r1] = reg_values[r2] - reg_values[r3];
	return 0;
}

int Mul(string reg1, string reg2, string reg3, int line_no){
	int r1 = inArray(reg1, registers, 32) - 1;
	int r2 = inArray(reg2, registers, 32) - 1;
	int r3 = inArray(reg3, registers, 32) - 1;
	reg_values[r1] = reg_values[r2] * reg_values[r3];
	if(isZero(r1) == -1){
		cout << "Zero register can't be changed, line: " << line_no + 1 << endl; 
		return -1;
	}
	return 0;
}

int Beq(string reg1, string reg2, int val){
	int r1 = inArray(reg1, registers, 32) - 1;
	int r2 = inArray(reg2, registers, 32) - 1;
	if(reg_values[r1] == reg_values[r2]){
		return 1;
	}
	return 0;
}

int Bne(string reg1, string reg2, int val){
	int r1 = inArray(reg1, registers, 32) - 1;
	int r2 = inArray(reg2, registers, 32) - 1;
	if(reg_values[r1] != reg_values[r2]){
		return 1;
	}
	return 0;
}

int Slt(string reg1, string reg2, string reg3, int line_no){
	int r1 = inArray(reg1, registers, 32) - 1;
	int r2 = inArray(reg2, registers, 32) - 1;
	int r3 = inArray(reg3, registers, 32) - 1;
	if(isZero(r1) == -1){
		cout << "Zero register can't be changed, line: " << line_no + 1 << endl; 
		return -1;
	}
	if(reg_values[r2] < reg_values[r3]){
		reg_values[r1] = 1;
		return 0;
	}
	else{
		reg_values[r1] = 0;
		return 0;
	}
}

int Lw(string reg1, string reg2, int offset, int line_no, int ROW_ACCESS_DELAY, int COL_ACCESS_DELAY){
	int r1 = inArray(reg1, registers, 32) - 1;
	int r2 = inArray(reg2, registers, 32) - 1;
	if(isZero(r1) == -1){
		cout << "Zero register can't be changed, line: " << line_no + 1 << endl; 
		return -1;
	}
	if(reg_values[r2] + offset >= pow(2.0, 20.0)){

		cout << "Memory address out of bounds, line no: " << line_no + 1 << endl;
		return -1;
	}
	else if(reg_values[r2] + offset < data_address){

		cout << "Invalid memory address, line no: " << line_no + 1 << endl;
		return -1;
	}
	else if((reg_values[r2] + offset)%4 != 0){

		cout << "Invalid memory address, line no: " << line_no + 1 << endl;
		return -1;
	}
	else{
		on_hold = true;
		cur_dram = "lw";
		clock_cycles++;
		cout << "cycle " << clock_cycles << ": DRAM request issued" << endl;
		cout << endl;
		prev_clock_cycles = clock_cycles;
		int row = (reg_values[r2] + offset)/1024;
		int col = (reg_values[r2] + offset)%1024;
		int x = reg_values[r2] + offset;
		int y = x + 3;
		cur_register = reg1;
		cur_register_2 = reg2;
		if (cur_row == -1){
			for(int i = 0; i < 1024; i++){
				row_buffer[i] = memory[row*1024 + i];
			}
			reg_values[r1] = row_buffer[col];
			cur_row = row;
			clock_cycles = clock_cycles + COL_ACCESS_DELAY + ROW_ACCESS_DELAY;
			onhold_output =  "cycle " + std::to_string(prev_clock_cycles + 1) + "-" + std::to_string(clock_cycles) + ": " + reg1 + "="  + std::to_string(reg_values[r1])+", Row buffer not updated" + ", Activated row " + std::to_string(row) + " and accessed address " + std::to_string(x) + "\n\n";
			max_cycle = clock_cycles;
			clock_cycles = prev_clock_cycles;
			buffer_updates = buffer_updates + 1;
			return 0;
		}
		else if(row == cur_row){
			reg_values[r1] = row_buffer[col];
			clock_cycles = clock_cycles + COL_ACCESS_DELAY;
			onhold_output =  "cycle " + std::to_string(prev_clock_cycles + 1) + "-" + std::to_string(clock_cycles) + ": " + reg1 + "="  + std::to_string(reg_values[r1])+ ", Row buffer not updated"+ ", Accessed address " + std::to_string(x) + " from row buffer\n\n";
			max_cycle = clock_cycles;
			clock_cycles = prev_clock_cycles;
			return 0;
		}
		else{
			for(int i = 0; i < 1024; i++){
				memory[cur_row*1024 + i] = row_buffer[i];
				row_buffer[i] = memory[row*1024 + i];
			}
			reg_values[r1] = row_buffer[col];
			cur_row = row;
			clock_cycles = clock_cycles + COL_ACCESS_DELAY + 2*ROW_ACCESS_DELAY;
			onhold_output =  "cycle " + std::to_string(prev_clock_cycles + 1) + "-" + std::to_string(clock_cycles) + ": " + reg1 + "="  + std::to_string(reg_values[r1])+ ", Row buffer not updated" + ", Copied a row and activated row " + std::to_string(row) + " and accessed address " + std::to_string(x) + "\n\n";
			max_cycle = clock_cycles;
			clock_cycles = prev_clock_cycles;
			buffer_updates = buffer_updates + 1;
			return 0;
		}
	}	
}

int Sw(string reg1, string reg2, int offset, int line_no, int ROW_ACCESS_DELAY, int COL_ACCESS_DELAY){

	int r1 = inArray(reg1, registers, 32) - 1;
	int r2 = inArray(reg2, registers, 32) - 1;
	if(reg_values[r2] + offset >= pow(2.0, 20.0)){
		cout << "Memory address out of bounds, line no: " << line_no + 1 << endl;
		return -1;
	}
	else if(reg_values[r2] + offset < data_address){
		cout << "Invalid memory address, line no: " << line_no + 1 << endl;
		return -1;
	}
	else if((reg_values[r2] + offset)%4 != 0){

		cout << "Invalid memory address, line no:" << line_no + 1 << endl;
		return -1;
	}
	
	else{
		cur_dram = "sw";
		clock_cycles++;
		cout << "cycle " << clock_cycles << ": DRAM request issued" << endl;
		cout << endl;
		prev_clock_cycles = clock_cycles;
		int x = reg_values[r2] + offset;
		int y = x + 3;
		on_hold = true;
		int row = (reg_values[r2] + offset)/1024;
		int col = (reg_values[r2] + offset)%1024;
		cur_register = reg1;
		cur_register_2 = reg2;
		if (cur_row == -1){
			for(int i = 0; i < 1024; i++){
				row_buffer[i] = memory[row*1024 + i];
			}
			row_buffer[col] = reg_values[r1];
			cur_row = row;
			clock_cycles = clock_cycles + COL_ACCESS_DELAY + ROW_ACCESS_DELAY;
			onhold_output = "cycle " + std::to_string(prev_clock_cycles + 1) + "-" + std::to_string(clock_cycles) + ": " + "No register modified, Updated memory address " + std::to_string(x) + "-" + std::to_string(y) + " = " + std::to_string(row_buffer[col])+ ", Activated row " + std::to_string(row) + " and accessed address " + std::to_string(x) + "\n\n";
			max_cycle = clock_cycles;
			clock_cycles = prev_clock_cycles;
			buffer_updates = buffer_updates + 2;
			return 0;
		}
		else if(row == cur_row){
			row_buffer[col] = reg_values[r1];
			clock_cycles = clock_cycles + COL_ACCESS_DELAY;
			onhold_output = "cycle " + std::to_string(prev_clock_cycles + 1) + "-" + std::to_string(clock_cycles) + ": " + "No register modified, Updated memory address " + std::to_string(x) + "-" + std::to_string(y) + " = " + std::to_string(row_buffer[col]) + "Accessed address" + std::to_string(x) + " from the row buffer\n\n";;
			max_cycle = clock_cycles;
			clock_cycles = prev_clock_cycles;
			buffer_updates = buffer_updates + 1;
			return 0;
		}
		else{
			for(int i = 0; i < 1024; i++){
				memory[cur_row*1024 + i] = row_buffer[i];
				row_buffer[i] = memory[row*1024 + i];
			}
			row_buffer[col] = reg_values[r1];
			cur_row = row;
			clock_cycles = clock_cycles + COL_ACCESS_DELAY + 2*ROW_ACCESS_DELAY;
			onhold_output = "cycle " + std::to_string(prev_clock_cycles + 1) + "-" + std::to_string(clock_cycles) + ": " + "No register modified, Updated memory address " + std::to_string(x) + "-" + std::to_string(y) + " = " + std::to_string(row_buffer[col]) + ", Copied a row and activated row " + std::to_string(row) + " and accessed address " + std::to_string(x) + "\n\n";
			max_cycle = clock_cycles;
			clock_cycles = prev_clock_cycles;
			buffer_updates = buffer_updates + 2;
			return 0;
		}
	}
}

int Addi(string reg1, string reg2, int val, int line_no){
	int r1 = inArray(reg1, registers, 32) - 1;
	int r2 = inArray(reg2, registers, 32) - 1;
	if(isZero(r1) == -1){
		cout << "Zero register can't be changed, line: " << line_no + 1<< endl; 
		return -1;
	}
	reg_values[r1] = reg_values[r2] + val;
	return 0;
}

instruct getInstruct(vector<instruct> PC, int line_no){
	int i = 0;
	for(instruct it: PC){
		if (i == line_no){
			return it;
		}
		i++;
	}
}
string getLine(vector<string> all_ins, int line_no){
	int i = 0;
	for(string it: all_ins){
		if (i == line_no){
			return it;
		}
		i++;
	}
}
void print_register(bool is_ins[], int temp){
	if(is_ins[temp] == true){
		cout << getLine(all_ins, temp) << endl;
		int mod = 0;
		for(int i = 0; i < 32; i++){
			if(prev_reg_values[i] != reg_values[i]){
				cout << registers[i] << " = " << reg_values[i];
				mod++;
			}
		}
		if(mod == 0){
			cout << "No register modified, Memory not modified";
		}
		else{
			cout << ", Memory not modified";
		}
		cout << "\n" << endl;
	}
}

void print_memory(){

	for(int i = 0; i < (int)pow(2.0, 20.0); i++){
		if(memory[i] != 0){
			cout << i << "-" << i + 3 << " = " << memory[i] << endl;
		}
	}
	cout << endl;
	
}

bool check_1(instruct ins, int line_no){
	if (ins.reg[0] == "None" || ins.reg[1] == "None" || ins.reg[2] == "None"){
		cout << "Missing Register(s) in the instruction in line: " << line_no + 1 << endl;
		return false;
	}
	if(ins.num_val != "NULL"){
		cout << "Unexpected Integer(s) in the instruction in line: " << line_no + 1 << endl;
		return false;
	}
	return true;
}

bool check_2(instruct ins, int line_no){
	if(ins.reg[0] == "None" || ins.reg[1] == "None" || ins.reg[2] != "None"){

		cout << "Invalid syntax in line:" << line_no + 1 << endl;
		return false;
	}
	if(ins.num_val == "NULL"){
		cout << "Invalid syntax in line:" << line_no + 1 << endl;

		return false;
	}
	return true;
}

bool check_3(instruct ins, int line_no){
	if (ins.reg[0] != "None" || ins.reg[1] != "None" || ins.reg[2] != "None"){
		cout << "Invalid syntax in line:" << line_no + 1 << endl; 
		return false;
	}
	if(ins.num_val == "NULL"){
		cout << "Invalid syntax in line:" << line_no + 1 << endl;
		return false;
	}
	return true;
}

int isZero(int r){
	if(r == 0){
		return -1;
	}
	return 0;
}