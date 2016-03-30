#include<stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nr.h"
#include "nrutil.h"

#define DEBUG 0
#define PROGNAME "temp_date.exe"

#define MAXLINE 500 
void abort_run()
/* This function aborts program run */
{
	extern void exit();
 
	fprintf(stderr, "%s: Aborting Run... \n", PROGNAME);
	exit(1);

}
 
void check_file(filename,fptr)
char *filename;
FILE *fptr;
/* Returns error if couldn't open file: filename */
{
	if(fptr==0) {
		printf("Can't open %s \n",filename);
		abort_run();
	}
	else {
		if(DEBUG)
			printf("Opened file %s \n", filename);
	}
}
 
void read_flow_archive(input_file,dvec)
/* Reads the date from the flow archive file that
is used to create the realtime.real file when 
gastemps.sh is run. 
The date has the form of:   yymmdd   string value
and is stored in dvec as mmddyy.
*/
 
char *input_file;
char *dvec;
{
	char foo[MAXLINE],temp[20];
	int i;
	FILE *fptr;
 
	/* Open Flow archive */
 
	   fptr = fopen(input_file, "r");
	   check_file(input_file,fptr);
 
	/* Skip header */
	   fgets(foo,MAXLINE,fptr);
	   fscanf(fptr,"%s", foo);
	   fscanf(fptr,"%s", foo);
	   fscanf(fptr,"%s",temp);
	
	   fclose(fptr);
	   
	/* Reformat */

	    for(i=0; i<6; i++)
		dvec[i] = temp[i];
	    dvec[6] = '\0';

	
}

void output_date_file(dvec)
char *dvec; 
{
	printf("%s", dvec);
}

int main(int argc, char **argv)
{
	char input_file[20];
	char dvec[20];

	if(argc != 2)
	{
		fprintf(stderr, "Usage:  temp_date.exe flow_archive_file\n");
		return 1;
	}

	read_flow_archive(argv[1], dvec);
	output_date_file(dvec);

	return 0;
		
} 
