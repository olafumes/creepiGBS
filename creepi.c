#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>



char File_Name[80] = "\0";
char File_Name_Copy[80]  = "\0";
char RE_FW[4][10]; //An array of the expected restriction cut sites after bisulfite treatment in forward direction, used when looking for overlapping forward and reverse reads
char RE_REV[4][10]; // An array of the expected restriction cut sites after bisulfite treatment in reverse direction
int overlap_mismatches=0; //number of allowed mismatches in overlapping regions of forward and reverse reads
int fragment_mismatches=0; //number of mismatches allowed between forward and reverse reads and still allow them to be treated as belonging from the same fragment

typedef struct DNA_tags {
    unsigned short Sample_ID; //B -> A
    unsigned int Locus_ID; //C -> B
    char Strand; //F
    char Sequence_type[10]; //G ->C
    unsigned int Stack_component; //H ->D
    char Sequence_ID[50]; //I -> E
    char Sequence[260]; //J -> F
    char Deleveraged; //K ->G
    char Blacklisted; //L ->H
    char Lumberjackstack; //M ->I
    char RY_Sequence[260]; //Sequence converted to purines/pyrimidines
    char RY_Sequence_RC[260]; //reverse complement of above
    char Reverse_complement[260]; //reverse complement of original sequence
    char Original_Sequence[260]; //Reconstruction of original Sequence before bisulphite treatment
    char Met_Sequence[260];//Reconstruction of original sequence with methylation state: 5 = 5-Methyl-Cytosine; 6 = Gunanine with opposite 5-Methyl-Cytosine
    unsigned long Sequence_Number; 
    unsigned long RC_Sequence_Number;
    char used; // indication of whether the sequence has already been used
    }DNA_tags;

typedef struct consensus_Sequences {
    int Locus_ID;
    unsigned long Sequence_Number;
    unsigned long RC_Sequence_Number;
    char Strand;
    char Original_Sequence[260];
    char Reverse_complement[260];
    char RY_Sequence[260];
    char RY_Sequence_RC[260];
    char model_Sequence[260];
    short int Variable_Position[16];
}  consensus_Sequences;


void Komplement(char* DNA, char* DNA_Komp, int DNA_Laenge);

void Reverse_Komplement(char* DNA, char* DNA_RC, int DNA_Laenge);

void Compare(char * DNA1, char* DNA2, char* Result, int DNA_Laenge);

void To_RY(char *DNA1, char *Result, int DNA_Laenge);

char* mystrsep(char** stringp, const char* delim);

void seek_sequence_variants(unsigned long a, unsigned long b, unsigned long * Line_Counter, DNA_tags * Sequence_Information, consensus_Sequences * consensus_Table, char* OutFile);

void error(char *msg);

void to_sequence (char * Met_DNA_in, char * DNA_out);

int char_vergleich(char* String1, char* String2, int Laenge);

void * Final_Sequence(char* In_String, char * Out_String);

int charcount(FILE * const infile);



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
				strcpy(RE_FW[2], optarg);
				break;
			case '4':
				strcpy(RE_FW[3], optarg);
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
				overlap_mismatches = atoi(optarg); //convert string to int 
				break;	
			case 'M':
				fragment_mismatches = atoi(optarg);
				break;
			case'?':
				fprintf(stderr, "Problem with command line arguments. Please consult instructions! Errornumber = : '%i'\n", opterr);
				return 1;
			}
		
	}

    char *Inline = NULL; //variable to read stacks output line
    char* running = NULL; //pointer variable pointing to the current Inline
    char* token[9] ;
    const char delimiters[] = "\t";
    int i,j,l,m;	
    int Number = 1000;
    int DNA_Laenge;
    int mismatches = 0;
    int flag = 0;
    int comparison1 = 0;
    int MockSequenceLength = 0;
    int DNA_Fragments_Number = 0;
    DNA_tags Test_input;
    FILE *fp_in, *fp_out, *fp_out2, *fp_out3, *fp_out4; 
    char FileNameIn[100];
    char FileNameOut[100];
    char FileNameOut2[100];
    char FileNameOut3[100];
    char FileNameOut4[100];
    char FileNameInF[100];
    char FileNameOutF[100];
    char FileNameOut2F[100];
    char FileNameOut3F[100];
    char FileNameOut4F[100];
    char Sequence_Intermediate[350];
    char null[2] = "0";
    unsigned long pos = 0;
    unsigned char_number;
        
    strcpy(FileNameInF, File_Name);
    strcpy(FileNameOutF, File_Name);
    strcpy(FileNameOut2F, File_Name);
	strcpy(FileNameOut3F, File_Name);
	strcpy(FileNameOut4F, File_Name);
	
    snprintf(FileNameIn, 100,"%s.tsv",FileNameInF);
    snprintf(FileNameOut, 100,"%s_out.tsv",FileNameOutF);
    snprintf(FileNameOut2, 100,"%s_mock_genome.fa",FileNameOut2F);
    snprintf(FileNameOut3, 100,"%s_fragments.fa",FileNameOut3F);
    snprintf(FileNameOut4, 100,"%s_fragment_numbers.txt",FileNameOut4F);
    
    fp_in = fopen(FileNameIn, "r");
    fp_out = fopen(FileNameOut, "w");
    fp_out2 = fopen(FileNameOut2, "w");
    fp_out3 = fopen(FileNameOut3, "w");
    fp_out4 = fopen(FileNameOut4, "w");
    if (fp_in == NULL || fp_out == NULL) {
            printf("Could not open file \n");
            exit(1);
    }

DNA_tags *FeldPointer;
    FeldPointer = (DNA_tags*)malloc(Number * sizeof(DNA_tags));
    if(FeldPointer == 0){
        printf("Could not allocate memory \n");
        exit(1);
    }


	j = 0; //Number of lines in input - 1
  

    while(1) {
		pos = ftell(fp_in); 
		char_number = charcount(fp_in);
		Inline = (char*) malloc((char_number +1) * sizeof(char));
		if(Inline == NULL) break;
		fseek(fp_in, pos, SEEK_SET);
		fscanf(fp_in,"%[^\n]", Inline);
		
		if(fscanf(fp_in,"%[^\n]", Inline) == EOF) {
			free(Inline);
			break;
		}
		fgetc(fp_in);
		Inline[char_number] = '\0'; 
		running = Inline;
		if(running[0] == '#') {
			free(Inline);
			continue;
			} //do not process firstline of input, which starts with '#'
		for(i = 0; i < 9; i++) token[i] = mystrsep(&running, delimiters);
		Test_input.Sample_ID = atoi(token[0]);
		Test_input.Locus_ID = atoi(token[1]);
		Test_input.Strand = '+'; //*token[5];
		strcpy(Test_input.Sequence_type, token[2]);
		Test_input.Stack_component = atoi(token[3]);
		strcpy(Test_input.Sequence_ID, "z");
		strcpy(Test_input.Sequence, token[5]);
		DNA_Laenge = (int)strlen(Test_input.Sequence);
		Final_Sequence(&Test_input.Sequence[0], &Sequence_Intermediate[0]);
		strcpy(Test_input.Sequence, Sequence_Intermediate);
		DNA_Laenge = (int)strlen(Test_input.Sequence);
		Test_input.Deleveraged = *token[6];
		Test_input.Blacklisted = *token[7];
		Test_input.Lumberjackstack = *token[8];
		strcpy(Test_input.Reverse_complement, "0" );
		Test_input.used = '0';
		Reverse_Komplement(Test_input.Sequence,Test_input.Reverse_complement, strlen(Test_input.Sequence));
		To_RY(Test_input.Sequence,Test_input.RY_Sequence, strlen(Test_input.Sequence));
		To_RY(Test_input.Reverse_complement,Test_input.RY_Sequence_RC, strlen(Test_input.Reverse_complement));
		strcpy(Test_input.Original_Sequence, "0");
		strcpy(Test_input.Met_Sequence, "0");
		Test_input.Sequence_Number = j+1;
		FeldPointer[j] = Test_input;
		if(Test_input.Sequence_Number%Number == 0) {FeldPointer = (DNA_tags*)realloc(FeldPointer, (Test_input.Sequence_Number + Number) * sizeof(DNA_tags));}//reallocate memory if necessary
		if(FeldPointer == 0){
        printf("Could not allocate memory \n");
        exit(1);
    }
    free(Inline);
    j++;
 }

for (i =0; i<j-1; i++) {
    DNA_Laenge = (int)strlen(FeldPointer[i].Sequence);
    printf("Sequence %i: %s \n", i, FeldPointer[i].Sequence);
    if (FeldPointer[i].Strand == '-' ) continue;
    for (l = i +1; l <= j; l++){
		if (flag == 1) {flag = 0; break;}
        if (FeldPointer[l].Strand == '-' ) continue;   //next l because locus has already found to be reverse complement
        for (m = 0; m <= DNA_Laenge; m++){
            if (FeldPointer[i].RY_Sequence[m] != FeldPointer[l].RY_Sequence_RC[m]) {  //jump to next Sequence, because Sequence "i" and RC_Sequence "l" are different
				mismatches += 1;
				if (mismatches == fragment_mismatches + 1) {
					mismatches = 0;
					if (l == j) FeldPointer[i].RC_Sequence_Number = 0;
					 break;
					 }
				}
            if (m == DNA_Laenge) {
									  mismatches = 0;
									  flag = 1;
									  FeldPointer[i].RC_Sequence_Number = l;
                                      FeldPointer[l].RC_Sequence_Number = i;
                                      FeldPointer[l].Strand = '-';
                                      Compare(FeldPointer[i].Sequence, FeldPointer[l].Reverse_complement, FeldPointer[i].Met_Sequence, DNA_Laenge);
                                      to_sequence(FeldPointer[i].Met_Sequence, FeldPointer[i].Sequence);
            }
        }
    }
}


fprintf(fp_out2, ">mock_genome\n");
for(i = 0; i <= j; i++) {fprintf(fp_out,"%s\t%s\t%s\t%lu\n", FeldPointer[i].Sequence, FeldPointer[i].RY_Sequence, FeldPointer[i].Met_Sequence, FeldPointer[i].RC_Sequence_Number);
	comparison1 = strcmp(FeldPointer[i].Met_Sequence, null);
	if(comparison1 && FeldPointer[i].Strand == '+') {
		DNA_Fragments_Number += 1;
		fprintf(fp_out2, "%s", FeldPointer[i].Sequence);
		fprintf(fp_out3, ">%i\n%s\n",FeldPointer[i].Locus_ID, FeldPointer[i].Sequence);
		fprintf(fp_out4, "%i\t%s\t", MockSequenceLength, FeldPointer[i].Sequence);
		DNA_Laenge = (int)strlen(FeldPointer[i].Sequence);
		MockSequenceLength += DNA_Laenge;
		fprintf(fp_out4, "%i\n", MockSequenceLength);}}
printf("Number of reconstructed fragments: %i\n Reached end\n", DNA_Fragments_Number);
free(FeldPointer);
fclose(fp_in);
fclose(fp_out);
fclose(fp_out2);
fclose(fp_out3);
fclose(fp_out4);
    return 0;
}

void error(char *msg) {
	fprintf(stderr, "%s: %s \n", msg, strerror(errno));
	exit(1);
}


void Komplement (char * DNA, char* DNA_Komp, int DNA_Laenge){
	int i;
	for (i = 0; i < DNA_Laenge; ++i)  {
	switch(DNA[i]) {
		case 'A':
			DNA_Komp[i] = 'T';
			break;
		case 'C':
			DNA_Komp[i] = 'G';
			break;
		case 'G':
			DNA_Komp[i] = 'C';
			break;
		case 'T':
			DNA_Komp[i] = 'A';
			break;
		case 'R':
			DNA_Komp[i] = 'Y';
			break;
		case 'Y':
			DNA_Komp[i] = 'R';
			break;
		}
	}
DNA_Komp[DNA_Laenge] = '\0';
}


void Reverse_Komplement(char *DNA, char* DNA_RC, int DNA_Laenge) {
int i,j;
for (i = DNA_Laenge -1, j = 0; i >= 0; --i,++j)  {
	switch(DNA[i]) {
		case 'A':
			DNA_RC[j] = 'T';
			break;
		case 'C':
			DNA_RC[j] = 'G';
			break;
		case 'G':
			DNA_RC[j] = 'C';
			break;
		case 'T':
			DNA_RC[j] = 'A';
			break;
		case 'R':
			DNA_RC[j] = 'Y';
			break;
		case 'Y':
			DNA_RC[j] = 'R';
			break;
		}
	}
DNA_RC[DNA_Laenge] = '\0';
}

void Compare(char * DNA1, char* DNA2, char* Result, int DNA_Laenge) {
	int i;
	for(i = 0; i < DNA_Laenge; i++) {
		if ((DNA1[i] == 'A') & (DNA2[i] == 'A')) {Result[i] = 'A';continue;}
		if ((DNA1[i] == 'A') & (DNA2[i] == 'G')) {Result[i] = 'G';continue;}
		if ((DNA1[i] == 'C') & (DNA2[i] == 'C')) {Result[i] = '5';continue;}
		if ((DNA1[i] == 'C') & (DNA2[i] == 'T')) {Result[i] = 'C';continue;}
		if ((DNA1[i] == 'G') & (DNA2[i] == 'G')) {Result[i] = '6';continue;}
		if ((DNA1[i] == 'T') & (DNA2[i] == 'T')) {Result[i] = 'T';continue;}
		if ((DNA1[i] == 'A') & (DNA2[i] == 'C')) {Result[i] = 'M';continue;}
		if ((DNA1[i] == 'A') & (DNA2[i] == 'T')) {Result[i] = 'W';continue;}
		if ((DNA1[i] == 'C') & (DNA2[i] == 'A')) {Result[i] = 'M';continue;}
		if ((DNA1[i] == 'C') & (DNA2[i] == 'G')) {Result[i] = 'S';continue;}
		if ((DNA1[i] == 'G') & (DNA2[i] == 'A')) {Result[i] = 'R';continue;}
		if ((DNA1[i] == 'G') & (DNA2[i] == 'C')) {Result[i] = 'S';continue;}
		if ((DNA1[i] == 'G') & (DNA2[i] == 'T')) {Result[i] = 'K';continue;}
		if ((DNA1[i] == 'T') & (DNA2[i] == 'A')) {Result[i] = 'W';continue;}
		if ((DNA1[i] == 'T') & (DNA2[i] == 'C')) {Result[i] = 'Y';continue;}
		if ((DNA1[i] == 'T') & (DNA2[i] == 'G')) {Result[i] = 'K';continue;}
		}
		Result[DNA_Laenge] = '\0';
}

void To_RY(char* DNA1, char* Result, int DNA_Laenge) {
    int i;
    for (i = 0; i < DNA_Laenge; i++) {
        if((DNA1[i] == 'A') | (DNA1[i] == 'G')) {Result[i] = 'R';continue;}
        if((DNA1[i] == 'C') | (DNA1[i] == 'T')) {Result[i] = 'Y';continue;}
    }
    Result[DNA_Laenge] = '\0';
}


char* mystrsep(char** stringp, const char* delim)
{
  char* start = *stringp;
  char* p;

  p = (start != NULL) ? strpbrk(start, delim) : NULL;

  if (p == NULL)
  {
    *stringp = NULL;
  }
  else
  {
    *p = '\0';
    *stringp = p + 1;
  }
  return start;
}

// Function to seek sequence pairs in order to reconstruct methylated state of original sequences
void seek_sequence_variants(unsigned long a, unsigned long b, unsigned long * P_Line_Counter, DNA_tags * Sequence_Information, consensus_Sequences * consensus_Table, char * OutFile) {
	FILE *fp_out;
	DNA_tags variant_sequence_1[16];  //array for different primary sequences of orientation 1
	DNA_tags variant_sequence_2[16];
	DNA_tags To_Output;
	char DNA_Sequence[260] = "0";
	int i = 1;
	unsigned int Stack_Component = 0;
	unsigned long counter1 = consensus_Table[a].Sequence_Number+1;
	unsigned long counter2 = consensus_Table[b].Sequence_Number+1; //two ahead in array; array starts with 0, sequence number with 1
	fp_out = fopen(OutFile, "a");
	variant_sequence_1[0] = Sequence_Information[counter1];

	while(strcmp(Sequence_Information[counter1+1].Sequence_type, "primary") == 0){  // look for different stack components within a stack
		if(Sequence_Information[counter1+1].Stack_component == Sequence_Information[counter1].Stack_component) {
                counter1 ++; continue;}
		variant_sequence_1[i] = Sequence_Information[counter1+1];
		counter1++;
		i++;
	}

	int j = 1;	//the same as above but for the '-' strand

	counter2 = consensus_Table[b].Sequence_Number+1;

	variant_sequence_2[0] = Sequence_Information[counter2];

	while(strcmp(Sequence_Information[counter2+1].Sequence_type, "primary") == 0){
		if(Sequence_Information[counter2+1].Stack_component == Sequence_Information[counter2].Stack_component)
			{counter2++; continue;}
		variant_sequence_2[j] = Sequence_Information[counter2+1];
		counter2++;
		j++;
	}

	int k,l;	//search which stack components of "+" stack are the reverse complement of which stack component of "-" stack
	for (k = 0;k < i; k++) {
		for (l = 0;l < j; l++) {
			if(strcmp(variant_sequence_1[k].RY_Sequence, variant_sequence_2[l].RY_Sequence_RC) == 0) {
				Compare(variant_sequence_1[k].Sequence, variant_sequence_2[l].Reverse_complement, variant_sequence_1[k].Met_Sequence, (int)strlen(variant_sequence_1[k].Sequence)); //determine original Sequence with methylation state
				Compare(variant_sequence_1[k].Sequence, variant_sequence_2[l].Reverse_complement, variant_sequence_2[l].Met_Sequence, (int)strlen(variant_sequence_2[l].Sequence));
				To_Output = variant_sequence_1[k];
				To_Output.Sequence_Number = *P_Line_Counter;
				To_Output.Stack_component = Stack_Component;
            	(*P_Line_Counter)++;
            	to_sequence(To_Output.Met_Sequence,DNA_Sequence);
				fprintf(fp_out,"%lu\t%i\t%s\t%s\t%s\n", To_Output.Sequence_Number, To_Output.Locus_ID, To_Output.Sequence, To_Output.Met_Sequence, DNA_Sequence);
				counter1 = variant_sequence_1[k].Sequence_Number;
				while(strcmp(Sequence_Information[counter1].Sequence,Sequence_Information[counter1 - 1].Sequence)== 0){
                    strcpy(Sequence_Information[counter1].Met_Sequence, variant_sequence_1[k].Met_Sequence);
                    To_Output = Sequence_Information[counter1];
                    To_Output.Sequence_Number = *P_Line_Counter;
                    to_sequence(To_Output.Met_Sequence,DNA_Sequence);
                    (*P_Line_Counter)++;
                    counter1++;
                    fprintf(fp_out,"%lu\t%i\t%s\t%s\t%s\n", To_Output.Sequence_Number, To_Output.Locus_ID, To_Output.Sequence, To_Output.Met_Sequence, DNA_Sequence);
				}

				To_Output = variant_sequence_2[l];
				To_Output.Sequence_Number = *P_Line_Counter;
				To_Output.Stack_component = Stack_Component;
				To_Output.Locus_ID = Sequence_Information[counter1-1].Locus_ID;
				(*P_Line_Counter)++;
				to_sequence(To_Output.Met_Sequence,DNA_Sequence);
				fprintf(fp_out,"%lu\t%i\t%s\t%s\t%s\n", To_Output.Sequence_Number, To_Output.Locus_ID, To_Output.Sequence, To_Output.Met_Sequence, DNA_Sequence);
				counter2 = variant_sequence_2[l].Sequence_Number;
				while(strcmp(Sequence_Information[counter2].Sequence,Sequence_Information[counter2 - 1].Sequence)== 0){
                    strcpy(Sequence_Information[counter2].Met_Sequence, variant_sequence_1[l].Met_Sequence);
                    To_Output = Sequence_Information[counter2];
                    To_Output.Sequence_Number = *P_Line_Counter;
                    To_Output.Locus_ID = Sequence_Information[counter1-1].Locus_ID;
                    (*P_Line_Counter)++;
                    counter2++;
                    to_sequence(To_Output.Met_Sequence,DNA_Sequence);
                    fprintf(fp_out,"%lu\t%i\t%s\t%s\t%s\n", To_Output.Sequence_Number, To_Output.Locus_ID, To_Output.Sequence, To_Output.Met_Sequence, DNA_Sequence);
				}
			Stack_Component++;
}}}
fclose(fp_out);
}


void to_sequence(char * Met_DNA_in, char * DNA_out){
     int i = 0;
     while(Met_DNA_in[i] != '\0')
        {
            switch(Met_DNA_in[i]){
            case '5':
                DNA_out[i] = 'C';
                i++;
                break;
            case '6':
                DNA_out[i] = 'G';
                i++;
                break;
            default:
                DNA_out[i] = Met_DNA_in[i];
                i++;}
        }
        DNA_out[i] = '\0';
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
	if(RE_FW[3][0] != '\0'){k = 3;}
	 // This part is when there is an RE cutsite within the SE length. The cutsite should be found in both reads at he same distance from the beginning
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
	return NULL;  //this eliminates 8 'As' from usearch
}

int char_vergleich(char* String1, char* String2, int Laenge) { //helper function for Final_Sequence, looks for identical substrings
	int i;
	int Zaehler = 0;
	for (i=0; i < Laenge;i++) {
		if (String1[i] != String2[i]) Zaehler +=1;
	}
return Zaehler;
}	

int charcount(FILE *const infile) {
	int c, count;
	
	count = 0;
	for( ; ; ) {
		c = fgetc(infile);
		if(c == EOF || c == '\n') break;
		++count;
	}
	return count;
}
