/*
*********************************************
*  314 Principles of Programming Languages  *
*  Spring 2017                              *
*  Author: Ulrich Kremer                    *
*  Student Version                          *
*********************************************
*/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "InstrUtils.h"
#include "Utils.h"

void deallocateNoncritical(Instruction* head) {
	Instruction* current = head;
	while (current != NULL) {
		if (current->critical == 'n') {
			Instruction* temp = current;

			//Head case
			if (current == head) {
				head = current->next;
				head->prev = NULL;
			}

			//Last or middle node case
			else {
				current->prev->next = current->next;
				current->next->prev = current->prev;
			}

			current = current->next;

			//Free
			temp->prev = NULL;
			temp->next = NULL;
			free(temp);
			temp = NULL;
		}

		//Otherwise advance
		else {
			current = current->next;
		}
	}
}

void deallocateAll(Instruction* head) {
	Instruction* current = head;
	head = NULL;
	while (current != NULL) {
		Instruction* temp = current;
		current = current->next;

		//Free
		temp->prev = NULL;
		temp->next = NULL;
		free(temp);
		temp = NULL;
	}
}

int main()
{
	Instruction *head;

	head = ReadInstructionList(stdin);
	if (!head) {
		WARNING("No instructions\n");
		exit(EXIT_FAILURE);
	}

	//Count number of memory offsets used so array can be initialized
	int offsetCount = 0;
	Instruction* current = head;
	while (current != NULL) {
		if (current->opcode == STOREAI || current->opcode == LOADAI || current->opcode == OUTPUTAI) {
			offsetCount++;
		}
		current = current->next;
	}

	//Initialize arrays
	//4096 possible registers
	//Index is register number; 1 is critical
	int criticalReg[4096];

	//Initialize to 0
	int i;
	for (i = 0; i < 4096; i++) {
		criticalReg[i] = 0;;
	}

	//offsetCount possible offsets
	//Assume that offset is always from r0
	//We can make this assumption becasue we wrote the compiler to behave this way
	int criticalOffset[offsetCount];

	//Initialize to -1
	for (i = 0; i < offsetCount; i++) {
		criticalOffset[i] = -1;
	}

	//iterator for criticalOffset
	int offsetIterator = 0;

	current = head;
	//Initialize to outputAI instructions
	//Otherwise not critical
	while (current != NULL) {
		if (current->opcode == OUTPUTAI) {
			current->critical = 'y';
			//field1 is target register; important
			criticalReg[current->field1] = 1;
			//field2 is offset
			criticalOffset[offsetIterator] = current->field2;
			offsetIterator++;
		}
		else {
			current->critical = 'n';
		}
		//Next instructions
		current = current->next;
	}

	//Loop through all instructions, starting from the end
	current = LastInstruction(head);

	while (current != NULL) {
		if (current->critical == 'n') {

			//Case ADD, SUB, MUL, DIV
			if (current->opcode == ADD || current->opcode == SUB || current->opcode == MUL || current->opcode == DIV) {
				//If the result is stored in a critical register
				//Then field1 and field2 are also critical registers
				if (criticalReg[current->field3] == 1) {
					criticalReg[current->field1] = 1;
					criticalReg[current->field2] = 1;
					current->critical = 'y';
				}
			}

			//Case LoadI
			if (current->opcode == LOADI) {
				//If the destination register is a critical register
				//Then the instruction is critical
				if (criticalReg[current->field2] == 1) {
					current->critical = 'y';
				}
			}

			//Case LoadAI
			if (current->opcode == LOADAI) {
				//If destination register mataches
				if (criticalReg[current->field3] == 1) {
					criticalOffset[offsetIterator] = current->field2;
					offsetIterator++;
					current->critical = 'y';
				}
			}

			//Case StoreAI
			if (current->opcode == STOREAI) {
				//If offset matches
				int i;
				for (i = 0; i <= offsetIterator; i++) {
					if (criticalOffset[i] == current->field3) {
						criticalReg[current->field1] = 1;
						//Only keep last storeAI; all earlier others are not important
						criticalOffset[i] = -1;
						current->critical = 'y';
						break;
					}
				}
			}
		}

		//Next instruction
		current = current->prev;
	}

	//Deallocate noncritical instructions
	deallocateNoncritical(head);

	if (head)
	PrintInstructionList(stdout, head);

	//Deallocate all instructions
	deallocateAll(head);

	return EXIT_SUCCESS;
}
