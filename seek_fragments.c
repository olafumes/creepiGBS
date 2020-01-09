#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>


char File_Name_Positions[80];
char File_Name_Positions_Copy[80];
char File_Name_fragments[80];
char File_Name_fragments_Copy[80];
char File_Name_output[80];

typedef struct DNA_fragments {
    unsigned long Beginning;
    char Sequence[250];
    unsigned long End; //A
    }DNA_fragments;
    
  
char* mystrsep(char** stringp, const char* delim);
    
int main(int argc, char **argv)
{
	
	char option;
	int i = 0;
	DNA_fragments Test_input;
	long int Base_Number=0;
	long int Sequence_Number=0;
	long int Last_Number = 0;
	int Fragments_Number = 0;
	
	while((option = getopt(argc, argv, "p:f:o:")) != EOF) { //option:get filename
		switch(option) {
			case 'p':
				strcpy(File_Name_Positions, optarg);
				printf("%s\n", File_Name_Positions);
				break;
			case 'f':
				strcpy(File_Name_fragments, optarg);
				printf("%s\n", File_Name_fragments);
				break;
			case 'o':
				strcpy(File_Name_output, optarg);
				printf("%s\n", File_Name_output);
				break;	
			default:
				fprintf(stderr, "Unknown Option: '%s'\n", optarg);
				return 1;
			}
		}
			
	char Inline[500]; //variable to read input line of sequence file with numbers
    char* running; //pointer variable pointing to the current Inline
    char* token[3];
    const char delimiters[] = "\t";
    int j;	//K
    int Number = 1000;
    
     FILE *fp_in_Sequence, *fp_in_Numbers, *fp_output; 
    fp_in_Sequence = fopen(File_Name_fragments, "r");
    fp_in_Numbers = fopen(File_Name_Positions, "r");
    fp_output = fopen(File_Name_output, "w"); 
    
	DNA_fragments *FeldPointer;
    FeldPointer = (DNA_fragments*)malloc(Number * sizeof(DNA_fragments));
    if(FeldPointer == 0){
        printf("Could not allocate memory \n");
        exit(1);
    }
	
	j = 0; //Number of lines in input - 1
  
    while(fscanf(fp_in_Sequence,"%[^\n]", Inline) != EOF) {
    fgetc(fp_in_Sequence);
    running = Inline;
    for(i = 0; i < 3; i++) token[i] = mystrsep(&running, delimiters);
    Test_input.Beginning = atoi(token[0]);
    strcpy(Test_input.Sequence, token[1]);
    Test_input.End = atoi(token[2]);
    FeldPointer[j] = Test_input;
    if((j+1)%Number == 0) {FeldPointer = (DNA_fragments*)realloc(FeldPointer, (j+1 + Number) * sizeof(DNA_fragments));}//reallocate memory if necessary
    if(FeldPointer == 0){
        printf("Could not allocate memory \n");
        exit(1);
    }
    
    j++;                                                           

    
}
fclose(fp_in_Sequence);
while(fscanf(fp_in_Numbers,"%[^\n]", Inline) != EOF) {
	fgetc(fp_in_Numbers);
	Base_Number = atoi(Inline);
	if(Base_Number > Last_Number){;
	for(i=Sequence_Number; i < j; i++) {
		if(FeldPointer[i].Beginning <= Base_Number && FeldPointer[i].End >= Base_Number) {
			Sequence_Number = i;
			Last_Number = FeldPointer[i].End;
			fprintf(fp_output, ">fragment%i\n%s\n", i+1, FeldPointer[i].Sequence);
			Fragments_Number +=1;
			break;
		}}
	}}
	printf("Fragment number: %i\n", Fragments_Number);
	return 0;
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
