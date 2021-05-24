/**
 * @file ass5_part2.cpp
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

using namespace std;



struct instruct{
	// ins_id of add: 1; sub: 2; mul: 3; beq: 4; bne: 5; slt: 6; j:7; lw: 8; sw: 9; addi: 10
	int core_no;
	int ins_id = 0;
	string reg[3] = {"None", "None", "None"};
	int val = 0;
	string num_val = "NULL";
	string label = "None";
	int line_no;
	int reg1_val = 0;
	int reg2_val = 0;
	bool in_mrm = false;
	int in_cycle;
};

struct core
{
	vector<instruct> PC;
	int reg_values[32] = {0};
	int prev_reg_values[32] = {0};
	int data_address = 0;
	int clock_cycles = 0;
	int prev_clock_cycles = 0;
	bool on_hold = false;
	int max_cycle = -1;
	string onhold_output;
	vector<string> all_ins;
	map<string, int> label_dict;
	int start_address;
	int end_address;
	int ins_count = 0;
};

struct Memory
{
	int ROW_ACCESS_DELAY;
	int COL_ACCESS_DELAY;
	int simulation_time;
};

vector<core> CPU_CORES;
Memory DRAM;
int memory[(int)pow(2.0, 20.0)];
int queue_length = 0;
int row_buffer[1024] = {0};
int cur_row = -1;
int cur_address;
int buffer_updates = 0;
instruct ins_queue[128];
int MAX_LENGTH = 128;
int cur_core;
string cur_ins = "None";
int division = 0;
int no_cores = 0;
int running_cycle = 1;
int MRM_delay = 2;
int last_mrm;
string operations[10] = {"add", "sub", "mul", "beq", "bne", "slt", "j", "lw", "sw", "addi"};
string registers[32] = {"$zero", "$at", "$v0", "$v1", "$a0", "$a1", "$a2", "$a3", 
						"$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7", 
						"$s0", "$s1", "$s2", "$s3", "$s4", "$s5", "$s6", "$s7",
						"$t8", "$t9", "$k0", "$k1", "$gp", "$sp", "$fp", "$ra"};

int inArray(string word, string arr[], int size);
int execute();
int compute(int core_no, instruct ins);
int Add(string reg1, string reg2, string reg3, int line_no, int core_no);
int Sub(string reg1, string reg2, string reg3, int line_no, int core_no);
int Mul(string reg1, string reg2, string reg3, int line_no, int core_no);
int Beq(string reg1, string reg2, int core_no);
int Bne(string reg1, string reg2, int core_no);
int Slt(string reg1, string reg2, string reg3, int line_no, int core_no);
int Lw(int core_no, string reg1, string reg2, int reg2_val, int offset, int line_no, int in_cycle);
int Sw(int core_no, string reg1, string reg2, int reg1_val, int reg2_val, int offset, int line_no, int in_cycle);
int Addi(string reg1, string reg2, int val, int line_no, int core_no);
void print_register(int core_no, int temp);
instruct getInstruct(vector<instruct> PC, int line_no);
bool check_1(instruct ins, int line_no);
bool check_2(instruct ins, int line_no);
bool check_3(instruct ins, int line_no);
bool check_4(instruct ins, int line_no);
bool check_5(instruct ins, int line_no);
void initialize();
int isZero(int r, int line_no);
void print_memory();
bool is_independent(string reg[], int len, int id, int core_no);
string getLine(vector<string> all_ins, int line_no);
void execute_queue(int core_no, int id);
void modify();
bool independent_ins(int x, int y);
int compute_row(instruct ins);
void reorder(int x);
void print_queue();
///////////////////////////////////////////////////////////////////


					// MODIFIED MAIN FOR LABELS


//////////////////////////////////////////////////////////////////
void x(Memory DRAM){
	//cout << getLine(CPU_CORES[1].all_ins, 5) << endl;	
}

int main(int argc, char* argv[]){
	
	std::istringstream ss(argv[1]);
	if (!(ss >> no_cores)) {
		cout << "Invalid number: " << argv[1] << '\n';
		return -1;
	}
	int simulation_time;
	std::istringstream ss0(argv[2]);
	if (!(ss0 >> simulation_time)) {
		cout << "Invalid number: " << argv[2] << '\n';
		return -1;
	}

	fstream input_files[no_cores];
	for(int i = 0; i < no_cores; i++){
		input_files[i].open(argv[3 + i]);
	}
	int ROW_ACCESS_DELAY;
	std::istringstream ss2(argv[no_cores + 3]);
	if (!(ss2 >> ROW_ACCESS_DELAY)) {
		cout << "Invalid number: " << argv[no_cores + 3] << '\n';
		return -1;
	} 
	int COL_ACCESS_DELAY;
	std::istringstream ss1(argv[no_cores + 4]);
	if (!(ss1 >> COL_ACCESS_DELAY)) {
		cout << "Invalid number: " << argv[no_cores + 4] << '\n';
		return -1;
	}

	
	division = (int)pow(2.0, 20.0)/no_cores;
	DRAM.COL_ACCESS_DELAY = COL_ACCESS_DELAY;
	DRAM.ROW_ACCESS_DELAY = ROW_ACCESS_DELAY;
	DRAM.simulation_time = simulation_time;
	cout << DRAM.simulation_time << endl;
	//core CPU_CORES[no_cores];
	//cout << DRAM.ROW_ACCESS_DELAY << endl;
	for(int file_no = 0; file_no < no_cores; file_no++){
		core CPU;
		string str;
		string word;
		int start = 0;
		int line_num = 0;
		while(getline(input_files[file_no], str, '\n')){
			stringstream ss(str);
			instruct line;
			int i = 0;
			int val_start = 0;
			int ins_id = 0;
			while (ss >> word){
				//cout << file_no << endl;
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
						auto itr = (CPU.label_dict).find(label);
						if(itr != (CPU.label_dict).end()){
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
						(CPU.label_dict).insert({label, line_num});
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
			if(i == 0){
				line.ins_id = 11;
			}
			CPU.data_address += 4;
			line.core_no = file_no;
			line.line_no = line_num;
			(CPU.PC).push_back(line);
			(CPU.all_ins).push_back(str);
			line_num++;
		}
		CPU.start_address = division*file_no - ((division*file_no)%4);
		CPU.end_address = division*(file_no + 1) - ((division*(file_no + 1))%4) - 1;
		CPU_CORES.push_back(CPU);
	}
	execute();
	/*int up = 0;
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
	cout << "Clock Cycles: " << clock_cycles << endl;*/
	cout << endl;
	print_memory();
	//cout << "Total Buffer Updates: " << buffer_updates << endl;
	return 0;
	//return 0;
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

int execute(){
	//cout << "here" << endl;
	//cout << getLine(CPU_CORES.at(1).all_ins, 5) << endl;
	//cout << DRAM.ROW_ACCESS_DELAY << endl;	
	int temp;
	int line_no[no_cores];
	for(int i = 0; i < no_cores; i++){
		line_no[i] = 0;
	}
	while(running_cycle <= DRAM.simulation_time){
		//cout << "hii" << endl;
		int flag = 0;
		for(int core_no = 0; core_no < no_cores; core_no++){
			if(line_no[core_no] >= (CPU_CORES.at(core_no).data_address/4)){
				continue;
			}
			instruct ins = getInstruct(CPU_CORES.at(core_no).PC, line_no[core_no]);
			if(ins.ins_id == 11 || ins.ins_id == 0){
				line_no[core_no]++;
				flag = 1;
				break;
			}
		}
		if(flag == 1){
			continue;
		}

		cout << "\e[1m" << "\x1B[31mCYCLE: " << running_cycle << "\e[0m" << "\e[0m" << endl;
		for(int core_no = 0; core_no < no_cores; core_no++){
			cout << "\e[1m" << "\e[34m" << "Core: " << core_no + 1 << "\e[0m" << "\e[0m" << endl;
			if(line_no[core_no] < (CPU_CORES.at(core_no).data_address/4)){
				temp = line_no[core_no];
				instruct ins = getInstruct(CPU_CORES.at(core_no).PC, line_no[core_no]);
				line_no[core_no] = compute(core_no, ins);
				if(ins.ins_id == 8 || ins.ins_id == 9){
					//core_no = core_no - 1;
					continue;
				}

				if(temp != line_no[core_no]){
					if(CPU_CORES.at(core_no).clock_cycles - CPU_CORES.at(core_no).prev_clock_cycles == 1){
						//cout << "\e[1m" << "\e[34m" << "Core: " << core_no + 1 << ": " << "\e[0m" << "\e[0m" << endl;
					}
					cout << "Memory address of the instruction: " << ins.line_no*4 << endl;
					print_register(core_no, temp);
					for (int j = 0; j < 32; j++){
						CPU_CORES.at(core_no).prev_reg_values[j] = CPU_CORES.at(core_no).reg_values[j];
					}
					CPU_CORES.at(core_no).prev_clock_cycles = CPU_CORES.at(core_no).clock_cycles;
				}
			}
			else{
				//cout << "\e[1m" << "\e[34m" << "Core: " << core_no + 1 << "\e[0m" << "\e[0m" << endl;
				//CPU_CORES.at(core_no).clock_cycles++;
				if(CPU_CORES.at(core_no).on_hold == true && running_cycle == CPU_CORES.at(core_no).max_cycle){
					execute_queue(core_no, 0);
					CPU_CORES.at(core_no).clock_cycles--;
					CPU_CORES.at(core_no).prev_clock_cycles--;
				}
				/*else if(CPU_CORES.at(core_no).on_hold == true){
					cout << "Waiting for DRAM completion " << endl << endl;
				}*/
				else{
					cout << "No Activity " << endl << endl;
				}
				//cout << endl;
				CPU_CORES.at(core_no).clock_cycles++;
				CPU_CORES.at(core_no).prev_clock_cycles++;
			}
			//cout << CPU_CORES.at(core_no).clock_cycles << endl;
			/*int temp_length = queue_length;
			for(int i = 0; i < temp_length; i++){
				execute_queue(CPU_CORES, core_no, DRAM, 0);
			}	
			if(on_hold == true){
				cout << CPU.onhold_output;
				CPU.clock_cycles = CPU.max_cycle;
				CPU.on_hold = false;
			}*/
		}
		//cout << CPU_CORES.at(0).clock_cycles;
		/*cout << "\e[1m" << "\e[34m" << "DRAM:" << "\e[0m" << "\e[0m" << endl;
		if(queue_length > 0){
			cout << "DRAM processing request" << endl << endl;
		}
		else{
			cout << "No DRAM requests" << endl << endl;
		}*/
		running_cycle++;
	}
	cout << "\e[1m" << "\x1B[31m" << "Instructions count" << "\e[0m" << "\e[0m" << endl;
	for(int i = 0; i < no_cores; i++){
		cout << "\e[1m" << "\e[34m" << "Core - " << i + 1 << ": " << "\e[0m" << "\e[0m" << CPU_CORES.at(i).ins_count << endl;
	}
	//cout << "\e[1m" << "\x1B[31mCYCLE: " << "Buffer Updates" << "\e[0m" << "\e[0m" << ": " << buffer_updates << endl;
	return 0;
}

///////////////////////////////////////////////////////////////////

					// MODIFIED FOR QUEUE

///////////////////////////////////////////////////////////////////
int compute(int core_no, instruct ins){
	//core CPU = CPU_CORES.at(core_no);
	if(ins.ins_id == 1){
		bool is_ind = is_independent(ins.reg, 3, ins.ins_id, ins.core_no);
		if(is_ind == false /*|| running_cycle - 1 == CPU_CORES.at(core_no).max_cycle*/){
			execute_queue(core_no, ins.ins_id);
			//return compute(ins, line_no, is_ins, ROW_ACCESS_DELAY, COL_ACCESS_DELAY);
			return ins.line_no;
		}
		if(running_cycle == CPU_CORES.at(core_no).max_cycle){
			
			if(cur_core == core_no && cur_ins == "lw"){
				execute_queue(core_no, ins.ins_id);
				return ins.line_no;
			}
			execute_queue(core_no, ins.ins_id);
			CPU_CORES.at(core_no).clock_cycles--;
		}
		CPU_CORES.at(core_no).clock_cycles++;
		CPU_CORES.at(core_no).ins_count++;
		check_1(ins, ins.line_no);
		Add(ins.reg[0], ins.reg[1], ins.reg[2], ins.line_no, core_no);
		return ins.line_no + 1;
	}

	else if(ins.ins_id == 2){
		bool is_ind = is_independent(ins.reg, 3, ins.ins_id, ins.core_no);
		if(is_ind == false /*|| running_cycle - 1 == CPU_CORES.at(core_no).max_cycle*/){
			execute_queue(core_no, ins.ins_id);
			//return compute(ins, line_no, is_ins, ROW_ACCESS_DELAY, COL_ACCESS_DELAY);
			return ins.line_no;
		}
		if(running_cycle == CPU_CORES.at(core_no).max_cycle){
			
			if(cur_core == core_no && cur_ins == "lw"){
				execute_queue(core_no, ins.ins_id);
				return ins.line_no;
			}
			execute_queue(core_no, ins.ins_id);
			CPU_CORES.at(core_no).clock_cycles--;
		}
		CPU_CORES.at(core_no).clock_cycles++;
		CPU_CORES.at(core_no).ins_count++;
		check_1(ins, ins.line_no);
		Sub(ins.reg[0], ins.reg[1], ins.reg[2], ins.line_no, core_no);
		return ins.line_no + 1;
		
	}

	else if(ins.ins_id == 3){
		bool is_ind = is_independent(ins.reg, 3, ins.ins_id, ins.core_no);
		if(is_ind == false /*|| running_cycle - 1 == CPU_CORES.at(core_no).max_cycle*/){
			execute_queue(core_no, ins.ins_id);
			//return compute(ins, line_no, is_ins, ROW_ACCESS_DELAY, COL_ACCESS_DELAY);
			return ins.line_no;
		}
		if(running_cycle == CPU_CORES.at(core_no).max_cycle){
			
			if(cur_core == core_no && cur_ins == "lw"){
				execute_queue(core_no, ins.ins_id);
				return ins.line_no;
			}
			execute_queue(core_no, ins.ins_id);
			CPU_CORES.at(core_no).clock_cycles--;
		}
		CPU_CORES.at(core_no).clock_cycles++;
		CPU_CORES.at(core_no).ins_count++;
		bool go = check_1(ins, ins.line_no);
		Mul(ins.reg[0], ins.reg[1], ins.reg[2], ins.line_no, core_no);
		return ins.line_no + 1;
	}

	else if(ins.ins_id == 4){
		bool is_ind = is_independent(ins.reg, 2, ins.ins_id, ins.core_no);
		if(is_ind == false /*|| running_cycle - 1 == CPU_CORES.at(core_no).max_cycle*/){
			execute_queue(core_no, ins.ins_id);
			//return compute(ins, line_no, is_ins, ROW_ACCESS_DELAY, COL_ACCESS_DELAY);
			return ins.line_no;
		}
		if(running_cycle == CPU_CORES.at(core_no).max_cycle){
			execute_queue(core_no, ins.ins_id);
			CPU_CORES.at(core_no).clock_cycles--;
		}
		CPU_CORES.at(core_no).clock_cycles++;
		CPU_CORES.at(core_no).ins_count++;
		check_4(ins, ins.line_no);
		int x = Beq(ins.reg[0], ins.reg[1], core_no);
		auto itr = CPU_CORES.at(core_no).label_dict.find(ins.label);
		if(itr == CPU_CORES.at(core_no).label_dict.end()){
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
		bool is_ind = is_independent(ins.reg, 2, ins.ins_id, ins.core_no);
		if(is_ind == false /*|| running_cycle - 1 == CPU_CORES.at(core_no).max_cycle*/){
			execute_queue(core_no, ins.ins_id);
			//return compute(ins, line_no, is_ins, ROW_ACCESS_DELAY, COL_ACCESS_DELAY);
			return ins.line_no;
		}
		if(running_cycle == CPU_CORES.at(core_no).max_cycle){
			execute_queue(core_no, ins.ins_id);
			CPU_CORES.at(core_no).clock_cycles--;
		}
		CPU_CORES.at(core_no).clock_cycles++;
		CPU_CORES.at(core_no).ins_count++;
		check_4(ins, ins.line_no);
		int x = Bne(ins.reg[0], ins.reg[1], core_no);
		auto itr = CPU_CORES.at(core_no).label_dict.find(ins.label);
		if(itr == CPU_CORES.at(core_no).label_dict.end()){
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
		bool is_ind = is_independent(ins.reg, 3, ins.ins_id, ins.core_no);
		if(is_ind == false /*|| running_cycle - 1 == CPU_CORES.at(core_no).max_cycle*/){
			execute_queue(core_no, ins.ins_id);
			//return compute(ins, line_no, is_ins, ROW_ACCESS_DELAY, COL_ACCESS_DELAY);
			return ins.line_no;
		}
		if(running_cycle == CPU_CORES.at(core_no).max_cycle){
			
			if(cur_core == core_no && cur_ins == "lw"){
				execute_queue(core_no, ins.ins_id);
				return ins.line_no;
			}
			execute_queue(core_no, ins.ins_id);
			CPU_CORES.at(core_no).clock_cycles--;
		}
		CPU_CORES.at(core_no).clock_cycles++;
		CPU_CORES.at(core_no).ins_count++;
		check_1(ins, ins.line_no);
		Slt(ins.reg[0], ins.reg[1], ins.reg[2], ins.line_no, core_no);
		return ins.line_no + 1;
	}

	else if(ins.ins_id == 7){
		bool is_ind = is_independent(ins.reg, 0, ins.ins_id, ins.core_no);
		if(is_ind == false /*|| running_cycle - 1 == CPU_CORES.at(core_no).max_cycle*/){
			execute_queue(core_no, ins.ins_id);
			//return compute(ins, line_no, is_ins, ROW_ACCESS_DELAY, COL_ACCESS_DELAY);
			return ins.line_no;
		}
		if(running_cycle == CPU_CORES.at(core_no).max_cycle){
			execute_queue(core_no, ins.ins_id);
			CPU_CORES.at(core_no).clock_cycles--;
		}
		CPU_CORES.at(core_no).clock_cycles++;
		CPU_CORES.at(core_no).ins_count++;
		check_5(ins, ins.line_no);
		auto itr = CPU_CORES.at(core_no).label_dict.find(ins.label);
		if(itr == CPU_CORES.at(core_no).label_dict.end()){
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
		if(is_independent(ins.reg, 2, 8, ins.core_no) == false || queue_length == MAX_LENGTH){
			execute_queue(core_no, 0);			
			return ins.line_no;	
		}
		
		int r2 = inArray(ins.reg[1], registers, 32) - 1;
		ins.reg2_val = CPU_CORES.at(core_no).reg_values[r2];
		int flag = 0;
		/*for(int itr = queue_length - 1; itr > 0; itr--){
			if(ins_queue[itr].ins_id == ins.ins_id && ins_queue[itr].reg[0] == ins.reg[0] && ins_queue[itr].core_no == ins.core_no){
				flag = 1;
				ins_queue[itr] = ins;
				break;
			}
			else if(ins_queue[itr].core_no == ins.core_no){
				break;
			}
		}*/
		
		if(flag == 0){
			ins_queue[queue_length] = ins;
			queue_length++;
		}
		ins.in_cycle = running_cycle;
		if(queue_length == 1){
			CPU_CORES.at(core_no).clock_cycles++;
			//cout << "\e[1m" << "\e[34m"<< "Core: " << core_no + 1 << "\e[0m" << "\e[0m"<< endl;
			cout << /*"cycle " << CPU_CORES.at(core_no).clock_cycles <<*/ "Memory address of the instruction: " << ins.line_no*4 << endl;
			cout << "Executed Instruction: " << "lw " << ins.reg[0] << ", " << ins.val << "(" << ins.reg[1] << ")" << endl;
			cout << "Added to MRM wait buffer" << endl;
			cout << endl;
			CPU_CORES.at(core_no).prev_clock_cycles = CPU_CORES.at(core_no).clock_cycles;
			check_2(ins_queue[0], ins_queue[0].line_no);
			//ins.in_cycle = CPU_CORES.at(core_no).clock_cycles + 1;
			Lw(core_no, ins_queue[0].reg[0], ins_queue[0].reg[1], ins_queue[0].reg2_val, ins_queue[0].val, ins_queue[0].line_no, ins.in_cycle);
			return ins.line_no + 1;
		}
		if(CPU_CORES.at(core_no).on_hold == true && running_cycle == CPU_CORES.at(core_no).max_cycle){
			execute_queue(core_no, ins.ins_id);			
			return ins.line_no + 1;	
		}
		CPU_CORES.at(core_no).clock_cycles++;
		//cout << "\e[1m" << "\e[34m"<< "Core: " << core_no + 1 << "\e[0m" << "\e[0m"<< endl;
		cout << /*"cycle " << CPU_CORES.at(core_no).clock_cycles << */"Memory address of the instruction: " << ins.line_no*4 << endl;
		cout << "Executed Instruction: " << "lw " << ins.reg[0] << ", " << ins.val << "(" << ins.reg[1] << ")" << endl;
		cout << "Added to MRM wait buffer" << endl;
		cout << endl;
		CPU_CORES.at(core_no).prev_clock_cycles = CPU_CORES.at(core_no).clock_cycles;
		//ins.in_cycle = CPU_CORES.at(core_no).clock_cycles + 1;
		return ins.line_no + 1;
	}

	else if(ins.ins_id == 9){
		if(is_independent(ins.reg, 2, 9, ins.core_no) == false || queue_length == MAX_LENGTH){
			execute_queue(core_no, 0);			
			return ins.line_no;	
		}
		int r1 = inArray(ins.reg[0], registers, 32) - 1;
		int r2 = inArray(ins.reg[1], registers, 32) - 1;
		ins.reg1_val = CPU_CORES.at(core_no).reg_values[r1];
		ins.reg2_val = CPU_CORES.at(core_no).reg_values[r2];
		int queue_val = CPU_CORES.at(core_no).reg_values[r1];
		int flag = 0;
		/*for(int itr = queue_length - 1; itr > 0; itr--){
			if(ins.core_no == ins_queue[itr].core_no){
				
			}
		}*/
		if(flag == 0){
			ins_queue[queue_length] = ins;
			queue_length++;
		}
		ins.in_cycle = running_cycle;
		if(queue_length == 1){
			CPU_CORES.at(core_no).clock_cycles++;
			//cout << "\e[1m" << "\e[34m"<< "Core: " << core_no + 1 << "\e[0m" << "\e[0m"<< endl;
			cout << "cycle " << CPU_CORES.at(core_no).clock_cycles << "Memory address of the instruction: " << ins.line_no*4 << endl;
			cout << "Executed Instruction: " << "sw " << ins.reg[0] << ", " << ins.val << "(" << ins.reg[1] << ")" << endl;
			cout << "Added to MRM wait buffer" << endl;
			cout << endl;
			CPU_CORES.at(core_no).prev_clock_cycles = CPU_CORES.at(core_no).clock_cycles;
			//ins.in_cycle = CPU_CORES.at(core_no).clock_cycles + 1;
			check_2(ins_queue[0], ins_queue[0].line_no);
			//Sw(CPU, core_no, DRAM.ins_queue[0].reg[0], DRAM.ins_queue[0].reg[1], DRAM.ins_queue[0].reg1_val, DRAM.ins_queue[0].reg2_val, DRAM.ins_queue[0].val, DRAM.ins_queue[0].line_no, DRAM);
			Sw(core_no, ins_queue[0].reg[0], ins_queue[0].reg[1], ins_queue[0].reg1_val, ins_queue[0].reg2_val, ins_queue[0].val, ins_queue[0].line_no, ins.in_cycle);
			return ins.line_no + 1;
		}
		if(CPU_CORES.at(core_no).on_hold == true && running_cycle == CPU_CORES.at(core_no).max_cycle){
			execute_queue(core_no, ins.ins_id);			
			return ins.line_no + 1;	
		}
		CPU_CORES.at(core_no).clock_cycles++;
		//cout << "\e[1m" << "\e[34m"<< "Core: " << core_no + 1 << "\e[0m" << "\e[0m"<< endl;
		cout << "cycle " << CPU_CORES.at(core_no).clock_cycles << "Memory address of the instruction: " << ins.line_no*4 << endl;
		cout << "Executed Instruction: " << "sw " << ins.reg[0] << ", " << ins.val << "(" << ins.reg[1] << ")" << endl;
		cout << "Added to MRM wait buffer" << endl;
		cout << endl;
		CPU_CORES.at(core_no).prev_clock_cycles = CPU_CORES.at(core_no).clock_cycles;
		//ins.in_cycle = CPU_CORES.at(core_no).clock_cycles + 1;
		return ins.line_no + 1;
		
	}

	else if(ins.ins_id == 10){
		bool is_ind = is_independent(ins.reg, 2, ins.ins_id, ins.core_no);
		if(is_ind == false /*|| running_cycle - 1 == CPU_CORES.at(core_no).max_cycle*/){
			execute_queue(core_no, ins.ins_id);
			//return compute(ins, line_no, is_ins, ROW_ACCESS_DELAY, COL_ACCESS_DELAY);
			return ins.line_no;
		}
		if(running_cycle == CPU_CORES.at(core_no).max_cycle){
			
			if(cur_core == core_no && cur_ins == "lw"){
				execute_queue(core_no, ins.ins_id);
				return ins.line_no;
			}
			execute_queue(core_no, ins.ins_id);
			CPU_CORES.at(core_no).clock_cycles--;
		}
		CPU_CORES.at(core_no).clock_cycles++;
		CPU_CORES.at(core_no).ins_count++;
		check_2(ins, ins.line_no);
		Addi(ins.reg[0], ins.reg[1], ins.val, ins.line_no, core_no);
		return ins.line_no + 1;
	}

	else{
		return ins.line_no + 1;
	}
}


int Add(string reg1, string reg2, string reg3, int line_no, int core_no){
	int r1 = inArray(reg1, registers, 32) - 1;
	int r2 = inArray(reg2, registers, 32) - 1;
	int r3 = inArray(reg3, registers, 32) - 1;
	isZero(r1, line_no) ;
	CPU_CORES.at(core_no).reg_values[r1] = CPU_CORES.at(core_no).reg_values[r2] + CPU_CORES.at(core_no).reg_values[r3];
	return 0;
}

int Sub(string reg1, string reg2, string reg3, int line_no, int core_no){
	int r1 = inArray(reg1, registers, 32) - 1;
	int r2 = inArray(reg2, registers, 32) - 1;
	int r3 = inArray(reg3, registers, 32) - 1;
	isZero(r1, line_no);
	CPU_CORES.at(core_no).reg_values[r1] = CPU_CORES.at(core_no).reg_values[r2] - CPU_CORES.at(core_no).reg_values[r3];
	return 0;
}

int Mul(string reg1, string reg2, string reg3, int line_no, int core_no){
	int r1 = inArray(reg1, registers, 32) - 1;
	int r2 = inArray(reg2, registers, 32) - 1;
	int r3 = inArray(reg3, registers, 32) - 1;
	isZero(r1, line_no);
	CPU_CORES.at(core_no).reg_values[r1] = CPU_CORES.at(core_no).reg_values[r2] * CPU_CORES.at(core_no).reg_values[r3];
	return 0;
}

int Beq(string reg1, string reg2, int core_no){
	int r1 = inArray(reg1, registers, 32) - 1;
	int r2 = inArray(reg2, registers, 32) - 1;
	if(CPU_CORES.at(core_no).reg_values[r1] == CPU_CORES.at(core_no).reg_values[r2]){
		return 1;
	}
	return 0;
}

int Bne(string reg1, string reg2, int core_no){
	int r1 = inArray(reg1, registers, 32) - 1;
	int r2 = inArray(reg2, registers, 32) - 1;
	if(CPU_CORES.at(core_no).reg_values[r1] != CPU_CORES.at(core_no).reg_values[r2]){
		return 1;
	}
	return 0;
}

int Slt(string reg1, string reg2, string reg3, int line_no, int core_no){
	int r1 = inArray(reg1, registers, 32) - 1;
	int r2 = inArray(reg2, registers, 32) - 1;
	int r3 = inArray(reg3, registers, 32) - 1;
	isZero(r1, line_no) ;
	if(CPU_CORES.at(core_no).reg_values[r2] < CPU_CORES.at(core_no).reg_values[r3]){
		CPU_CORES.at(core_no).reg_values[r1] = 1;
		return 0;
	}
	else{
		CPU_CORES.at(core_no).reg_values[r1] = 0;
		return 0;
	}
}

int Lw(int core_no, string reg1, string reg2, int reg2_val, int offset, int line_no, int in_cycle){
	int r1 = inArray(reg1, registers, 32) - 1;
	int r2 = inArray(reg2, registers, 32) - 1;
	isZero(r1, line_no) ;
	if(reg2_val + offset + CPU_CORES.at(core_no).start_address >= CPU_CORES.at(core_no).end_address){
		cout << "Memory address out of bounds, line no: " << line_no + 1 << " in file no: " << core_no << endl;
		exit(-1);
	}
	else if(reg2_val + offset + CPU_CORES.at(core_no).start_address < CPU_CORES.at(core_no).start_address){
		cout << "Invalid memory address, line no: " << line_no + 1 << " in file no: " << core_no << endl;
		exit(-1);
	}
	else if((reg2_val + offset + CPU_CORES.at(core_no).start_address)%4 != 0){
		cout << "Invalid memory address, line no: " << line_no + 1 << " in file no: " << core_no << endl;
		exit(-1);
	}
	else{
		CPU_CORES.at(core_no).on_hold = true;
		cur_core = core_no;
		int row = (reg2_val + offset + CPU_CORES.at(core_no).start_address)/1024;
		int col = (reg2_val + offset + CPU_CORES.at(core_no).start_address)%1024;
		int x = reg2_val + offset + CPU_CORES.at(core_no).start_address;
		int y = x + 3;
		if(cur_address == row + col && cur_ins == "sw"){
			CPU_CORES.at(core_no).reg_values[r1] = row_buffer[col];
			CPU_CORES.at(core_no).clock_cycles = CPU_CORES.at(core_no).clock_cycles + 1;
			CPU_CORES.at(core_no).onhold_output = "Forwarding from sw to lw: " + reg1 + "="  + std::to_string(CPU_CORES.at(core_no).reg_values[r1]) + "\n\n";
			CPU_CORES.at(core_no).max_cycle = CPU_CORES.at(core_no).clock_cycles;
			CPU_CORES.at(core_no).clock_cycles = CPU_CORES.at(core_no).prev_clock_cycles;
			cur_ins = "lw";
			cur_address = reg2_val + offset + CPU_CORES.at(core_no).start_address;
			return 0;
		}
		cur_ins = "lw";
		cur_address = reg2_val + offset + CPU_CORES.at(core_no).start_address;
		if (cur_row == -1){
			for(int i = 0; i < 1024; i++){
				row_buffer[i] = memory[row*1024 + i];
			}
			CPU_CORES.at(core_no).reg_values[r1] = row_buffer[col];
			cur_row = row;
			if(CPU_CORES.at(core_no).prev_clock_cycles == in_cycle){
				CPU_CORES.at(core_no).clock_cycles = CPU_CORES.at(core_no).clock_cycles + DRAM.COL_ACCESS_DELAY + DRAM.ROW_ACCESS_DELAY + MRM_delay;
				CPU_CORES.at(core_no).onhold_output =  "cycle " + std::to_string(CPU_CORES.at(core_no).prev_clock_cycles + 1) + "-" + std::to_string(CPU_CORES.at(core_no).prev_clock_cycles + MRM_delay) + ": " + "MRM to DRAM\n" +
				/*"Core: " + std::to_string(core_no) +*/ "cycle " + std::to_string(CPU_CORES.at(core_no).prev_clock_cycles + 1 + MRM_delay) + "-" + std::to_string(CPU_CORES.at(core_no).clock_cycles) + ": " + reg1 + "="  + std::to_string(CPU_CORES.at(core_no).reg_values[r1])+", Row buffer not updated" + ", Activated row " + std::to_string(row) + " and accessed address " + std::to_string(x) + "\n\n";
				CPU_CORES.at(core_no).max_cycle = CPU_CORES.at(core_no).clock_cycles;
				CPU_CORES.at(core_no).clock_cycles = CPU_CORES.at(core_no).prev_clock_cycles;
				last_mrm = CPU_CORES.at(core_no).prev_clock_cycles + MRM_delay;
			}
			else{
				int start = std::max(last_mrm + 1, CPU_CORES.at(core_no).prev_clock_cycles - 2);
				CPU_CORES.at(core_no).clock_cycles = start + DRAM.COL_ACCESS_DELAY + DRAM.ROW_ACCESS_DELAY + 2;
				CPU_CORES.at(core_no).onhold_output =  "cycle " + std::to_string(start) + "-" + std::to_string(start + 2) + ": " + "MRM to DRAM\n" +
				/*"Core: " + std::to_string(core_no) +*/ "cycle " + std::to_string(start + 3) + "-" + std::to_string(CPU_CORES.at(core_no).clock_cycles) + ": " + reg1 + "="  + std::to_string(CPU_CORES.at(core_no).reg_values[r1])+", Row buffer not updated" + ", Activated row " + std::to_string(row) + " and accessed address " + std::to_string(x) + "\n\n";
				CPU_CORES.at(core_no).max_cycle = CPU_CORES.at(core_no).clock_cycles;
				CPU_CORES.at(core_no).clock_cycles = CPU_CORES.at(core_no).prev_clock_cycles;
				last_mrm = start + 2;
			}
			//CPU_CORES.at(core_no).clock_cycles = CPU_CORES.at(core_no).clock_cycles + DRAM.COL_ACCESS_DELAY + DRAM.ROW_ACCESS_DELAY;
			//CPU_CORES.at(core_no).onhold_output =  /*"Core: " + std::to_string(core_no) +*/ "cycle " + std::to_string(CPU_CORES.at(core_no).prev_clock_cycles + 1) + "-" + std::to_string(CPU_CORES.at(core_no).clock_cycles) + ": " + reg1 + "="  + std::to_string(CPU_CORES.at(core_no).reg_values[r1])+", Row buffer not updated" + ", Activated row " + std::to_string(row) + " and accessed address " + std::to_string(x) + "\n\n";
			//CPU_CORES.at(core_no).max_cycle = CPU_CORES.at(core_no).clock_cycles;
			//CPU_CORES.at(core_no).clock_cycles = CPU_CORES.at(core_no).prev_clock_cycles;
			buffer_updates = buffer_updates + 1;
			for(int p = 0; p < 32; p++){
				CPU_CORES.at(core_no).prev_reg_values[p] = CPU_CORES.at(core_no).reg_values[p];
			}
			return 0;
		}
		else if(row == cur_row){
			CPU_CORES.at(core_no).reg_values[r1] = row_buffer[col];
			if(CPU_CORES.at(core_no).prev_clock_cycles == in_cycle){
				CPU_CORES.at(core_no).clock_cycles = CPU_CORES.at(core_no).clock_cycles + DRAM.COL_ACCESS_DELAY + MRM_delay;
				CPU_CORES.at(core_no).onhold_output =  "cycle " + std::to_string(CPU_CORES.at(core_no).prev_clock_cycles + 1) + "-" + std::to_string(CPU_CORES.at(core_no).prev_clock_cycles + MRM_delay) + ": " + "MRM to DRAM\n" +
				/*"Core: " + std::to_string(core_no) +*/ "cycle " + std::to_string(CPU_CORES.at(core_no).prev_clock_cycles + 1 + MRM_delay) + "-" + std::to_string(CPU_CORES.at(core_no).clock_cycles) + ": " + reg1 + "="  + std::to_string(CPU_CORES.at(core_no).reg_values[r1])+ ", Row buffer not updated"+ ", Accessed address " + std::to_string(x) + " from row buffer\n\n";
				CPU_CORES.at(core_no).max_cycle = CPU_CORES.at(core_no).clock_cycles;
				CPU_CORES.at(core_no).clock_cycles = CPU_CORES.at(core_no).prev_clock_cycles;
				last_mrm = CPU_CORES.at(core_no).prev_clock_cycles + MRM_delay;
			}
			else{
				int start = std::max(last_mrm + 1, CPU_CORES.at(core_no).prev_clock_cycles - 2);
				CPU_CORES.at(core_no).clock_cycles = start + 2 + DRAM.COL_ACCESS_DELAY;
				CPU_CORES.at(core_no).onhold_output =  "cycle " + std::to_string(start) + "-" + std::to_string(start + 2) + ": " + "MRM to DRAM\n" +
				/*"Core: " + std::to_string(core_no) +*/ "cycle " + std::to_string(start + 3) + "-" + std::to_string(CPU_CORES.at(core_no).clock_cycles) + ": " + reg1 + "="  + std::to_string(CPU_CORES.at(core_no).reg_values[r1])+ ", Row buffer not updated"+ ", Accessed address " + std::to_string(x) + " from row buffer\n\n";
				CPU_CORES.at(core_no).max_cycle = CPU_CORES.at(core_no).clock_cycles;
				CPU_CORES.at(core_no).clock_cycles = CPU_CORES.at(core_no).prev_clock_cycles;
				last_mrm = start + 2;
			}
			for(int p = 0; p < 32; p++){
				CPU_CORES.at(core_no).prev_reg_values[p] = CPU_CORES.at(core_no).reg_values[p];
			}
			return 0;
		}
		else{
			for(int i = 0; i < 1024; i++){
				memory[cur_row*1024 + i] = row_buffer[i];
				row_buffer[i] = memory[row*1024 + i];
			}
			CPU_CORES.at(core_no).reg_values[r1] = row_buffer[col];
			cur_row = row;
			if(CPU_CORES.at(core_no).prev_clock_cycles == in_cycle){
				CPU_CORES.at(core_no).clock_cycles = CPU_CORES.at(core_no).clock_cycles + DRAM.COL_ACCESS_DELAY + 2*DRAM.ROW_ACCESS_DELAY + MRM_delay;
				CPU_CORES.at(core_no).onhold_output =  "cycle " + std::to_string(CPU_CORES.at(core_no).prev_clock_cycles + 1) + "-" + std::to_string(CPU_CORES.at(core_no).prev_clock_cycles + MRM_delay) + ": " + "MRM to DRAM\n" +
				 /*"Core: " + std::to_string(core_no) + */"cycle " + std::to_string(CPU_CORES.at(core_no).prev_clock_cycles + 1 + MRM_delay) + "-" + std::to_string(CPU_CORES.at(core_no).clock_cycles) + ": " + reg1 + "="  + std::to_string(CPU_CORES.at(core_no).reg_values[r1])+ ", Row buffer not updated" + ", Copied a row and activated row " + std::to_string(row) + " and accessed address " + std::to_string(x) + "\n\n";
				CPU_CORES.at(core_no).max_cycle = CPU_CORES.at(core_no).clock_cycles;
				CPU_CORES.at(core_no).clock_cycles = CPU_CORES.at(core_no).prev_clock_cycles;
				last_mrm = CPU_CORES.at(core_no).prev_clock_cycles + MRM_delay;
			}
			else{
				int start = std::max(last_mrm + 1, CPU_CORES.at(core_no).prev_clock_cycles - 2);
				CPU_CORES.at(core_no).clock_cycles = start + 2 + DRAM.COL_ACCESS_DELAY + 2*DRAM.ROW_ACCESS_DELAY;
				CPU_CORES.at(core_no).onhold_output =  "cycle " + std::to_string(start) + "-" + std::to_string(start + 2) + ": " + "MRM to DRAM\n" +
				 /*"Core: " + std::to_string(core_no) + */"cycle " + std::to_string(start + 3) + "-" + std::to_string(CPU_CORES.at(core_no).clock_cycles) + ": " + reg1 + "="  + std::to_string(CPU_CORES.at(core_no).reg_values[r1])+ ", Row buffer not updated" + ", Copied a row and activated row " + std::to_string(row) + " and accessed address " + std::to_string(x) + "\n\n";
				CPU_CORES.at(core_no).max_cycle = CPU_CORES.at(core_no).clock_cycles;
				CPU_CORES.at(core_no).clock_cycles = CPU_CORES.at(core_no).prev_clock_cycles;
				last_mrm = start + 2;
			}
			buffer_updates = buffer_updates + 1;
			for(int p = 0; p < 32; p++){
				CPU_CORES.at(core_no).prev_reg_values[p] = CPU_CORES.at(core_no).reg_values[p];
			}
			return 0;
		}
	}	
}

int Sw(int core_no, string reg1, string reg2, int reg1_val, int reg2_val, int offset, int line_no, int in_cycle){

	int r1 = inArray(reg1, registers, 32) - 1;
	int r2 = inArray(reg2, registers, 32) - 1;
	if(reg2_val + offset + CPU_CORES.at(core_no).start_address >= CPU_CORES.at(core_no).end_address){
		cout << "Memory address out of bounds, line no: " << line_no + 1 << " in file no: " << core_no << endl;
		exit(-1);
	}
	else if(reg2_val + offset + CPU_CORES.at(core_no).start_address < CPU_CORES.at(core_no).start_address){
		cout << "Invalid memory address, line no: " << line_no + 1 << " in file no: " << core_no << endl;
		exit(-1);
	}
	else if((reg2_val + offset + CPU_CORES.at(core_no).start_address)%4 != 0){

		cout << "Invalid memory address, line no:" << line_no + 1 << " in file no: " << core_no << endl;
		exit(-1);
	}
	
	else{
		cur_ins = "sw";
		int x = reg2_val + offset + CPU_CORES.at(core_no).start_address;
		int y = x + 3;
		CPU_CORES.at(core_no).on_hold = true;
		cur_core = core_no;
		int row = (reg2_val + offset + CPU_CORES.at(core_no).start_address)/1024;
		int col = (reg2_val + offset + CPU_CORES.at(core_no).start_address)%1024;
		cur_address = reg2_val + offset + CPU_CORES.at(core_no).start_address;
		if (cur_row == -1){
			for(int i = 0; i < 1024; i++){
				row_buffer[i] = memory[row*1024 + i];
			}
			row_buffer[col] = reg1_val;
			cur_row = row;
			if(CPU_CORES.at(core_no).prev_clock_cycles == in_cycle){
				CPU_CORES.at(core_no).clock_cycles = CPU_CORES.at(core_no).clock_cycles + DRAM.COL_ACCESS_DELAY + DRAM.ROW_ACCESS_DELAY + MRM_delay;
				CPU_CORES.at(core_no).onhold_output =  "cycle " + std::to_string(CPU_CORES.at(core_no).prev_clock_cycles + 1) + "-" + std::to_string(CPU_CORES.at(core_no).prev_clock_cycles + MRM_delay) + ": " + "MRM to DRAM\n" +
				"cycle " + std::to_string(CPU_CORES.at(core_no).prev_clock_cycles + 1 + MRM_delay) + "-" + std::to_string(CPU_CORES.at(core_no).clock_cycles) + ": " + "No register modified, Updated memory address " + std::to_string(x) + "-" + std::to_string(y) + " = " + std::to_string(row_buffer[col])+ ", Activated row " + std::to_string(row) + " and accessed address " + std::to_string(x) + "\n\n";
				CPU_CORES.at(core_no).max_cycle = CPU_CORES.at(core_no).clock_cycles;
				CPU_CORES.at(core_no).clock_cycles = CPU_CORES.at(core_no).prev_clock_cycles;
				last_mrm = CPU_CORES.at(core_no).prev_clock_cycles + MRM_delay;
			}
			else{
				int start = std::max(last_mrm + 1, CPU_CORES.at(core_no).prev_clock_cycles - 2);
				CPU_CORES.at(core_no).clock_cycles = start + 2 + DRAM.COL_ACCESS_DELAY + DRAM.ROW_ACCESS_DELAY;
				CPU_CORES.at(core_no).onhold_output =  "cycle " + std::to_string(start) + "-" + std::to_string(start + 2) + ": " + "MRM to DRAM\n" +
				"cycle " + std::to_string(start + 3) + "-" + std::to_string(CPU_CORES.at(core_no).clock_cycles) + ": " + "No register modified, Updated memory address " + std::to_string(x) + "-" + std::to_string(y) + " = " + std::to_string(row_buffer[col])+ ", Activated row " + std::to_string(row) + " and accessed address " + std::to_string(x) + "\n\n";
				CPU_CORES.at(core_no).max_cycle = CPU_CORES.at(core_no).clock_cycles;
				CPU_CORES.at(core_no).clock_cycles = CPU_CORES.at(core_no).prev_clock_cycles;
				last_mrm = start + 2;
			}
			//CPU_CORES.at(core_no).clock_cycles = CPU_CORES.at(core_no).clock_cycles + DRAM.COL_ACCESS_DELAY + DRAM.ROW_ACCESS_DELAY;
			//CPU_CORES.at(core_no).onhold_output = /*"Core: " + std::to_string(core_no) +*/ "cycle " + std::to_string(CPU_CORES.at(core_no).prev_clock_cycles + 1) + "-" + std::to_string(CPU_CORES.at(core_no).clock_cycles) + ": " + "No register modified, Updated memory address " + std::to_string(x) + "-" + std::to_string(y) + " = " + std::to_string(row_buffer[col])+ ", Activated row " + std::to_string(row) + " and accessed address " + std::to_string(x) + "\n\n";
			//CPU_CORES.at(core_no).max_cycle = CPU_CORES.at(core_no).clock_cycles;
			//CPU_CORES.at(core_no).clock_cycles = CPU_CORES.at(core_no).prev_clock_cycles;
			buffer_updates = buffer_updates + 2;
			return 0;
		}
		else if(row == cur_row){
			row_buffer[col] = reg1_val;
			if(CPU_CORES.at(core_no).prev_clock_cycles == in_cycle){
				CPU_CORES.at(core_no).clock_cycles = CPU_CORES.at(core_no).clock_cycles + DRAM.COL_ACCESS_DELAY + MRM_delay;
				CPU_CORES.at(core_no).onhold_output =  "cycle " + std::to_string(CPU_CORES.at(core_no).prev_clock_cycles + 1) + "-" + std::to_string(CPU_CORES.at(core_no).prev_clock_cycles + MRM_delay) + ": " + "MRM to DRAM\n" +
				/*"Core: " + std::to_string(core_no) + */"cycle " + std::to_string(CPU_CORES.at(core_no).prev_clock_cycles + 1 + MRM_delay) + "-" + std::to_string(CPU_CORES.at(core_no).clock_cycles) + ": " + "No register modified, Updated memory address " + std::to_string(x) + "-" + std::to_string(y) + " = " + std::to_string(row_buffer[col]) + ", Accessed address " + std::to_string(x) + " from the row buffer\n\n";;
				CPU_CORES.at(core_no).max_cycle = CPU_CORES.at(core_no).clock_cycles;
				CPU_CORES.at(core_no).clock_cycles = CPU_CORES.at(core_no).prev_clock_cycles;
				last_mrm = CPU_CORES.at(core_no).prev_clock_cycles + MRM_delay;
			}
			else{
				int start = std::max(last_mrm + 1, CPU_CORES.at(core_no).prev_clock_cycles - 2);
				CPU_CORES.at(core_no).clock_cycles = start + 2 + DRAM.COL_ACCESS_DELAY;
				CPU_CORES.at(core_no).onhold_output =  "cycle " + std::to_string(start) + "-" + std::to_string(start + 2) + ": " + "MRM to DRAM\n" +
				/*"Core: " + std::to_string(core_no) + */"cycle " + std::to_string(start + 3) + "-" + std::to_string(CPU_CORES.at(core_no).clock_cycles) + ": " + "No register modified, Updated memory address " + std::to_string(x) + "-" + std::to_string(y) + " = " + std::to_string(row_buffer[col]) + ", Accessed address " + std::to_string(x) + " from the row buffer\n\n";;
				CPU_CORES.at(core_no).max_cycle = CPU_CORES.at(core_no).clock_cycles;
				CPU_CORES.at(core_no).clock_cycles = CPU_CORES.at(core_no).prev_clock_cycles;
				last_mrm = start + 2;
			}
			//CPU_CORES.at(core_no).clock_cycles = CPU_CORES.at(core_no).clock_cycles + DRAM.COL_ACCESS_DELAY;
			//CPU_CORES.at(core_no).onhold_output = /*"Core: " + std::to_string(core_no) + */"cycle " + std::to_string(CPU_CORES.at(core_no).prev_clock_cycles + 1) + "-" + std::to_string(CPU_CORES.at(core_no).clock_cycles) + ": " + "No register modified, Updated memory address " + std::to_string(x) + "-" + std::to_string(y) + " = " + std::to_string(row_buffer[col]) + ", Accessed address " + std::to_string(x) + " from the row buffer\n\n";;
			//CPU_CORES.at(core_no).max_cycle = CPU_CORES.at(core_no).clock_cycles;
			//CPU_CORES.at(core_no).clock_cycles = CPU_CORES.at(core_no).prev_clock_cycles;
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
			if(CPU_CORES.at(core_no).prev_clock_cycles == in_cycle){
				CPU_CORES.at(core_no).clock_cycles = CPU_CORES.at(core_no).clock_cycles + DRAM.COL_ACCESS_DELAY + 2*DRAM.ROW_ACCESS_DELAY + MRM_delay;
				CPU_CORES.at(core_no).onhold_output =  "cycle " + std::to_string(CPU_CORES.at(core_no).prev_clock_cycles + 1) + "-" + std::to_string(CPU_CORES.at(core_no).prev_clock_cycles + MRM_delay) + ": " + "MRM to DRAM\n" +
				 /*"Core: " + std::to_string(core_no) + */"cycle " + std::to_string(CPU_CORES.at(core_no).prev_clock_cycles + 1 + MRM_delay) + "-" + std::to_string(CPU_CORES.at(core_no).clock_cycles) + ": " + "No register modified, Updated memory address " + std::to_string(x) + "-" + std::to_string(y) + " = " + std::to_string(row_buffer[col]) + ", Copied a row and activated row " + std::to_string(row) + " and accessed address " + std::to_string(x) + "\n\n";
				CPU_CORES.at(core_no).max_cycle = CPU_CORES.at(core_no).clock_cycles;
				CPU_CORES.at(core_no).clock_cycles = CPU_CORES.at(core_no).prev_clock_cycles;
				last_mrm = CPU_CORES.at(core_no).prev_clock_cycles + MRM_delay;
			}
			else{
				int start = std::max(last_mrm + 1, CPU_CORES.at(core_no).prev_clock_cycles - 2);
				CPU_CORES.at(core_no).clock_cycles = start + 2 + DRAM.COL_ACCESS_DELAY + 2*DRAM.ROW_ACCESS_DELAY;
				CPU_CORES.at(core_no).onhold_output =  "cycle " + std::to_string(start) + "-" + std::to_string(start + 2) + ": " + "MRM to DRAM\n" +
				 /*"Core: " + std::to_string(core_no) + */"cycle " + std::to_string(start + 3) + "-" + std::to_string(CPU_CORES.at(core_no).clock_cycles) + ": " + "No register modified, Updated memory address " + std::to_string(x) + "-" + std::to_string(y) + " = " + std::to_string(row_buffer[col]) + ", Copied a row and activated row " + std::to_string(row) + " and accessed address " + std::to_string(x) + "\n\n";
				CPU_CORES.at(core_no).max_cycle = CPU_CORES.at(core_no).clock_cycles;
				CPU_CORES.at(core_no).clock_cycles = CPU_CORES.at(core_no).prev_clock_cycles;
				last_mrm = start + 2;
			}
			//CPU_CORES.at(core_no).clock_cycles = CPU_CORES.at(core_no).clock_cycles + DRAM.COL_ACCESS_DELAY + 2*DRAM.ROW_ACCESS_DELAY;
			//CPU_CORES.at(core_no).onhold_output = /*"Core: " + std::to_string(core_no) + */"cycle " + std::to_string(CPU_CORES.at(core_no).prev_clock_cycles + 1) + "-" + std::to_string(CPU_CORES.at(core_no).clock_cycles) + ": " + "No register modified, Updated memory address " + std::to_string(x) + "-" + std::to_string(y) + " = " + std::to_string(row_buffer[col]) + ", Copied a row and activated row " + std::to_string(row) + " and accessed address " + std::to_string(x) + "\n\n";
			//CPU_CORES.at(core_no).max_cycle = CPU_CORES.at(core_no).clock_cycles;
			//CPU_CORES.at(core_no).clock_cycles = CPU_CORES.at(core_no).prev_clock_cycles;
			buffer_updates = buffer_updates + 2;
			return 0;
		}
	}
}

int Addi(string reg1, string reg2, int val, int line_no, int core_no){
	int r1 = inArray(reg1, registers, 32) - 1;
	int r2 = inArray(reg2, registers, 32) - 1;
	isZero(r1, line_no) ;
	CPU_CORES.at(core_no).reg_values[r1] = CPU_CORES.at(core_no).reg_values[r2] + val;
	return 0;
}

bool is_independent(string reg[], int len, int id, int core_no){
	if(len == 0 || queue_length == 0){
		return true;
	}
	for(int j = 0; j < queue_length; j++){
		if(ins_queue[j].core_no != core_no){
			continue;
		}
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

void execute_queue(int core_no, int id){
	//print_queue();
	if(cur_core != core_no){
		if(running_cycle < CPU_CORES.at(cur_core).max_cycle){
			//cout << "\e[1m" << "\e[34m"<< "Core: " << core_no + 1 << "\e[0m" << "\e[0m"<< endl;
			cout << "Waiting for DRAM & MRM completion\n" << endl;
			return;
		}
		cout << CPU_CORES.at(cur_core).onhold_output;
		CPU_CORES.at(cur_core).ins_count++;
		CPU_CORES.at(cur_core).on_hold = false;
		CPU_CORES.at(cur_core).clock_cycles = CPU_CORES.at(cur_core).max_cycle;
		CPU_CORES.at(cur_core).prev_clock_cycles = CPU_CORES.at(cur_core).clock_cycles;
		CPU_CORES.at(core_no).clock_cycles = CPU_CORES.at(cur_core).max_cycle;
		CPU_CORES.at(core_no).prev_clock_cycles = CPU_CORES.at(core_no).clock_cycles;
	}
	else{
		if(running_cycle < CPU_CORES.at(cur_core).max_cycle){
			//cout << "\e[1m" << "\e[34m"<< "Core: " << core_no + 1 << "\e[0m" << "\e[0m"<< endl;
			cout << "Waiting for DRAM & MRM completion\n" << endl;
			return;
		}
		cout << CPU_CORES.at(core_no).onhold_output;
		CPU_CORES.at(cur_core).ins_count++;
		CPU_CORES.at(core_no).on_hold = false;
		CPU_CORES.at(core_no).clock_cycles = CPU_CORES.at(core_no).max_cycle;
		CPU_CORES.at(core_no).prev_clock_cycles = CPU_CORES.at(core_no).clock_cycles;
	}

	if(queue_length > 1){
		for(int i = 1; i < queue_length; i++){
			ins_queue[i - 1] = ins_queue[i];
		}
		queue_length--;
		/*int flag = 0;
		while(flag == 0){
			modify();
			if(queue_length > 2){
				int change = 0;
				for(int i = 1; i < queue_length; i++){
					if(ins_queue[0].core_no == ins_queue[i].core_no){
						if(ins_queue[0].ins_id == ins_queue[i].ins_id && ins_queue[0].ins_id == 8){
							if(ins_queue[0].reg[0]  == ins_queue[i].reg[0]){
								for(int j = 1; j < queue_length; j++){
									ins_queue[j - 1] = ins_queue[j];
								}
								change = 1;
								queue_length--;
							}
						}
						else{
							if(ins_queue[0].ins_id == ins_queue[i].ins_id && ins_queue[0].reg2_val + ins_queue[0].val == ins_queue[i].reg2_val + ins_queue[i].val){
								for(int j = 1; j < queue_length; j++){
									ins_queue[j - 1] = ins_queue[j];
								}
								change = 1;
								queue_length--;
							}
						}
						
						break;
					}
				}
				if(change == 1){
					flag = 0;
				}
				else{
					flag = 1;
				}
			}
			else{
				break;
			}
		}*/
		if(id == 8){
			//CPU_CORES.at(core_no).clock_cycles++;
			//cout << "\e[1m" << "\e[34m"<< "Core: " << core_no + 1 << "\e[0m" << "\e[0m"<< endl;
			cout << /*"cycle " << CPU_CORES.at(core_no).clock_cycles << */"Memory address of the instruction: " << ins_queue[queue_length].line_no*4 << endl;
			cout << "Executed Instruction: " << "lw " << ins_queue[queue_length].reg[0] << ", " << ins_queue[queue_length].val << "(" << ins_queue[queue_length].reg[1] << ")" << endl;
			cout << "Added to MRM wait buffer" << endl;
			cout << endl;
			//CPU_CORES.at(core_no).prev_clock_cycles = CPU_CORES.at(core_no).clock_cycles;
		}
		else if(id == 9){
			//cout << "\e[1m" << "\e[34m"<< "Core: " << core_no + 1 << "\e[0m" << "\e[0m"<< endl;
			//CPU_CORES.at(core_no).clock_cycles++;
			cout << /*"cycle " << CPU_CORES.at(core_no).clock_cycles << */"Memory address of the instruction: " << ins_queue[queue_length].line_no*4 << endl;
			cout << "Executed Instruction: " << "sw " << ins_queue[queue_length].reg[0] << ", " << ins_queue[queue_length].val << "(" << ins_queue[queue_length].reg[1] << ")" << endl;
			cout << "Added to MRM wait buffer" << endl;
			cout << endl;
			//CPU_CORES.at(core_no).prev_clock_cycles = CPU_CORES.at(core_no).clock_cycles;
		}
		//if(flag == 1){
			modify();
		//}
		
		check_2(ins_queue[0], ins_queue[0].line_no);
		int cur_core_2 = ins_queue[0].core_no;
		if(cur_core_2 != cur_core){
			CPU_CORES.at(cur_core_2).clock_cycles = CPU_CORES.at(cur_core).max_cycle;
			CPU_CORES.at(cur_core_2).prev_clock_cycles = CPU_CORES.at(cur_core).max_cycle;
		}
		CPU_CORES.at(cur_core).max_cycle = -1;
		cur_core = cur_core_2;
		//cout << ins_queue[0].line_no << endl;	
		core CPU = CPU_CORES.at(cur_core);
		if(ins_queue[0].ins_id == 8){
			Lw(cur_core, ins_queue[0].reg[0], ins_queue[0].reg[1], ins_queue[0].reg2_val, ins_queue[0].val, ins_queue[0].line_no, ins_queue[0].in_cycle);
		}
		else{
			Sw(cur_core, ins_queue[0].reg[0], ins_queue[0].reg[1], ins_queue[0].reg1_val, ins_queue[0].reg2_val, ins_queue[0].val, ins_queue[0].line_no, ins_queue[0].in_cycle);
		}
	}
	else{
		CPU_CORES.at(cur_core).max_cycle = -1;
		queue_length--;
		cur_ins = "None";
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
	int row = (ins.reg2_val + ins.val + CPU_CORES.at(ins.core_no).start_address)/1024;
	return row;
}

void reorder( int x){
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
	if(ins_queue[x].core_no != ins_queue[y].core_no){
		return true;
	}
	core CPU = CPU_CORES.at(ins_queue[x].core_no);
	int r1 = inArray(ins_queue[x].reg[1], registers, 32) - 1;
	int r2 = inArray(ins_queue[y].reg[1], registers, 32) - 1;
	int offset_1 = ins_queue[x].val;
	int offset_2 = ins_queue[y].val;
	int memory_1 = CPU.reg_values[r1] + offset_1 + CPU.start_address;
	int memory_2 = CPU.reg_values[r2] + offset_2 + CPU.start_address;
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
void print_register(int core_no, int temp){
	core CPU = CPU_CORES.at(core_no);
	cout << "Executed Instruction: " << getLine(CPU.all_ins, temp) << endl;
	int mod = 0;
	for(int i = 0; i < 32; i++){
		if(CPU.prev_reg_values[i] != CPU.reg_values[i]){
			cout << registers[i] << " = " << CPU.reg_values[i];
			mod++;
		}
	}
	/*if(mod == 0){
		cout << "No register modified, Memory not modified";
	}
	else{
		cout << ", Memory not modified";
	}*/
	cout << "\n" << endl;
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
		cout << "Missing Register(s) in the instruction in line: " << line_no + 1 << " in file no:" << ins.core_no << endl;
		exit(-1);
	}
	if(ins.num_val != "NULL"){
		cout << "Unexpected Integer(s) in the instruction in line: " << line_no + 1 << " in file no:" << ins.core_no << endl;
		exit(-1);
	}
	return true;
}

bool check_2(instruct ins, int line_no){
	if(ins.reg[0] == "None" || ins.reg[1] == "None" || ins.reg[2] != "None"){
		cout << "Invalid syntax in line:" << line_no + 1 << " in file no:" << ins.core_no <<  endl;
		exit(-1);
	}
	if(ins.num_val == "NULL"){
		cout << "Invalid syntax in line:" << line_no + 1 << " in file no:" << ins.core_no << endl;
		exit(-1);
	}
	return true;
}

bool check_3(instruct ins, int line_no){
	if (ins.reg[0] != "None" || ins.reg[1] != "None" || ins.reg[2] != "None"){
		cout << "Invalid syntax in line:" << line_no + 1 << " in file no:" << ins.core_no << endl; 
		exit(-1);
	}
	if(ins.num_val == "NULL"){
		cout << "Invalid syntax in line:" << line_no + 1 << " in file no:" << ins.core_no << endl;
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

bool check_4(instruct ins, int line_no){
	if(ins.reg[0] == "None" || ins.reg[1] == "None" || ins.reg[2] != "None"){
		cout << "Invalid syntax in line:" << line_no + 1 << " in file no:" << ins.core_no << endl;
		exit(-1);
	}
	if(ins.num_val != "NULL"){
		cout << "Invalid syntax in line:" << line_no + 1 << " in file no:" << ins.core_no << endl;
		exit(-1);
	}
	if(ins.label == "None"){
		cout << "Invalid syntax in line:" << line_no + 1 << " in file no:" << ins.core_no << endl;
		exit(-1);
	}
	return true;
}

bool check_5(instruct ins, int line_no){
	if(ins.reg[0] != "None" || ins.reg[1] != "None" || ins.reg[2] != "None"){
		cout << "Invalid syntax in line:" << line_no + 1 << " in file no:" << ins.core_no << endl;
		exit(-1);
	}
	if(ins.num_val != "NULL"){
		cout << "Invalid syntax in line:" << line_no + 1 << " in file no:" << ins.core_no << endl;
		exit(-1);
	}
	if(ins.label == "None"){
		cout << "Invalid syntax in line:" << line_no + 1 << " in file no:" << ins.core_no << endl;
		exit(-1);
	}
	return true;
}


