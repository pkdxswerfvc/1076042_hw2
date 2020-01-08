#include <iostream>
#include <string>
#include <sstream>
#include <fstream>

#pragma warning (disable:4996)
using namespace std;

#define CODE 10			// the default amount of input_code
#define RF_number 9		// the default amount of RF
#define rat_number 9	// the default amount of register allocation table

// rs amount
#define RSandADD 3
#define RSandMUL 2
#define RS_CHECKER 2999

// function
void print_result();
void scheduling();
char print_operator(int inst_type);
char filename[20];
ofstream output_file;

// 指令種類
char inst_add[] = "ADD";
char inst_sub[] = "SUB";
char inst_multiple[] = "MUL";
char inst_divide[] = "DIV";
char inst_addi[] = "ADDI";

// 宣告變數
int line_id = 1;
int current_cycle = 1;
int inst_now = 0;
int register_used = 0;
int rs_space_add = RSandADD;
int rs_space_mul = RSandMUL;

// 指令的 cycle
int add_cycle_amount = 2;
int sub_cycle_amount = 2;
int mul_cycle_amount = 10;
int div_cycle_amount = 40;
int addi_cycle_amount = 2;

struct input_code {
	int inst_type = 0;    // add, sub, mul, div, addi
	char out_reg[5];
	char in_reg_1[5];
	char in_reg_2[5];
	int out_reg_id = 0;
	int in_reg_1_id = 0;
	int in_reg_2_id = 0;
	int issue_cycle = 0;
	int dispatch = 0;
}input_code[CODE]; 

struct rs_vec {
	int state = 0;    // 0為空 1為已被使用
	int inst_id = 0;
	int inst_type = 0;    // add, sub, mul, div, addi
	int source_a = 0;
	int source_b = 0;
	int write_cycle = 0;
	int in_rat_id = 0;
	int ready = 0;  // 1 = ok
}RS[RSandADD + RSandMUL + 1];    // RS1 ~ R3 = add, sub ; RS4 ~ RS5 = mul, div

struct rs_add_sub_buffer {
	int state = 0;    // 0為空 1為已被使用
	int rs_id = 0;
	int inst_type = 0;    // add, sub, mul, div, addi
	int source_a = 0;
	int source_b = 0;
}add_buffer;

struct rs_mul_div_buffer {
	int state = 0;    // 0為空 1為已被使用
	int rs_id = 0;    // ex. RS4, RS5 ...
	int inst_type = 0;    // add, sub, mul, div, addi
	int source_a = 0;
	int source_b = 0;
}mul_buffer;

// register allocation table
int RAT_F[rat_number + 1] = { 0 };
// register file
int RF_F[RF_number + 1] = { 0 };

// main function start
int main(int argc, char *argv[]) {
	cout << "  1076042 HW2 Instruction scheduling" << endl;
	cout << "Enter input file name: ";
	cin >> filename;

	ifstream infile(filename);
	if (!infile) {
		cout << "找不到檔案..." << endl;
		system("pause");
		exit(0);
	}
	// 初始化
	for (int i = 1, j = 0; i < RF_number + 1; i++) {
		RF_F[i] = j;
		j += 2;
	}

	char i1[10], i2[5], i3[5], i4[5];
	string raed_line;

	while (getline(infile, raed_line)) {
		istringstream iss(raed_line);
		iss >> i1 >> i2 >> i3 >> i4;
		cout << i1 << "\t" << i2 << "\t" << i3 << "\t" << i4 << endl;

		if (!strcmp(i1, inst_add)) {
			input_code[line_id].inst_type = 1;
		}
		else if (!strcmp(i1, inst_sub)) {
			input_code[line_id].inst_type = 2;
		}
		else if (!strcmp(i1, inst_multiple)) {
			input_code[line_id].inst_type = 3;
		}
		else if (!strcmp(i1, inst_divide)) {
			input_code[line_id].inst_type = 4;
		}
		else if (!strcmp(i1, inst_addi)) {
			input_code[line_id].inst_type = 5;
		}
		else {
			cout << "指令錯誤..." << endl;
			system("pause");
			exit(0);
		}
		strcpy(input_code[line_id].out_reg, strtok(i2, ","));
		strcpy(input_code[line_id].in_reg_1, strtok(i3, ","));
		strcpy(input_code[line_id].in_reg_2, i4);
		char tmp[5];
		strncpy(tmp, input_code[line_id].out_reg + 1, sizeof(input_code[line_id].out_reg));
		input_code[line_id].out_reg_id = atoi(tmp);
		memset(tmp, 0, sizeof(tmp));
		if (input_code[line_id].out_reg_id > register_used)
			register_used = input_code[line_id].out_reg_id;
		strncpy(tmp, input_code[line_id].in_reg_1 + 1, sizeof(input_code[line_id].in_reg_1));
		input_code[line_id].in_reg_1_id = atoi(tmp);
		memset(tmp, 0, sizeof(tmp));
		if (input_code[line_id].in_reg_1_id > register_used)
			register_used = input_code[line_id].in_reg_1_id;
		if (input_code[line_id].inst_type == 5) {
			strncpy(tmp, input_code[line_id].in_reg_2, sizeof(input_code[line_id].in_reg_2));
			input_code[line_id].in_reg_2_id = atoi(tmp);
		}
		else {
			strncpy(tmp, input_code[line_id].in_reg_2 + 1, sizeof(input_code[line_id].in_reg_2));
			input_code[line_id].in_reg_2_id = atoi(tmp);
			if (input_code[line_id].in_reg_2_id > register_used)
				register_used = input_code[line_id].in_reg_2_id;
		}
		memset(tmp, 0, sizeof(tmp));
		line_id++;
	}
	infile.close();
	cout << endl;
	line_id -= 1;
	cout << "\n輸入 cycle: ";
	cin >> current_cycle;
	cout << endl;
	if (current_cycle == 0) {
		print_result();
		system("pause");
		exit(0);
	}
	else {
		scheduling();
	}
	print_result();	// 印出結果
	system("pause");
	return 0;
}

void print_result() {
	char filename_output[40] = "output";
	char *str_temp = (char*)malloc(5 * sizeof(char));
	sprintf(str_temp, "%d", current_cycle);
	strcat(filename_output, "_");
	strcat(filename_output, str_temp);
	strcat(filename_output, ".txt");
	output_file.open(filename_output);
	cout << "目前 Cycle: " << current_cycle << endl << endl;
	output_file << "Cycle: " << current_cycle << endl << endl;
	cout << "RF: " << endl;
	output_file << "RF: " << endl;
	for (int i = 1; i < register_used + 1; i++) {
		cout << "F" << i << ": " << RF_F[i] << endl;
		output_file << "F" << i << ": " << RF_F[i] << endl;
	}
	cout << endl;
	output_file << endl;
	cout << "RAT: " << endl;
	output_file << "RAT: " << endl;
	for (int i = 1; i < register_used + 1; i++) {
		if (RAT_F[i] != 0) {
			cout << "F" << i << ": " << "RS" << RAT_F[i] << endl;
			output_file << "F" << i << ": " << "RS" << RAT_F[i] << endl;
		}
		else {
			cout << "F" << i << ": " << endl;
			output_file << "F" << i << ": " << endl;
		}
	}
	cout << endl;
	output_file << endl;
	cout << "RS: ";
	output_file << "RS: ";
	for (int i = 1; i < RSandADD + 1; i++) {
		if (RS[i].inst_type != 0) {
			cout << "\nRS" << i << ": " << print_operator(RS[i].inst_type) << " ";
			output_file << "\nRS" << i << ": " << print_operator(RS[i].inst_type) << " ";
			if (RS[i].source_a % RS_CHECKER == 0 && RS[i].source_a != 0) {
				cout << "\nRS" << RS[i].source_a / RS_CHECKER;
				output_file << "\nRS" << RS[i].source_a / RS_CHECKER;
			}
			else {
				cout << RS[i].source_a;
				output_file << RS[i].source_a;
			}
			if (RS[i].source_b % RS_CHECKER == 0 && RS[i].source_b != 0) {
				cout << "\nRS" << RS[i].source_b / RS_CHECKER;
				output_file << "\nRS" << RS[i].source_b / RS_CHECKER;
			}
			else {
				cout << " " << RS[i].source_b;
				output_file << " " << RS[i].source_b;
			}
			cout << " " << endl;
			output_file << " " << endl;
		}
		else {
			cout << "\nRS" << i << " ";
			output_file << "\nRS" << i << " ";
			output_file << endl;
		}
	}
	if (add_buffer.state == 1) {
		cout << "\nBuffer: (RS" << add_buffer.rs_id << ") " << add_buffer.source_a << " " << print_operator(RS[add_buffer.rs_id].inst_type) << " " << add_buffer.source_b << endl << endl;
		output_file << "\nBuffer: (RS" << add_buffer.rs_id << ") " << add_buffer.source_a << " " << print_operator(RS[add_buffer.rs_id].inst_type) << " " << add_buffer.source_b << endl << endl;
	}
	else {
		cout << "\nBuffer: X" << endl;
		output_file << "\nBuffer: X" << endl;
	}
	for (int i = RSandADD + 1; i < RSandADD + RSandMUL + 1; i++) {
		if (RS[i].inst_type != 0) {
			cout << "\nRS" << i << ": " << print_operator(RS[i].inst_type) << " ";
			output_file << "\nRS" << i << ": " << print_operator(RS[i].inst_type) << " ";
			if (RS[i].source_a % RS_CHECKER == 0 && RS[i].source_a != 0) {
				cout << "\nRS" << RS[i].source_a / RS_CHECKER;
				output_file << "\nRS" << RS[i].source_a / RS_CHECKER;
			}
			else {
				cout << RS[i].source_a;
				output_file << RS[i].source_a;
			}
			if (RS[i].source_b % RS_CHECKER == 0 && RS[i].source_b != 0) {
				cout << "\nRS" << RS[i].source_b / RS_CHECKER;
				output_file << "\nRS" << RS[i].source_b / RS_CHECKER;
			}
			else {
				cout << " " << RS[i].source_b;
				output_file << " " << RS[i].source_b;
			}
			cout << " " << endl;
			output_file << " " << endl;
		}
		else {
			cout << "RS" << i << ": ";
			output_file << "RS" << i << ": ";
			cout << endl;
			output_file << endl;
		}
	}
	if (mul_buffer.state == 1) {
		cout << "\nBuffer: (RS" << mul_buffer.rs_id << ") " << mul_buffer.source_a << " " << print_operator(RS[mul_buffer.rs_id].inst_type) << " " << mul_buffer.source_b << endl << endl;
		output_file << "\nBuffer: (RS" << mul_buffer.rs_id << ") " << mul_buffer.source_a << " " << print_operator(RS[mul_buffer.rs_id].inst_type) << " " << mul_buffer.source_b << endl << endl;
	}
	else {
		cout << "\nBuffer: X" << endl;
		output_file << "\nBuffer: X" << endl;
	}
	cout << endl;
}



int execute(int source_a, int source_b, int inst_type) {
	int result = 0;
	if (inst_type == 1 || inst_type == 5)
		result = source_a + source_b;
	else if (inst_type == 2)
		result = source_a - source_b;
	else if (inst_type == 3)
		result = source_a * source_b;
	else if (inst_type == 4)
		result = source_a / source_b;
	else
		result = 0;
	return result;
}

void scheduling() {
	for (int i = 0; i < current_cycle; i++) {
		// 確認 buffer
		if (add_buffer.state == 1) {
			// 確認 WR 與 cycle
			if (i == RS[add_buffer.rs_id].write_cycle) {
				if (input_code[RS[add_buffer.rs_id].inst_id + 1].out_reg_id == RS[add_buffer.rs_id].in_rat_id
					&& (input_code[RS[add_buffer.rs_id].inst_id + 1].in_reg_1_id != RS[add_buffer.rs_id].in_rat_id
						&& input_code[RS[add_buffer.rs_id].inst_id + 1].in_reg_2_id != RS[add_buffer.rs_id].in_rat_id)) {
					// 丟棄
				}
				else {
					if (add_buffer.rs_id == RAT_F[RS[add_buffer.rs_id].in_rat_id]) {
						// 更新
						RAT_F[RS[add_buffer.rs_id].in_rat_id] = 0;
					}
					// 更新 RF	
					RF_F[RS[add_buffer.rs_id].in_rat_id] = execute(add_buffer.source_a, add_buffer.source_b, add_buffer.inst_type);
				}
				// 更新 AT in RS table
				for (int j = 1; j < RSandADD + RSandMUL + 1; j++) {
					if (RS[j].source_a % RS_CHECKER == 0 && RS[j].source_a / RS_CHECKER == add_buffer.rs_id)
						RS[j].source_a = RF_F[RS[add_buffer.rs_id].in_rat_id];
					if (RS[j].source_b % RS_CHECKER == 0 && RS[j].source_b / RS_CHECKER == add_buffer.rs_id)
						RS[j].source_b = RF_F[RS[add_buffer.rs_id].in_rat_id];
				}
				// 清空 RS table
				RS[add_buffer.rs_id].in_rat_id = 0;
				RS[add_buffer.rs_id].inst_type = 0;
				RS[add_buffer.rs_id].inst_id = 0;
				RS[add_buffer.rs_id].source_a = 0;
				RS[add_buffer.rs_id].source_b = 0;
				RS[add_buffer.rs_id].state = 0;
				RS[add_buffer.rs_id].write_cycle = 0;
				RS[add_buffer.rs_id].ready = 0;
				rs_space_add += 1;
				// 清空暫存器
				add_buffer.inst_type = 0;
				add_buffer.rs_id = 0;
				add_buffer.source_a = 0;
				add_buffer.source_b = 0;
				add_buffer.state = 0;
			}
		}
		if (add_buffer.state == 0) {
			for (int j = 1; j < RSandADD + 1; j++) {
				if (RS[j].state == 1 && RS[j].ready == 1) {
					add_buffer.state = 1;
					add_buffer.rs_id = j;
					add_buffer.inst_type = RS[j].inst_type;
					add_buffer.source_a = RS[j].source_a;
					add_buffer.source_b = RS[j].source_b;
					input_code[RS[j].inst_id].dispatch = i;
					if (RS[j].inst_type == 1) 
						RS[j].write_cycle = i + add_cycle_amount;
					else if (RS[j].inst_type == 2) 
						RS[j].write_cycle = i + sub_cycle_amount;
					else if (RS[j].inst_type == 5) 
						RS[j].write_cycle = i + addi_cycle_amount;
					break;
				}
			}
		}
		if (mul_buffer.state == 1) {
			if (i == RS[mul_buffer.rs_id].write_cycle) {
				if (input_code[RS[mul_buffer.rs_id].inst_id + 1].out_reg_id == RS[mul_buffer.rs_id].in_rat_id
					&& (input_code[RS[mul_buffer.rs_id].inst_id + 1].in_reg_1_id != RS[mul_buffer.rs_id].in_rat_id
						&& input_code[RS[mul_buffer.rs_id].inst_id + 1].in_reg_2_id != RS[mul_buffer.rs_id].in_rat_id)) {
				}
				else {
					if (mul_buffer.rs_id == RAT_F[RS[mul_buffer.rs_id].in_rat_id]) {
						// 更新 RAT
						RAT_F[RS[mul_buffer.rs_id].in_rat_id] = 0;
					}
					// 更新 RF
					RF_F[RS[mul_buffer.rs_id].in_rat_id] = execute(mul_buffer.source_a, mul_buffer.source_b, mul_buffer.inst_type);
				}
				// 更新 RS table
				for (int j = 1; j < RSandADD + RSandMUL + 1; j++) {
					if (RS[j].source_a % RS_CHECKER == 0 && RS[j].source_a / RS_CHECKER == mul_buffer.rs_id)
						RS[j].source_a = RF_F[RS[mul_buffer.rs_id].in_rat_id];
					if (RS[j].source_b % RS_CHECKER == 0 && RS[j].source_b / RS_CHECKER == mul_buffer.rs_id)
						RS[j].source_b = RF_F[RS[mul_buffer.rs_id].in_rat_id];
				}
				RS[mul_buffer.rs_id].in_rat_id = 0;
				RS[mul_buffer.rs_id].inst_type = 0;
				RS[mul_buffer.rs_id].inst_id = 0;
				RS[mul_buffer.rs_id].source_a = 0;
				RS[mul_buffer.rs_id].source_b = 0;
				RS[mul_buffer.rs_id].state = 0;
				RS[mul_buffer.rs_id].write_cycle = 0;
				RS[mul_buffer.rs_id].ready = 0;
				rs_space_mul += 1;
				mul_buffer.inst_type = 0;
				mul_buffer.rs_id = 0;
				mul_buffer.source_a = 0;
				mul_buffer.source_b = 0;
				mul_buffer.state = 0;
			}
		}
		// 修改 buffer
		if (mul_buffer.state == 0) {
			for (int j = RSandADD + 1; j < RSandADD + RSandMUL + 1; j++) {
				if (RS[j].state == 1 && RS[j].ready == 1) {
					mul_buffer.state = 1;
					mul_buffer.rs_id = j;
					mul_buffer.inst_type = RS[j].inst_type;
					mul_buffer.source_a = RS[j].source_a;
					mul_buffer.source_b = RS[j].source_b;
					input_code[RS[j].inst_id].dispatch = i;
					if (RS[j].inst_type == 3) {
						RS[j].write_cycle = i + mul_cycle_amount;
					}
					else if (RS[j].inst_type == 4) {
						RS[j].write_cycle = i + div_cycle_amount;
					}
					break;
				}
			}
		}
		// issue
		if ((input_code[inst_now + 1].inst_type == 1 || input_code[inst_now + 1].inst_type == 2
			|| input_code[inst_now + 1].inst_type == 5) && rs_space_add != 0) {
			for (int j = 1; j < RSandADD + 1; j++) {
				if (RS[j].state == 0) {
					inst_now += 1;
					RS[j].state = 1;
					RS[j].inst_id = inst_now;
					RS[j].inst_type = input_code[inst_now].inst_type;
					RS[j].in_rat_id = input_code[inst_now].out_reg_id;
					if (RAT_F[input_code[inst_now].in_reg_1_id] == 0) {
						RS[j].source_a = RF_F[input_code[inst_now].in_reg_1_id];
					}
					else {
						RS[j].source_a = RAT_F[input_code[inst_now].in_reg_1_id] * RS_CHECKER;
					}
					if (RS[j].inst_type == 5) {
						RS[j].source_b = input_code[inst_now].in_reg_2_id;
					}
					else if (RAT_F[input_code[inst_now].in_reg_2_id] == 0) {
						RS[j].source_b = RF_F[input_code[inst_now].in_reg_2_id];
					}
					else {
						RS[j].source_b = RAT_F[input_code[inst_now].in_reg_2_id] * RS_CHECKER;
					}
					RAT_F[input_code[inst_now].out_reg_id] = j;
					rs_space_add -= 1;
					break;
				}
			}
		}
		else if ((input_code[inst_now + 1].inst_type == 3 || input_code[inst_now + 1].inst_type == 4) && rs_space_mul != 0) {
			for (int j = RSandADD + 1; j < RSandADD + RSandMUL + 1; j++) {
				if (RS[j].state == 0) {
					inst_now += 1;
					RS[j].state = 1;
					RS[j].inst_id = inst_now;
					RS[j].inst_type = input_code[inst_now].inst_type;
					RS[j].in_rat_id = input_code[inst_now].out_reg_id;
					if (RAT_F[input_code[inst_now].in_reg_1_id] == 0) {
						RS[j].source_a = RF_F[input_code[inst_now].in_reg_1_id];
					}
					else {
						RS[j].source_a = RAT_F[input_code[inst_now].in_reg_1_id] * RS_CHECKER;
					}
					if (RAT_F[input_code[inst_now].in_reg_2_id] == 0) {
						RS[j].source_b = RF_F[input_code[inst_now].in_reg_2_id];
					}
					else {
						RS[j].source_b = RAT_F[input_code[inst_now].in_reg_2_id] * RS_CHECKER;
					}
					RAT_F[input_code[inst_now].out_reg_id] = j;
					rs_space_mul -= 1;
					break;
				}
			}
		}
		// 更新 ready state
		for (int j = 1; j < RSandADD + RSandMUL + 1; j++) {
			if ((RS[j].source_a % RS_CHECKER != 0 && RS[j].source_a % RS_CHECKER != RS_CHECKER) || RS[j].source_a == 0) {
				if ((RS[j].source_b % RS_CHECKER != 0 && RS[j].source_b % RS_CHECKER != RS_CHECKER) || RS[j].source_b == 0) {
					RS[j].ready = 1;
				}
				else {
					RS[j].ready = 0;
				}
			}
		}
		// 檢查 cycle 是否完成
		bool complete = 0;
		if (inst_now == line_id) {
			for (int j = 1; j < rat_number + 1; j++) {
				if (RAT_F[j] != 0) {
					complete = 1;
				}
			}
			if (complete == 0) {
				cout << "\nEnd cycle: " << i << endl << endl;
				current_cycle = i;

				for (int k = 1; k < RSandADD + RSandMUL + 1; k++) {
					RS[k].inst_type = 0;
					RS[k].in_rat_id = 0;
					RS[k].inst_id = 0;
					RS[k].source_a = 0;
					RS[k].source_b = 0;
					RS[k].state = 0;
					RS[k].write_cycle = 0;
					RS[k].ready = 0;
				}
				add_buffer.inst_type = 0;
				add_buffer.rs_id = 0;
				add_buffer.source_a = 0;
				add_buffer.source_b = 0;
				add_buffer.state = 0;
				mul_buffer.inst_type = 0;
				mul_buffer.rs_id = 0;
				mul_buffer.source_a = 0;
				mul_buffer.source_b = 0;
				mul_buffer.state = 0;
				break;
			}
		}
	}
}

char print_operator(int inst_type) {
	if (inst_type == 0)
		return ' ';
	else if (inst_type == 1 || inst_type == 5)
		return '+';
	else if (inst_type == 2)
		return '-';
	else if (inst_type == 3)
		return '*';
	else if (inst_type == 4)
		return '/';
	else
		return 0;
}
