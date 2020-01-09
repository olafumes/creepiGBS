#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>


char File_Name[80];
char File_Name_Copy[80];
char RE_FW[4][10]; //An array of the expected restriction cut sites after bisulfite treatment in forward direction, used when looking for overlapping forward and reverse reads
char RE_REV[4][10]; // An array of the expected restriction cut sites after bisulfite treatment in reverse direction
int overlap_mismatches=0; //number of allowed mismatches in overlapping regions of forward and reverse reads
int fragment_mismatches=0; //number of mismatches allowed between forward and reverse reads and still allow them to be treated as belonging from the same fragment

void * Final_Sequence(char* In_String, char * Out_String);
int char_vergleich(char* String1, char* String2, int Laenge);

int main(int argc, char **argv)
{
    
char ch;

 RE_FW[0][0] = RE_FW[1][0] = RE_FW[2][0] = RE_FW[3][0] = '\0';
 RE_REV[0][0] = RE_REV[1][0] = RE_REV[2][0] = RE_REV[3][0] = '\0';
	
while((ch = getopt(argc, argv, "f:1:2:3:4:5:6:7:8:O:M:")) != EOF) { //option:get filename
		switch(ch) {
			case 'f':
				strcpy(File_Name, optarg);
				break;
			case '1':
				strcpy(RE_FW[0], optarg);
				break;
			case '2':
				strcpy(RE_FW[1], optarg);
				break;	
			case '3':
				strcpy(RE_REV[0], optarg);
				break;
			case '4':
				strcpy(RE_REV[1], optarg);
				break;
			case '5':
				strcpy(RE_REV[0], optarg);
				break;
			case '6':
				strcpy(RE_REV[1], optarg);
				break;	
			case '7':
				strcpy(RE_REV[2], optarg);
				break;
			case '8':
				strcpy(RE_REV[3], optarg);
				break;	
			case 'O':
				overlap_mismatches = *optarg -48; //convert char to int (0 = ASCII 48)
				break;	
			case 'M':
				fragment_mismatches = *optarg -48;
				break;
			case'?':
				fprintf(stderr, "Problem with command line arguments. Please consult instructions! Errornumber = : '%i'\n", opterr);
				return 1;
			}
		
	}	
					
 FILE *fp_in, *fp_out, *fp_out2;
 char FileNameIn[100];
 char FileNameOut[100];
 char FileNameOut2[100];
 char FileNameInF[100];
 char FileNameOutF[100];
 char FileNameOutF2[100];
 
 strcpy(FileNameInF, File_Name);
 strcpy(FileNameOutF, File_Name);
 strcpy(FileNameOutF2, File_Name);
 snprintf(FileNameIn, 100,"%s.fq",FileNameInF);
 snprintf(FileNameOut, 100,"%s_merged.fq",FileNameOutF);
 snprintf(FileNameOut2, 100,"%s_merged.fa",FileNameOutF2);
 fp_in = fopen(FileNameIn, "r");
 fp_out = fopen(FileNameOut, "w");
 fp_out2 = fopen(FileNameOut2, "w");
 printf("%s\n", FileNameIn);
 
 char running[4][250]; 
 char Inline[250]; //pointer variable pointing to the current Inline
 int i = 0;
 int k = 0;
 int j;
 char Out_Sequence[250];
 char Out_Quality[250];
 int Seq_Length_PE_original;
 int Seq_Length_PE_final;
 int Seq_Length_SE;
    
 while(fscanf(fp_in,"%[^\n]", Inline) != EOF) {
    fgetc(fp_in);
    k = i%4;
    strcpy(running[k],Inline);
    printf("%i: %s\n", k, Inline);
    printf("%i: %s \n", k, running[k]);
    if(k==3){
		Final_Sequence(running[1], Out_Sequence);
		Seq_Length_PE_original = (int)strlen(running[1]);
		Seq_Length_PE_final = (int)strlen(Out_Sequence); 
		Seq_Length_SE = Seq_Length_PE_final/2;
		printf("PE_final %i\n", Seq_Length_PE_final);
		if(2*Seq_Length_SE == Seq_Length_PE_final){
			for(j = 0; j < Seq_Length_SE; j++) {
				Out_Quality[j] = running[3][j];
				Out_Quality[j+Seq_Length_SE] = running[3][Seq_Length_PE_original - Seq_Length_SE + j];}
			        Out_Quality[Seq_Length_PE_final] = '\0';}
		else {for(j = 0; j <= Seq_Length_SE; j++) {
				Out_Quality[j] = running[3][j];
				Out_Quality[j+Seq_Length_SE+1] = running[3][Seq_Length_PE_original - Seq_Length_SE + j];}}
			
			
		printf("i: %i \n%s\n%s\n%s\n%s\n", i, running[0], running[1], running[2], running[3]);
		fprintf(fp_out,"%s\n%s\n%s\n%s\n", running[0], Out_Sequence, running[2], Out_Quality);
		fprintf(fp_out2,">%s\n%s\n",running[0], Out_Sequence);
		
	}
i++;
}
fclose(fp_in);
fclose(fp_out);
fclose(fp_out2);
}

void* Final_Sequence(char* In_String, char * Out_String) {	//Function to shorten joined F and R reads if overlapping or staggering
	
	char String0[250];
	char String1[125];
	char String2[125];
	int Seq_Len_PE;
	int Seq_Len_SE;
	strcpy(String0, In_String);
	char Outstring[250];
	char * cptr1;
	char * cptr2;
	int Unterschiede =0;
	int i = 0;
	int j = 0;
	int k = 0;
	Seq_Len_PE = (int)strlen(String0); //Length of total Sequence including padding (8  bases) 
	Seq_Len_SE = (Seq_Len_PE - 8)/2;	//Length of individual FW and Rev reads
	for(i = 0; i <Seq_Len_SE; i++) {
		String1[i] = String0[i];
		String2[i] = String0[i+ 8 + Seq_Len_SE];
	}
	String1[Seq_Len_SE] = '\0';
	String2[Seq_Len_SE] = '\0';
	for(i = 0; i <= strlen(String1)-15; i++) {
		Unterschiede = char_vergleich(&String1[i], &String2[0], strlen(String1)-i);
		if (Unterschiede <= overlap_mismatches){
		String1[i]  = '\0';
		strcpy(Outstring, String1);
		strcat(Outstring, String2);
		strcpy(Out_String, Outstring);
		return NULL;}
	}
	
	
	
	if(RE_FW[1][0] != '\0'){k = 1;}
	if(RE_FW[2][0] != '\0'){k = 2;}
	if(RE_FW[3][0] != '\0'){k = 3;} // This part is if there is an RE cutsite within the SE length. The cutsite should be found in both reads at he same distance from the beginning
		
	for(i = 0; i <= k; i++){
		for(j = 0; j <= k; j++) {
	cptr1 = strstr(String1, RE_FW[i]);
	cptr2 = strstr(String2, RE_REV[j]);
	if((cptr1 -&String1[0]) + (cptr2 - &String2[0])  == strlen(String2) - strlen(RE_FW[i])){
	strcpy(Outstring, String1);
	Outstring[strlen(RE_FW[i])+cptr1-&String1[0]] = '\0';	
	strcpy(Out_String, Outstring);
	return NULL;}}}
	
	
	strcpy(Outstring, String0);
	for(i = Seq_Len_SE; i <= 2*Seq_Len_SE; i++) Outstring[i] = String0[i+8]; 
	strcpy(Out_String, Outstring);
	return NULL;  //this eliminates 8 'As' from usearch*/
}

int char_vergleich(char* String1, char* String2, int Laenge) { //helper function for Final_Sequence, looks for identical substrings
	int i;
	int Zaehler = 0;
	for (i=0; i < Laenge;i++) {
		if (String1[i] != String2[i]) Zaehler +=1;
	}
return Zaehler;
}	

