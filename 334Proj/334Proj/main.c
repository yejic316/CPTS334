#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

typedef struct btb {
	char currPC[17];
	char tarPC[17];
	char pred[4];
	int IND;
}BTB;

char * hexTobin();
int bstr_to_dec(char * str);
int findIndex();
void execute();
char *decToHexa(int n);
void displayBTB(BTB btb[]);
void clear_bin();
void printTrace(BTB btb[]);

//global
char hex[17], pcCurr[17] = "", pcPrev[17] = "", pcPlusFour[17] = "", prevPCplusFour[17] = "", bin[65] = "";
int index;
FILE *fptr;
char dec_hex[17], *read;
int hit = 0, miss = 0, right = 0, wrong = 0, collisions = 0, br_w_stall = 0, br_w_o_stall=0;
float hitRate = 0, accuracy = 0;

void main()
{
	char filename[15];
	char entry[128];

	char currPred[8];
	BTB ourBTB[1024] = { { "", "", "", 0 }, };

	printf("Enter the filename to be opened \n");
	scanf("%s", filename);

	/*  open the file for reading */
	fptr = fopen(filename, "r");
	if (fptr == NULL)
	{
		printf("Cannot open file \n");
		exit(0);
	}
	
	execute();

	int prevIndex;
	while (!feof(fptr))
	{

		//Adress in BTB?
		if (strcmp(ourBTB[index].currPC, pcCurr) == 0)
		{ //Yes
			hit += 1;
			strcpy(prevPCplusFour, pcPlusFour);
			prevIndex = index;
	
			execute();
			//Prediction right?
			if (strcmp(pcCurr, prevPCplusFour) == 0) 
			{ //No (br. wasn't taken, change pred. to 10? does it matter whether pred. was previously 00 or 01?)
				wrong += 1;
				if (strcmp(ourBTB[prevIndex].pred, "00") == 0) //|| (strcmp(ourBTB[prevIndex].pred, "01") == 0))
					strcpy(ourBTB[prevIndex].pred, "01"); 
				else if (strcmp(ourBTB[prevIndex].pred, "01") == 0)
					//strcpy(ourBTB[prevIndex].pred, "10");// basic state machine
					strcpy(ourBTB[prevIndex].pred, "11");// state machine A //state machine C
				else
					strcpy(ourBTB[prevIndex].pred, "11");
			}
			else 
			{ //Yes (br. was taken,samps
				//Address right?
				if (strcmp(pcCurr, ourBTB[prevIndex].tarPC) == 0) 
				{ //Yes
					right += 1;
					br_w_o_stall++;
					if (strcmp(ourBTB[prevIndex].pred, "11") == 0) //|| (strcmp(ourBTB[prevIndex].pred, "01") == 0))
						strcpy(ourBTB[prevIndex].pred, "10");
					else if (strcmp(ourBTB[prevIndex].pred, "10") == 0)
						//strcpy(ourBTB[prevIndex].pred, "01");//basic state machine // state machine A
						strcpy(ourBTB[prevIndex].pred, "00");//state machine C
					else
						strcpy(ourBTB[prevIndex].pred, "00");
				}
				else
				{//No, i.e. branch taken, but to a different address(update BTB)
					wrong += 1;
					br_w_stall++;
					collisions++;
					strcpy(ourBTB[prevIndex].tarPC, pcCurr);
					strcpy(ourBTB[prevIndex].pred, "00"); //MAKE SURE THIS IS THE DEFAULT TAKEN
				}
			}
		}
		else { //No, PC not in BTB
			strcpy(pcPrev, pcCurr);
			strcpy(prevPCplusFour, pcPlusFour);
			prevIndex = index;

			execute();
			if (strcmp(pcCurr, prevPCplusFour) != 0) 
			{ 	//Br.was taken
				miss += 1;
				br_w_stall++;
				if (strcmp("", ourBTB[prevIndex].currPC) != 0 )// if ourBTB[index].currPC is not empty 
					collisions++;
				strcpy(ourBTB[prevIndex].currPC, pcPrev);
				strcpy(ourBTB[prevIndex].tarPC, pcCurr);
				strcpy(ourBTB[prevIndex].pred, "00");
				ourBTB[prevIndex].IND = prevIndex;
			}
		}
	}

	fclose(fptr);
	displayBTB(ourBTB);
	printTrace(ourBTB);

	hitRate = (float)hit * 100 / (hit + miss);
	accuracy = (float)right * 100 / (right + wrong);
	printf("\nThe number of hit : %d Miss : %d, Right : %d, Wrong : %d\n", hit, miss, right, wrong);
	printf("collisions = %d    br_w_stall = %d   br_w_o_satll = %d\n", collisions, br_w_stall, br_w_o_stall);
	printf("Hit rate: %.2f%%, Accuracy: %.2f%%\n", hitRate, accuracy);
}

void clear_bin() {
	int i;
	for (i = 0; i < 65; i++) {
		bin[i] = '\0';
	}
}

char * hexTobin() {
	int i;
	clear_bin();
	for (i = 0; hex[i] != '\0'; i++)
	{
		switch (hex[i])
		{
		case '0':
			strcat(bin, "0000");
			break;
		case '1':
			strcat(bin, "0001");
			break;
		case '2':
			strcat(bin, "0010");
			break;
		case '3':
			strcat(bin, "0011");
			break;
		case '4':
			strcat(bin, "0100");
			break;
		case '5':
			strcat(bin, "0101");
			break;
		case '6':
			strcat(bin, "0110");
			break;
		case '7':
			strcat(bin, "0111");
			break;
		case '8':
			strcat(bin, "1000");
			break;
		case '9':
			strcat(bin, "1001");
			break;
		case 'a':
		case 'A':
			strcat(bin, "1010");
			break;
		case 'b':
		case 'B':
			strcat(bin, "1011");
			break;
		case 'c':
		case 'C':
			strcat(bin, "1100");
			break;
		case 'd':
		case 'D':
			strcat(bin, "1101");
			break;
		case 'e':
		case 'E':
			strcat(bin, "1110");
			break;
		case 'f':
		case 'F':
			strcat(bin, "1111");
			break;
		case '+':
			strcat(bin, "++++");
			break;

		default:
			printf("Invalid hexadecimal input.");
		}
	}

	return 0;
}

int bstr_to_dec(char * str)
{
	int val = 0;

	while (*str != '\0')
		val = 2 * val + (*str++ - '0');
	return val;
}

int findIndex() {
	char temp[16];

	strncpy(temp, (bin + 12), 10);
	temp[10] = '\0';
	return bstr_to_dec(temp);

}

void execute() {
	int pc_in_dec;
	read = fgets(hex, 17, fptr);
	hex[6] = '\0';
	hexTobin();
	strcpy(pcCurr, hex);

	if (bin[8] == '+') {
		strcpy(pcPlusFour, "4E+101");
	}
	else {
		pc_in_dec = bstr_to_dec(bin);
		decToHexa(pc_in_dec + 4);
		strcpy(pcPlusFour, dec_hex);
	}
	index = findIndex();
}


// function to convert decimal to hexadecimal
char *decToHexa(int n)
{
	// char array to store hexadecimal number
	char hexaDeciNum[17];

	// counter for hexadecimal number array
	int i = 0;
	while (n != 0)
	{
		// temporary variable to store remainder
		int temp = 0;

		// storing remainder in temp variable.
		temp = n % 16;

		// check if temp < 10
		if (temp < 10)
		{
			hexaDeciNum[i] = temp + 48;
			i++;
		}
		else
		{
			hexaDeciNum[i] = temp + 87;
			i++;
		}

		n = n / 16;
	}

	// printing hexadecimal number array in reverse order
	for (int j = i - 1; j >= 0; j--)
		dec_hex[i - j - 1] = hexaDeciNum[j];
	return &dec_hex;
	//printf("hex = %s\n", dec_hex);
}

void displayBTB(BTB btb[]) {
	int i;

	printf("Index	Current PC       Target PC     Pred.\n");
	for (i = 0; i < 1024; i++) {
		if (strcmp(btb[i].currPC, "") != 0)
			printf("%d		%s             %s            %s\n", btb[i].IND,btb[i].currPC, btb[i].tarPC, btb[i].pred);
	}
}

void printTrace(BTB btb[]) {
	FILE *fptr2;
	int i;
	fptr2 = fopen("resultTrace_Doduc_FP(C).txt", "w");
	if (fptr2 == NULL)
	{
		printf("Cannot open file \n");
		exit(0);
	}
	fprintf(fptr2, "Index	Current PC	Target PC	Pred.\n");
	for (i = 0; i < 1024; i++) {
		if (strcmp(btb[i].currPC, "") != 0)
			fprintf(fptr2, "%4d	%6s	%6s	%2s\n", btb[i].IND, btb[i].currPC, btb[i].tarPC, btb[i].pred);
	}
	fprintf(fptr2, "hits = %d    misses = %d   right = %d   wrong = %d\n", hit, miss, right, wrong);
	fprintf(fptr2, "collisions = %d   br_w_stall = %d    br_w_o_stall = %d\n", collisions, br_w_stall, br_w_o_stall);
	fprintf(fptr2, "Hit rate = %.2f        accuracy = %.2f\n", hitRate, accuracy);

}