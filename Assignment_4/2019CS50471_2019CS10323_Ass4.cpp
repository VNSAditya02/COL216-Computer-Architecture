/**
 * @file ass4.cpp
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

///////////////////////////////////////////////////////////////////////////


									// MODIFIED STRUCTURE

struct instruct{
	// ins_id of add: 1; sub: 2; mul: 3; beq: 4; bne: 5; slt: 6; j:7; lw: 8; sw: 9; addi: 10
	int ins_id = 0;
	string reg[3] = {"None", "None", "None"};
	int val = 0;
	string num_val = "NULL";
	string label = "None";
	int line_no;
	int reg1_val = 0;
	int reg2_val = 0;
};
////////////////////////////////////////////////////////////////////////////



string operations[10] = {"add", "sub", "mul", "beq", "bne", "slt", "j", "lw", "sw", "addi"};
string registers[32] = {"$zero", "$at", "$v0", "$v1", "$a0", "$a1", "$a2", "$a3", 
						"$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7", 
						"$s0", "$s1", "$s2", "$s3", "$s4", "$s5", "$s6", "$s7",
						"$t8", "$t9", "$k0", "$k1", "$gp", "$sp", "$fp", "$ra"};
int reg_values[32];
int prev_reg_values[32];
int data_address = 0;
int clock_cycles = 0;
int prev_clock_cycles = 0;
int memory[(int)pow(2.0, 20.0)];
int row_buffer[1024];
int cur_row = -1;
vector<string> lw_reg1;
vector<string> lw_reg2;
vector<string> sw_reg1;
vector<string> sw_reg2;
bool on_hold = false;
int max_cycle = -1;
string onhold_output;
vector<string> all_ins;
int buffer_updates;
instruct ins_queue[1000];
int queue_length;
map<string, int> label_dict;

int inArray(string word, string arr[], int size);
int execute(vector<instruct> PC, int line_no, int data_memory, bool is_ins[], int ROW_ACCESS_DELAY, int COL_ACCESS_DELAY);
int compute(instruct ins, int line_no, bool is_ins[], int ROW_ACCESS_DELAY, int COL_ACCESS_DELAY);
int Add(string reg1, string reg2, string reg3, int line_no);
int Sub(string reg1, string reg2, string reg3, int line_no);
int Mul(string reg1, string reg2, string reg3, int line_no);
int Beq(string reg1, string reg2);
int Bne(string reg1, string reg2);
int Slt(string reg1, string reg2, string reg3, int line_no);
int Lw(string reg1, string reg2, int reg2_val, int offset, int line_no, int ROW_ACCESS_DELAY, int COL_ACCESS_DELAY);
int Sw(string reg1, string reg2, int reg1_val, int reg2_val, int offset, int line_no, int ROW_ACCESS_DELAY, int COL_ACCESS_DELAY);
int Addi(string reg1, string reg2, int val, int line_no);
void print_register(bool is_ins[], int temp);
instruct getInstruct(vector<instruct> PC, int line_no);
bool check_1(instruct ins, int line_no);
bool check_2(instruct ins, int line_no);
bool check_3(instruct ins, int line_no);
bool check_4(instruct ins, int line_no);
bool check_5(instruct ins, int line_no);
void initialize();
int isZero(int r, int line_no);
void print_memory();
bool is_independent(string reg[], int len, int id);
string getLine(vector<string> all_ins, int line_no);
void execute_queue(int ROW_ACCESS_DELAY, int COL_ACCESS_DELAY, int id);
void modify();
bool independent_ins(int x, int y);
int compute_row(instruct ins);
void reorder(int x);void print_queue();
void print_queue();

///////////////////////////////////////////////////////////////////


					// MODIFIED MAIN FOR LABELS


//////////////////////////////////////////////////////////////////
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
	int start = 0;
	
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
		int ins_id = 0;
		while (ss >> word){
			if (word[word.length() - 1] == ','){
				word.pop_back();
			}
			if(i == 0){
				ins_id = inArray(word, operations, 10);
				if (ins_id != 0){
					line.ins_id = ins_id;
					if(start == 0){
						cout << "Main not found" << endl;
						return -1;
					}
					//cout << "Invalid instruction: " << word << " in line "<< line_num + 1 << endl;
					//return -1;
				}
				else if(word[word.length() - 1] == ':'){
					string label = word.substr(0, word.length() - 1);		
					if(inArray(label, registers, 32) != 0 || inArray(label, operations, 10) != 0){
						cout << "Invalid label: " << label << " in line " << line_num + 1 << endl;
						return -1;
					}
					auto itr = label_dict.find(label);
					if(itr != label_dict.end()){
						cout << "Label in line no " << line_num + 1 << " is already used" << endl;
						return -1;
					}
					if(!((label[0] >= 'a' && label[0] <= 'z') || (label[0] >= 'A' && label[0] <= 'Z'))){
						cout << label[0] << " - " << int(label[0]) <<endl;
						cout << "Invalid label in line:" << line_num + 1 << endl;
						return -1;
					}
					for(int k = 1; k < label.length(); k++){
						if(!((label[k] >= 'a' && label[k] <= 'z') || (label[k] >= 'A' && label[k] <= 'Z') || (int(label[k]) == 95) || (48 <= int(label[k]) &&  int(label[k]) <= 57))){
							cout << "Invalid label in line:" << line_num + 1 << endl;
							cout << label[k] << endl;
							return -1;
						}
					}
					if(start == 0){
						if(label != "main"){
							cout << "Main not found" << endl;
							return -1;
						}
						start++;
					}
					//cout << "label " << label << endl;
					label_dict.insert({label, line_num});
				}
				else{
					cout << "Invalid Syntax in line " << line_num + 1 << endl;
					return -1;
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
					if(ins_id == 7 || ins_id == 5 || ins_id == 4){
						line.label = temp;
						i++;
						continue;
					}
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
				cout << "Invalid instruction: " << word <<" in line "<< line_num + 1 << endl;
				return -1;
			}			
			i++;
		}
		data_address = data_address + 4;
		line.line_no = line_num;
		PC.push_back(line);
		all_ins.push_back(str);
		line_num++;
	}
	bool is_ins[line_num + 1];
	for(int k = 0; k < line_num + 1; k++){
		is_ins[k] = true;
	}
	execute(PC, 0, data_address, is_ins, ROW_ACCESS_DELAY, COL_ACCESS_DELAY);
	int up = 0;
	for(int p = 0; p < 1024; p++){
		if(memory[cur_row*1024 + p] != row_buffer[p]){
			up = 1;
		}
	}
	
	for(int p = 0; p < 1024; p++){
		memory[cur_row*1024 + p] = row_buffer[p];
	}
	if(up == 1){
	cout << "cycle " << clock_cycles + 1<< "-" << clock_cycles + ROW_ACCESS_DELAY << ": " << "write back row buffer to DRAM" << endl;
	cout << endl;
	clock_cycles = clock_cycles + ROW_ACCESS_DELAY;
	}
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
		line_no = compute(ins, ins.line_no, is_ins, ROW_ACCESS_DELAY, COL_ACCESS_DELAY);
		if(ins.ins_id == 8 || ins.ins_id == 9){
			continue;
		}

		if(temp != line_no){
			if(is_ins[temp] != false){
				if(clock_cycles - prev_clock_cycles == 1){
					cout << "cycle " << clock_cycles << ": ";
				}
				else{
					cout << "cycle " << prev_clock_cycles + 1 << "-" << clock_cycles << ": ";
				}
				cout << "Memory address of the instruction: " << ins.line_no*4 << endl;
			}
			print_register(is_ins, temp);
			for (int j = 0; j < 32; j++){
				prev_reg_values[j] = reg_values[j];
			}
			prev_clock_cycles = clock_cycles;
		}
	}
	int temp_length = queue_length;
	for(int i = 0; i < temp_length; i++){
		execute_queue(ROW_ACCESS_DELAY, COL_ACCESS_DELAY, 0);
	}	
	if(on_hold == true){
		cout << onhold_output;
		clock_cycles = max_cycle;
		on_hold = false;
	}
	return 0;
}

///////////////////////////////////////////////////////////////////

					// MODIFIED FOR QUEUE

///////////////////////////////////////////////////////////////////
int compute(instruct ins, int line_no, bool is_ins[], int ROW_ACCESS_DELAY, int COL_ACCESS_DELAY){
	if(ins.ins_id == 1){
		bool is_ind = is_independent(ins.reg, 3, ins.ins_id);
		if(is_ind == false || clock_cycles == max_cycle){
			execute_queue(ROW_ACCESS_DELAY, COL_ACCESS_DELAY, ins.ins_id);
			//return compute(ins, line_no, is_ins, ROW_ACCESS_DELAY, COL_ACCESS_DELAY);
			return ins.line_no;
		}
		clock_cycles++;
		check_1(ins, ins.line_no);
		Add(ins.reg[0], ins.reg[1], ins.reg[2], ins.line_no);
		return ins.line_no + 1;
	}

	else if(ins.ins_id == 2){
		bool is_ind = is_independent(ins.reg, 3, ins.ins_id);
		if(is_ind == false || clock_cycles == max_cycle){
			execute_queue(ROW_ACCESS_DELAY, COL_ACCESS_DELAY, ins.ins_id);
			//return compute(ins, line_no, is_ins, ROW_ACCESS_DELAY, COL_ACCESS_DELAY);
			return ins.line_no;
		}
		clock_cycles++;
		check_1(ins, ins.line_no);
		int x = Sub(ins.reg[0], ins.reg[1], ins.reg[2], ins.line_no);
		return ins.line_no + 1;
		
	}

	else if(ins.ins_id == 3){
		bool is_ind = is_independent(ins.reg, 3, ins.ins_id);
		if(is_ind == false || clock_cycles == max_cycle){
			execute_queue(ROW_ACCESS_DELAY, COL_ACCESS_DELAY, ins.ins_id);
			//return compute(ins, line_no, is_ins, ROW_ACCESS_DELAY, COL_ACCESS_DELAY);
			return ins.line_no;
		}
		clock_cycles++;
		bool go = check_1(ins, ins.line_no);
		Mul(ins.reg[0], ins.reg[1], ins.reg[2], ins.line_no);
		return ins.line_no + 1;
	}

	else if(ins.ins_id == 4){
		bool is_ind = is_independent(ins.reg, 2, ins.ins_id);
		if(is_ind == false || clock_cycles == max_cycle){
			execute_queue(ROW_ACCESS_DELAY, COL_ACCESS_DELAY, ins.ins_id);
			//return compute(ins, line_no, is_ins, ROW_ACCESS_DELAY, COL_ACCESS_DELAY);
			return ins.line_no;
		}
		clock_cycles++;
		check_4(ins, ins.line_no);
		int x = Beq(ins.reg[0], ins.reg[1]);
		auto itr = label_dict.find(ins.label);
		if(itr == label_dict.end()){
			cout << "Invalid label in line:" << ins.line_no + 1 << endl;
			exit(-1);
		}
		int to_line = itr->second; 
		if(x == 0){
			return 1 + ins.line_no;
		}
		else{
			if(to_line < 0){
				cout << "Can't go to the mentioned line from the branch, line " << ins.line_no + 1<< endl;
				exit(-1);
			}
			return to_line;
		}
	}

	else if(ins.ins_id == 5){
		bool is_ind = is_independent(ins.reg, 2, ins.ins_id);
		if(is_ind == false || clock_cycles == max_cycle){
			execute_queue(ROW_ACCESS_DELAY, COL_ACCESS_DELAY, ins.ins_id);
			//return compute(ins, line_no, is_ins, ROW_ACCESS_DELAY, COL_ACCESS_DELAY);
			return ins.line_no;
		}
		clock_cycles++;
		check_4(ins, ins.line_no);
		int x = Bne(ins.reg[0], ins.reg[1]);
		auto itr = label_dict.find(ins.label);
		if(itr == label_dict.end()){
			cout << "Invalid label in line:" << ins.line_no + 1 << endl;
			exit(-1);
		}
		int to_line = itr->second; 
		if(x == 0){
			return 1 + ins.line_no;
		}
		else{
			if(to_line < 0){
				cout << "Can't go to the mentioned line from the branch, line " << ins.line_no + 1<< endl;
				exit(-1);
			}
			return to_line;
		}
	}

	else if(ins.ins_id == 6){
		bool is_ind = is_independent(ins.reg, 3, ins.ins_id);
		if(is_ind == false || clock_cycles == max_cycle){
			execute_queue(ROW_ACCESS_DELAY, COL_ACCESS_DELAY, ins.ins_id);
			//return compute(ins, line_no, is_ins, ROW_ACCESS_DELAY, COL_ACCESS_DELAY);
			return ins.line_no;
		}
		clock_cycles++;
		check_1(ins, ins.line_no);
		Slt(ins.reg[0], ins.reg[1], ins.reg[2], ins.line_no);
		return ins.line_no + 1;
	}

	else if(ins.ins_id == 7){
		bool is_ind = is_independent(ins.reg, 0, ins.ins_id);
		if(is_ind == false || clock_cycles == max_cycle){
			execute_queue(ROW_ACCESS_DELAY, COL_ACCESS_DELAY, ins.ins_id);
			//return compute(ins, line_no, is_ins, ROW_ACCESS_DELAY, COL_ACCESS_DELAY);
			return ins.line_no;
		}
		clock_cycles++;
		check_5(ins, ins.line_no);
		auto itr = label_dict.find(ins.label);
		if(itr == label_dict.end()){
			cout << "Invalid label in line:" << ins.line_no + 1 << endl;
		}
		int to_line = itr->second; 
		if(to_line < 1){
			cout << "Can't go to the mentioned line from jump, line " << ins.line_no + 1 << endl;
			exit(-1);
		}
		return to_line;
	}

	else if(ins.ins_id == 8){
		if(is_independent(ins.reg, 2, 8) == false){
			execute_queue(ROW_ACCESS_DELAY, COL_ACCESS_DELAY, 0);			
			return ins.line_no;	
		}
		
		int r2 = inArray(ins.reg[1], registers, 32) - 1;
		ins.reg2_val = reg_values[r2];
		ins_queue[queue_length] = ins;
		queue_length++;
		if(queue_length == 1){
			clock_cycles++;
			cout << "cycle " << clock_cycles << ": Memory address of the instruction: " << ins.line_no*4 << endl;
			cout << "Executed Instruction: " << "lw " << ins.reg[0] << ", " << ins.val << "(" << ins.reg[1] << ")" << endl;
			cout << "DRAM request issued" << endl;
			cout << endl;
			prev_clock_cycles = clock_cycles;
			check_2(ins_queue[0], ins_queue[0].line_no);
			Lw(ins_queue[0].reg[0], ins_queue[0].reg[1], ins_queue[0].reg2_val, ins_queue[0].val, ins_queue[0].line_no, ROW_ACCESS_DELAY, COL_ACCESS_DELAY);
			return ins.line_no + 1;
		}
		if(on_hold == true && clock_cycles == max_cycle){
			execute_queue(ROW_ACCESS_DELAY, COL_ACCESS_DELAY, ins.ins_id);			
			return ins.line_no + 1;	
		}
		clock_cycles++;
		cout << "cycle " << clock_cycles << ": Memory address of the instruction: " << ins.line_no*4 << endl;
		cout << "Executed Instruction: " << "lw " << ins.reg[0] << ", " << ins.val << "(" << ins.reg[1] << ")" << endl;
		cout << "DRAM request issued" << endl;
		cout << endl;
		prev_clock_cycles = clock_cycles;
		return ins.line_no + 1;
	}

	else if(ins.ins_id == 9){
		if(is_independent(ins.reg, 2, 9) == false){
			execute_queue(ROW_ACCESS_DELAY, COL_ACCESS_DELAY, 0);			
			return ins.line_no;	
		}
		int r1 = inArray(ins.reg[0], registers, 32) - 1;
		int r2 = inArray(ins.reg[1], registers, 32) - 1;
		ins.reg1_val = reg_values[r1];
		ins.reg2_val = reg_values[r2];
		ins_queue[queue_length] = ins;
		queue_length++;
		if(queue_length == 1){
			clock_cycles++;
			cout << "cycle " << clock_cycles << ": Memory address of the instruction: " << ins.line_no*4 << endl;
			cout << "Executed Instruction: " << "sw " << ins.reg[0] << ", " << ins.val << "(" << ins.reg[1] << ")" << endl;
			cout << "DRAM request issued" << endl;
			cout << endl;
			prev_clock_cycles = clock_cycles;
			check_2(ins_queue[0], ins_queue[0].line_no);
			Sw(ins_queue[0].reg[0], ins_queue[0].reg[1], ins_queue[0].reg1_val, ins_queue[0].reg2_val, ins_queue[0].val, ins_queue[0].line_no, ROW_ACCESS_DELAY, COL_ACCESS_DELAY);
			return ins.line_no + 1;
		}
		if(on_hold == true && clock_cycles == max_cycle){
			
			execute_queue(ROW_ACCESS_DELAY, COL_ACCESS_DELAY, ins.ins_id);			
			return ins.line_no + 1;	
		}
		clock_cycles++;
		cout << "cycle " << clock_cycles << ": Memory address of the instruction: " << ins.line_no*4 << endl;
		cout << "Executed Instruction: " << "sw " << ins.reg[0] << ", " << ins.val << "(" << ins.reg[1] << ")" << endl;
		cout << "DRAM request issued" << endl;
		cout << endl;
		prev_clock_cycles = clock_cycles;
		return ins.line_no + 1;
		
	}

	else if(ins.ins_id == 10){
		bool is_ind = is_independent(ins.reg, 2, ins.ins_id);
		if(is_ind == false || clock_cycles == max_cycle){
			execute_queue(ROW_ACCESS_DELAY, COL_ACCESS_DELAY, ins.ins_id);
			//return compute(ins, line_no, is_ins, ROW_ACCESS_DELAY, COL_ACCESS_DELAY);
			return ins.line_no;
		}
		clock_cycles++;
		check_2(ins, ins.line_no);
		Addi(ins.reg[0], ins.reg[1], ins.val, ins.line_no);
		return ins.line_no + 1;
	}

	else{
		is_ins[ins.line_no] = false;
		return ins.line_no + 1;
	}
}


int Add(string reg1, string reg2, string reg3, int line_no){
	int r1 = inArray(reg1, registers, 32) - 1;
	int r2 = inArray(reg2, registers, 32) - 1;
	int r3 = inArray(reg3, registers, 32) - 1;
	isZero(r1, line_no) ;
	reg_values[r1] = reg_values[r2] + reg_values[r3];
	return 0;
}

int Sub(string reg1, string reg2, string reg3, int line_no){
	int r1 = inArray(reg1, registers, 32) - 1;
	int r2 = inArray(reg2, registers, 32) - 1;
	int r3 = inArray(reg3, registers, 32) - 1;
	isZero(r1, line_no);
	reg_values[r1] = reg_values[r2] - reg_values[r3];
	return 0;
}

int Mul(string reg1, string reg2, string reg3, int line_no){
	int r1 = inArray(reg1, registers, 32) - 1;
	int r2 = inArray(reg2, registers, 32) - 1;
	int r3 = inArray(reg3, registers, 32) - 1;
	isZero(r1, line_no);
	reg_values[r1] = reg_values[r2] * reg_values[r3];
	return 0;
}

int Beq(string reg1, string reg2){
	int r1 = inArray(reg1, registers, 32) - 1;
	int r2 = inArray(reg2, registers, 32) - 1;
	if(reg_values[r1] == reg_values[r2]){
		return 1;
	}
	return 0;
}

int Bne(string reg1, string reg2){
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
	isZero(r1, line_no) ;
	if(reg_values[r2] < reg_values[r3]){
		reg_values[r1] = 1;
		return 0;
	}
	else{
		reg_values[r1] = 0;
		return 0;
	}
}

////////////////////////////////////////////////////////////////////
			// MODIFIED FOR REORDERING AND QUEUE
////////////////////////////////////////////////////////////////////
int Lw(string reg1, string reg2, int reg2_val, int offset, int line_no, int ROW_ACCESS_DELAY, int COL_ACCESS_DELAY){
	int r1 = inArray(reg1, registers, 32) - 1;
	int r2 = inArray(reg2, registers, 32) - 1;
	isZero(r1, line_no) ;
	if(reg2_val + offset >= pow(2.0, 20.0)){
		cout << "Memory address out of bounds, line no: " << line_no + 1 << endl;
		exit(-1);
	}
	else if(reg2_val + offset < data_address){
		cout << "Invalid memory address, line no: " << line_no + 1 << endl;
		exit(-1);
	}
	else if((reg2_val + offset)%4 != 0){
		cout << "Invalid memory address, line no: " << line_no + 1 << endl;
		exit(-1);
	}
	else{
		on_hold = true;
		int row = (reg2_val + offset)/1024;
		int col = (reg2_val + offset)%1024;
		int x = reg2_val + offset;
		int y = x + 3;
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
			for(int p = 0; p < 32; p++){
				prev_reg_values[p] = reg_values[p];
			}
			return 0;
		}
		else if(row == cur_row){
			reg_values[r1] = row_buffer[col];
			clock_cycles = clock_cycles + COL_ACCESS_DELAY;
			onhold_output =  "cycle " + std::to_string(prev_clock_cycles + 1) + "-" + std::to_string(clock_cycles) + ": " + reg1 + "="  + std::to_string(reg_values[r1])+ ", Row buffer not updated"+ ", Accessed address " + std::to_string(x) + " from row buffer\n\n";
			max_cycle = clock_cycles;
			clock_cycles = prev_clock_cycles;
			for(int p = 0; p < 32; p++){
				prev_reg_values[p] = reg_values[p];
			}
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
			for(int p = 0; p < 32; p++){
				prev_reg_values[p] = reg_values[p];
			}
			return 0;
		}
	}	
}
////////////////////////////////////////////////////////////////////
			// MODIFIED FOR REORDERING AND QUEUE
////////////////////////////////////////////////////////////////////
int Sw(string reg1, string reg2, int reg1_val, int reg2_val, int offset, int line_no, int ROW_ACCESS_DELAY, int COL_ACCESS_DELAY){

	int r1 = inArray(reg1, registers, 32) - 1;
	int r2 = inArray(reg2, registers, 32) - 1;
	if(reg2_val + offset >= pow(2.0, 20.0)){
		cout << "Memory address out of bounds, line no: " << line_no + 1 << endl;
		exit(-1);
	}
	else if(reg2_val + offset < data_address){
		cout << "Invalid memory address, line no: " << line_no + 1 << endl;
		exit(-1);
	}
	else if((reg2_val + offset)%4 != 0){

		cout << "Invalid memory address, line no:" << line_no + 1 << endl;
		exit(-1);
	}
	
	else{
		int x = reg2_val + offset;
		int y = x + 3;
		on_hold = true;
		int row = (reg2_val + offset)/1024;
		int col = (reg2_val + offset)%1024;
		if (cur_row == -1){
			for(int i = 0; i < 1024; i++){
				row_buffer[i] = memory[row*1024 + i];
			}
			row_buffer[col] = reg1_val;
			cur_row = row;
			clock_cycles = clock_cycles + COL_ACCESS_DELAY + ROW_ACCESS_DELAY;
			onhold_output = "cycle " + std::to_string(prev_clock_cycles + 1) + "-" + std::to_string(clock_cycles) + ": " + "No register modified, Updated memory address " + std::to_string(x) + "-" + std::to_string(y) + " = " + std::to_string(row_buffer[col])+ ", Activated row " + std::to_string(row) + " and accessed address " + std::to_string(x) + "\n\n";
			max_cycle = clock_cycles;
			clock_cycles = prev_clock_cycles;
			buffer_updates = buffer_updates + 2;
			return 0;
		}
		else if(row == cur_row){
			row_buffer[col] = reg1_val;
			clock_cycles = clock_cycles + COL_ACCESS_DELAY;
			onhold_output = "cycle " + std::to_string(prev_clock_cycles + 1) + "-" + std::to_string(clock_cycles) + ": " + "No register modified, Updated memory address " + std::to_string(x) + "-" + std::to_string(y) + " = " + std::to_string(row_buffer[col]) + ", Accessed address " + std::to_string(x) + " from the row buffer\n\n";;
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
			row_buffer[col] = reg1_val;
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
	isZero(r1, line_no) ;
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
	exit(-1);
}
string getLine(vector<string> all_ins, int line_no){
	int i = 0;
	for(string it: all_ins){
		if (i == line_no){
			return it;
		}
		i++;
	}
	exit(-1);
}
void print_register(bool is_ins[], int temp){
	if(is_ins[temp] == true){
		cout << "Executed Instruction: " << getLine(all_ins, temp) << endl;
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
		exit(-1);
	}
	if(ins.num_val != "NULL"){
		cout << "Unexpected Integer(s) in the instruction in line: " << line_no + 1 << endl;
		exit(-1);
	}
	return true;
}

bool check_2(instruct ins, int line_no){
	if(ins.reg[0] == "None" || ins.reg[1] == "None" || ins.reg[2] != "None"){
		cout << "Invalid syntax in line:" << line_no + 1 << endl;
		exit(-1);
	}
	if(ins.num_val == "NULL"){
		cout << "Invalid syntax in line:" << line_no + 1 << endl;
		exit(-1);
	}
	return true;
}

bool check_3(instruct ins, int line_no){
	if (ins.reg[0] != "None" || ins.reg[1] != "None" || ins.reg[2] != "None"){
		cout << "Invalid syntax in line:" << line_no + 1 << endl; 
		exit(-1);
	}
	if(ins.num_val == "NULL"){
		cout << "Invalid syntax in line:" << line_no + 1 << endl;
		exit(-1);
	}
	return true;
}

int isZero(int r, int line_no){
	if(r == 0){
		cout << "Zero register can't be changed, line: " << line_no + 1 << endl; 
		exit(-1);
	}
	return 0;
}

////////////////////////////////////////////////////////////////////

				// NEW FUNCTIONS FOR ASSIGNMENT -4

////////////////////////////////////////////////////////////////////
bool check_4(instruct ins, int line_no){
	if(ins.reg[0] == "None" || ins.reg[1] == "None" || ins.reg[2] != "None"){
		cout << "Invalid syntax in line:" << line_no + 1 << endl;
		exit(-1);
	}
	if(ins.num_val != "NULL"){
		cout << "Invalid syntax in line:" << line_no + 1 << endl;
		exit(-1);
	}
	if(ins.label == "None"){
		cout << "Invalid syntax in line:" << line_no + 1 << endl;
		exit(-1);
	}
	return true;
}

bool check_5(instruct ins, int line_no){
	if(ins.reg[0] != "None" || ins.reg[1] != "None" || ins.reg[2] != "None"){
		cout << "Invalid syntax in line:" << line_no + 1 << endl;
		exit(-1);
	}
	if(ins.num_val != "NULL"){
		cout << "Invalid syntax in line:" << line_no + 1 << endl;
		exit(-1);
	}
	if(ins.label == "None"){
		cout << "Invalid syntax in line:" << line_no + 1 << endl;
		exit(-1);
	}
	return true;
}

bool is_independent(string reg[], int len, int id){
	if(len == 0 || queue_length == 0){
		return true;
	}
	for(int j = 0; j < queue_length; j++){
		if(ins_queue[j].ins_id == 8){
			if(id == 1 || id == 2 || id == 3 || id == 6 || id == 10 || id == 9){
				for(int i = 0; i < len; i++){
					if(ins_queue[j].reg[0] == reg[i]){
						return false;
					}
				}
				/*if(j != 0){
					if(ins_queue[j].reg[1] == reg[0]){
						return false;
					}
				}*/
			}
			else if(id == 8){
				if(ins_queue[j].reg[0] == reg[1]){
					return false;
				}
			}
			else if(id == 4 || id == 5){
				for(int i = 0; i < len; i++){
					if(ins_queue[j].reg[0] == reg[i]){
						return false;
					}
				}
			}
		}
		/*else if(ins_queue[j].ins_id == 9){
			//return true;
			if(j != 0){
				if(id == 1 || id == 2 || id == 3 || id == 6 || id == 10){
					if(ins_queue[j].reg[0] == reg[0] || ins_queue[j].reg[1] == reg[0]){
						return false;
					}
				}
			}
			//else if(id == 4 || id == 5){
				//return true;
			//}
		}*/
	}
	return true;
}

void execute_queue(int ROW_ACCESS_DELAY, int COL_ACCESS_DELAY, int id){
	//print_queue();
	cout << onhold_output;
	on_hold = false;
	clock_cycles = max_cycle;
	prev_clock_cycles = clock_cycles;
	if(queue_length > 1){
		for(int i = 1; i < queue_length; i++){
			ins_queue[i - 1] = ins_queue[i];
		}
		queue_length--;
		modify();
		check_2(ins_queue[0], ins_queue[0].line_no);
		//cout << ins_queue[0].line_no << endl;	
		if(ins_queue[0].ins_id == 8){
			Lw(ins_queue[0].reg[0], ins_queue[0].reg[1], ins_queue[0].reg2_val, ins_queue[0].val, ins_queue[0].line_no, ROW_ACCESS_DELAY, COL_ACCESS_DELAY);
		}
		else{
			Sw(ins_queue[0].reg[0], ins_queue[0].reg[1], ins_queue[0].reg1_val, ins_queue[0].reg2_val, ins_queue[0].val, ins_queue[0].line_no, ROW_ACCESS_DELAY, COL_ACCESS_DELAY);
		}
		if(id == 8){
			clock_cycles++;
			cout << "cycle " << clock_cycles << ": Memory address of the instruction: " << ins_queue[queue_length].line_no*4 << endl;
			cout << "Executed Instruction: " << "lw " << ins_queue[queue_length].reg[0] << ", " << ins_queue[queue_length].val << "(" << ins_queue[queue_length].reg[1] << ")" << endl;
			cout << "DRAM request issued" << endl;
			cout << endl;
			prev_clock_cycles = clock_cycles;
		}
		else if(id == 9){
			clock_cycles++;
			cout << "cycle " << clock_cycles << ": Memory address of the instruction: " << ins_queue[queue_length].line_no*4 << endl;
			cout << "Executed Instruction: " << "sw " << ins_queue[queue_length].reg[0] << ", " << ins_queue[queue_length].val << "(" << ins_queue[queue_length].reg[1] << ")" << endl;
			cout << "DRAM request issued" << endl;
			cout << endl;
			prev_clock_cycles = clock_cycles;
		}
	}
	else{
		max_cycle = -1;
		queue_length--;
	}
}

void print_queue(){
	cout << "[ ";
	for(int i = 0; i < queue_length; i++){
		cout << ins_queue[i].reg[0] << ", " << ins_queue[i].val << "(" << ins_queue[i].reg[1] << ")" << "; ";
	}
	cout << "]";
	cout << endl;
}

void modify(){
	instruct dummy_queue[queue_length];
	for(int i = 0; i < queue_length; i++){
		if(compute_row(ins_queue[i]) == cur_row){
			//cout << ins_queue[i].line_no  + 1<< " - " << cur_row << endl;
			reorder(i);
		}
	}
}

int compute_row(instruct ins){
	check_2(ins, ins.line_no);
	int row = (ins.reg2_val + ins.val)/1024;
	return row;
}

void reorder(int x){
	int i, p;
	//instruct temp = ins_queue[x];
	//print_queue();
	for(i = x - 1; i >= 0; i--){
		if(!independent_ins(i, x)){
			break;
		}
	}
	for(p = i + 1; p < x; p++){
		if(compute_row(ins_queue[p]) != cur_row){
			break;
		}
	}
	//cout << p << endl;
	instruct temp = ins_queue[p];
	for(int j = p; j < x; j++){
		instruct temp1 = ins_queue[j + 1];
		ins_queue[j + 1] = temp;
		temp = temp1;
	}
	ins_queue[p] = temp;
	//print_queue();
}

bool independent_ins(int x, int y){
	check_2(ins_queue[x], ins_queue[x].line_no);
	check_2(ins_queue[y], ins_queue[y].line_no);
	int r1 = inArray(ins_queue[x].reg[1], registers, 32) - 1;
	int r2 = inArray(ins_queue[y].reg[1], registers, 32) - 1;
	int offset_1 = ins_queue[x].val;
	int offset_2 = ins_queue[y].val;
	int memory_1 = reg_values[r1] + offset_1;
	int memory_2 = reg_values[r2] + offset_2;
	if(ins_queue[x].ins_id == 8){
		if(ins_queue[y].ins_id == 8){
			if(ins_queue[x].reg[0] == ins_queue[y].reg[0] || ins_queue[x].reg[0] == ins_queue[y].reg[1] || ins_queue[x].reg[1] == ins_queue[y].reg[0]){
				return false;
			}
		}
		else{
			if(ins_queue[x].reg[0] == ins_queue[y].reg[0] || ins_queue[x].reg[0] == ins_queue[y].reg[1] || memory_1 == memory_2){
				return false;
			}
		}
	}
	else{
		if(ins_queue[y].ins_id == 8){
			if(ins_queue[y].reg[0] == ins_queue[x].reg[0] || ins_queue[y].reg[0] == ins_queue[x].reg[1] || memory_1 == memory_2){
				return false;
			}
		}
		else{
			if(memory_1 == memory_2){
				return false;
			}
		}
	}
	return true;
}
