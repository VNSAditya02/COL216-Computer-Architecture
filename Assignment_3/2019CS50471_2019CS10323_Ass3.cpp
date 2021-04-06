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

using namespace std;


// stores possible operations
string operations[10] = {"add", "sub", "mul", "beq", "bne", "slt", "j", "lw", "sw", "addi"};
// stores the register names
string registers[32] = {"$zero", "$at", "$v0", "$v1", "$a0", "$a1", "$a2", "$a3", 
						"$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7", 
						"$s0", "$s1", "$s2", "$s3", "$s4", "$s5", "$s6", "$s7",
						"$t8", "$t9", "$k0", "$k1", "$gp", "$sp", "$fp", "$ra"};
string registers_final[32] = {"zero", "at", "v0", "v1", "a0", "a1", "a2", "a3", 
						"t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7", 
						"s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7",
						"t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra"};

// Stores the value in each register
int reg_values[32];

// Stores the start address of data memory
int data_address = 0;

int clock_cycles = 0;

int memory[(int)pow(2.0, 20.0)];

struct instruct{

	// ins_id of add: 1; sub: 2; mul: 3; beq: 4; bne: 5; slt: 6; j:7; lw: 8; sw: 9; addi: 10
	int ins_id = 0;

	// reg1, reg2, reg3 stores the registers used in the instruction
	// If the register has less than 3 registers, then the remaining registers store None
	string reg[3] = {"None", "None", "None"};

	// stores the value (either integer in case of addi and offset in case of lw/sw)
	int val[2] = {0, 0};

	string num_val[2] = {"NULL", "NULL"};

	// lineNo stores the aline number to which we go if the instruction is j/bne/beq
	int lineNo = 0;

};
int inArray(string word, string arr[], int size);
int execute(vector<instruct> PC, int line_no, int data_memory, int ins_count[], bool is_ins[]);
int compute(instruct ins, int line_no, int ins_count[], bool is_ins[]);
int Add(string reg1, string reg2, string reg3, int line_no);
int Sub(string reg1, string reg2, string reg3, int line_no);
int Mul(string reg1, string reg2, string reg3, int line_no);
int Beq(string reg1, string reg2, int val);
int Bne(string reg1, string reg2, int val);
int Slt(string reg1, string reg2, string reg3, int line_no);
int Lw(string reg1, string reg2, int offset, int line_no);
int Sw(string reg1, string reg2, int offset, int line_no);
int Addi(string reg1, string reg2, int val, int line_no);
void print_register();
instruct getInstruct(vector<instruct> PC, int line_no);
bool check_1(instruct ins, int line_no);
bool check_2(instruct ins, int line_no);
bool check_3(instruct ins, int line_no);
void initialize();
int isZero(int r);
void instructions_count(int ins_count[], bool is_ins[], int line_num);

int main(int argc, char* argv[]){

	reg_values[0] = 0;

	vector<instruct> PC;

	// Reads file location from command line
	// And opens the file to read
	fstream input_file;
	input_file.open(argv[1], ios::in);

	// Read the input file line by line
	string str;
	string word;
	int line_num = 0;

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
			
			// If the instructions is not permitted, exit from the program
			if(i == 0){

				ins_id = inArray(word, operations, 10);

				if (ins_id == 0){
					cout << "Invalid instruction: " << word << " in line "<< line_num + 1 << endl;
					return -1;
				}

				else{
					line.ins_id = ins_id;
				}
			}

			else if(i >= 1 && i < 4){
				// Check if the string is a register
				if (inArray(word, registers, 32) != 0){
					line.reg[i - 1] = word;
					i++;
					continue;
				}

				if(i >= 1){

					// Check if the number is an integer
					string temp = word;
					char* end;
					char* c = const_cast<char*>(word.c_str());
					int val = strtol(c, &end, 10);

					// If it is not an integer, exit from loop
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
							if(val_start > 1){
								cout << "Invalid instruction: " << temp << " in line "<< line_num + 1 << endl;
								return -1;
							}
							line.val[val_start] = val;
							line.num_val[val_start] = "yes";
							val_start++;

						}
						else{
    						cout << "Invalid instruction: " << temp << " in line "<< line_num + 1 << endl;
    						return -1;
    					}
					}

					// If it is an integer, add it to the val
					else {
						if(val_start > 1){
							cout << "Invalid instruction: " << temp << " in line "<< line_num + 1 << endl;
							return -1;
						}
	    				line.val[val_start] = val;
	    				line.num_val[val_start] = "yes";
	    				val_start++;
					}

				}

				// If both of the above cases are not valid, then exit from loop
				else{
					cout << "Invalid instruction: " << word << " in line "<< line_num + 1 << endl;
					return -1;
				}

			}

			// If length is greater than 4, then exit from loop
			else if(i >= 4){
				cout << "Invalid instruction: " << word <<" in line "<< line_num << endl;
				return -1;
			}			
			i++;
			
		}

		line.lineNo = line_num;
		data_address = data_address + 4;
		PC.push_back(line);
		line_num++;
		
	}
	initialize();
	int ins_count[line_num + 1];
	bool is_ins[line_num + 1];
	for(int k = 0; k < line_num + 1; k++){
		is_ins[k] = true;
		ins_count[k] = 0;
	}
	int print_reg = execute(PC, 0, data_address, ins_count, is_ins);
	if(print_reg == -1){
		return -1;
	}
	cout << "Clock Cycles: " << clock_cycles << endl;
	instructions_count(ins_count, is_ins, line_num + 1);
	return 0;

}
void instructions_count(int ins_count[], bool is_ins[], int line_num){
	for(int i = 0; i < line_num - 1; i++){
		if (is_ins[i] == true){
			cout << "Number of times instructions in line " << i + 1 << ", executed is " << ins_count[i] << endl;
		}
	}
}

int inArray(string word, string arr[], int size){

	// check if the given word is one of the instructions
	for (int i = 0; i < size; i++){
		int x = word.compare(arr[i]);
		if (x == 0){
			return i + 1;
		}
	}

	return 0;
}

int execute(vector<instruct> PC, int line_no, int data_address, int ins_count[], bool is_ins[]){
	while(line_no < (data_address/4)){
		line_no = compute(getInstruct(PC, line_no), line_no, ins_count, is_ins);
		if (line_no == -1){
			return -1;
		}
		print_register();
	}	
	return 0;
}

int compute(instruct ins, int line_no, int ins_count[], bool is_ins[]){
	if(ins.ins_id == 1){
		clock_cycles++;
		bool go = check_1(ins, line_no);
		if(go == true){
			int x = Add(ins.reg[0], ins.reg[1], ins.reg[2], line_no);
			if (x == -1){
				return -1;
			}
			ins_count[line_no]++;
			return line_no + 1;
		}
		else{
			return -1;
		}
	}

	else if(ins.ins_id == 2){
		clock_cycles++;
		bool go = check_1(ins, line_no);
		if(go == true){
			int x = Sub(ins.reg[0], ins.reg[1], ins.reg[2], line_no);
			if (x == -1){
				return -1;
			}
			ins_count[line_no]++;
			return line_no + 1;
		}
		else{
			return -1;
		}
	}

	else if(ins.ins_id == 3){
		clock_cycles++;
		bool go = check_1(ins, line_no);
		if(go == true){
			int x = Mul(ins.reg[0], ins.reg[1], ins.reg[2], line_no);
			if (x == -1){
				return -1;
			}
			ins_count[line_no]++;
			return line_no + 1;
		}
		else{
			return -1;
		}
	}

	else if(ins.ins_id == 4){
		clock_cycles++;
		bool go = check_2(ins, line_no);
		if(go == true){
			int x = Beq(ins.reg[0], ins.reg[1], ins.val[0]);
			if(x == 0){
				ins_count[line_no]++;
				return 1 + line_no;
			}
			else{
				if(ins.val[0] - 1 < 1){
					cout << "Can't go to the mentioned line from the branch, line " << line_no + 1<< endl;
					return -1;
				}
				ins_count[line_no]++;
				return ins.val[0] - 1;
			}
		}
		else{
			return -1;
		}
	}

	else if(ins.ins_id == 5){
		clock_cycles++;
		bool go = check_2(ins, line_no);
		if(go == true){
			int x = Bne(ins.reg[0], ins.reg[1], ins.val[0]);
			if(x == 0){
				ins_count[line_no]++;
				return 1 + line_no;
			}
			else{
				if(ins.val[0] - 1 < 1){
					cout << "Can't go to the mentioned line from the branch, line " << line_no + 1<< endl;
					return -1;
				}
				ins_count[line_no]++;
				return ins.val[0] - 1;
			}
		}
		else{
			return -1;
		}
	}

	else if(ins.ins_id == 6){
		clock_cycles++;
		bool go = check_1(ins, line_no);
		if(go == true){
			int x = Slt(ins.reg[0], ins.reg[1], ins.reg[2], line_no);
			if (x == -1){
				return -1;
			}
			ins_count[line_no]++;
			return line_no + 1;
		}
		else{
			return -1;
		}
	}

	else if(ins.ins_id == 7){
		clock_cycles++;
		bool go = check_3(ins, line_no);
		if(go == true){
			if(ins.val[0] - 1 < 1){
				cout << "Can't go to the mentioned line from jump, line " << line_no + 1 << endl;
				return -1;
			}
			ins_count[line_no]++;
			return ins.val[0] - 1;
		}
		else{
			return -1;
		}
	}

	else if(ins.ins_id == 8){
		clock_cycles++;
		bool go = check_2(ins, line_no);
		if(go == true){
			int valid_addr = Lw(ins.reg[0], ins.reg[1], ins.val[0], line_no);
			if(valid_addr == -1){
				return -1;
			}
			ins_count[line_no]++;
			return line_no + 1;
		}
		else{
			return -1;
		}
	}

	else if(ins.ins_id == 9){
		clock_cycles++;
		bool go = check_2(ins, line_no);
		if(go == true){
			int valid_addr = Sw(ins.reg[0], ins.reg[1], ins.val[0], line_no);
			if(valid_addr == -1){
				return -1;
			}
			ins_count[line_no]++;
			return line_no + 1;
		}
		else{
			return -1;
		}
	}

	else if(ins.ins_id == 10){
		clock_cycles++;
		bool go = check_2(ins, line_no);
		if(go == true){
			int x = Addi(ins.reg[0], ins.reg[1], ins.val[0], line_no);
			if (x == -1){
				return -1;
			}
			ins_count[line_no]++;
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

int Lw(string reg1, string reg2, int offset, int line_no){
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
		reg_values[r1] = memory[reg_values[r2] + offset];
		return 0;
	}	
}

int Sw(string reg1, string reg2, int offset, int line_no){
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
		memory[reg_values[r2] + offset] = reg_values[r1];
		return 0;
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

void print_register(){
	cout << "[";
	for(int i = 0; i < 31; i++){
		cout << registers_final[i] << " : " << reg_values[i] << ", ";
	}
	cout << registers_final[31] << " : " << reg_values[31];
	cout << "]";
	cout << "\n";
	cout << endl;
}

bool check_1(instruct ins, int line_no){
	if (ins.reg[0] == "None" || ins.reg[1] == "None" || ins.reg[2] == "None"){
		cout << "Missing Register(s) in the instruction in line: " << line_no + 1 << endl;
		return false;
	}
	if(ins.num_val[0] != "NULL" || ins.num_val[1] != "NULL"){
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
	if(ins.num_val[0] == "NULL" || ins.num_val[1] != "NULL"){
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
	if(ins.num_val[0] == "NULL" || ins.num_val[1] != "NULL"){
		cout << "Invalid syntax in line:" << line_no + 1 << endl;
		return false;
	}
	return true;
}

void initialize(){
	for(int i = data_address; i < pow(2.0,20.0); i++){
		memory[i] = 0;
	}
}

int isZero(int r){
	if(r == 0){
		return -1;
	}
	return 0;
}