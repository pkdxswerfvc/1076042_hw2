# 1076042 HW2 Instruction cheduling
此檔案介紹如何使用此程式

## 執行方式
輸入 input 檔案檔名。

## 程式流程
	1. Issue  
	2. Load or Store  
	3. Load only  
	4. Store only  

## FUNCTION
char print_operator(int inst_type);	// 輸出控制字元  
void print_result();	// 印出結果  
void scheduling();		// 指令排序  

## INPUT FILE
輸入檔為 `Instruction`。
* **Example:** test_case.txt  

		ADDI F1, F2, 1  
    	SUB F1, F3, F4  
		DIV F1, F2, F3  
		MUL F2, F3, F4  
		ADD F2, F4, F2  
		ADDI F4, F1, 2  
		MUL F5, F5, F5  
		ADD F1, F4, F4  

## OUTPUT FILE
輸出檔包含 `RF`、`RAT` 、`RS` 以及 `Buffer`。
* **Example:** test_case_1.txt、test_case_63.txt

test_case_1.txt

		Cycle: 1

		RF: 
		F1: 0
		F2: 2
		F3: 4
		F4: 6
		F5: 8

		RAT: 
		F1: RS1
		F2: 
		F3: 
		F4: 
		F5: 

		RS: 
		RS1: + 2 1 
		RS2 
		RS3 

		Buffer: X
		RS4: 
		RS5: 

		Buffer: X


test_case_63.txt	

		Cycle: 63

		RF: 
		F1: 4
		F2: 30
		F3: 4
		F4: 2
		F5: 8

		RAT: 
		F1: 
		F2: 
		F3: 
		F4: 
		F5: RS4

		RS: 
		RS1 
		RS2 
		RS3 

		Buffer: X

		RS4: * 8 8 
		RS5: 

		Buffer: (RS4) 8 * 8

