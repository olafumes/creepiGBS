#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


int main(int argc, char **argv)
{
	char option;
	char In_File[80] = "0";
	char Out_File[80] = "0";
	size_t C_Count = 0;
	size_t T_Count = 0;
	size_t elsecount = 0;
	size_t LineCount = 0;
	char Inline[500] = "0";
	double percentage = 0.0;
	unsigned position = 0;
	char P[4];

	
	
	while((option = getopt(argc, argv, "i:o:p:")) != EOF) { //option:get filename
		switch(option) {
			case 'i':
				strcpy(In_File, optarg);
				//printf("%s\n", File_Name_fragments);
				break;
			case 'o':
				strcpy(Out_File, optarg);
				//printf("%s\n", File_Name_output);
				break;	
			case 'p':
				strcpy(P, optarg);
				position = atoi(P);
				break;
				
			default:
				fprintf(stderr, "Unknown Option: '%s'\n", optarg);
				return 1;
			}
		}
	
	FILE *fp_in_Sequence, *fp_output; 
    fp_in_Sequence = fopen(In_File, "r");
    fp_output = fopen(Out_File, "a"); 
         
     while(fscanf(fp_in_Sequence,"%[^\n]", Inline) != EOF) {
		 fgetc(fp_in_Sequence);
		 if (LineCount %4 == 1) { 
		 if (Inline[position - 1] == 'C') C_Count += 1;
		 if (Inline[position - 1] == 'T') T_Count += 1;
		 if (Inline[position - 1] == 'A' || Inline[position - 1] == 'G' ) elsecount += 1;
	}
	LineCount += 1;
}
	percentage = 100 * ((double)T_Count / ((double)T_Count + (double)C_Count));
	printf("Filename: %s\t T_Count %ld \t C_Count %ld \t Percentage % .3f \n" , In_File, T_Count, C_Count, percentage);
	fprintf(fp_output, "Filename: %s\tT_Count %ld \t C_Count %ld \t Percentage % .3f\n", In_File, T_Count, C_Count, percentage);
	fclose(fp_in_Sequence);
	fclose(fp_output);
	return 0;


}
