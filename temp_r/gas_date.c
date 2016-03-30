#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nr.h"
#include "nrutil.h"
#include "julian.h"

#define MAXLINE 500 
void abort_run()
/* This function aborts program run */
{
        extern void exit();
 
        printf("Aborting Run... \n");
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

	    for(i=0; i<4; i++)
		dvec[i] = temp[i+2];
	    dvec[4] = temp[0];
	    dvec[5] = temp[1];
	    dvec[6] = '\0';
	
}

void output_date_file(dvec)
char *dvec; 
{
	FILE *optr;
	char output_file[30];
	sprintf(output_file,"date_file");
	optr = fopen(output_file,"w");
	check_file(output_file, optr);
	fprintf(optr,"%s \n",dvec);
	fclose(optr);
}

void output_julian_file(dvec)
char *dvec;
{
	FILE *fptr,*optr;
	char output_file[30],month[3],day[3];
	int mo,year,da,julian;

	int	length;
	char	*realdir,
			*relpath = "gas/current_year",
			*path;

	realdir = getenv("REALDIR");
	if(realdir == NULL)
	{
		fprintf(stderr, "Can't determine RT path (set REALDIR environment variable).\n");
		abort_run();
	}

	length = 2 + strlen(realdir) + strlen(relpath);
	path = (char *) malloc(sizeof(char) * length);
	if(path == NULL)
	{
		fprintf(stderr, "output_juldate_file():	malloc failed.\n");
		abort_run();
	}
	strcat(path, (const char *) realdir);
	strcat(path, (const char *) "/");
	strcat(path, (const char *) relpath);

	sprintf(output_file,"julian_date_file");
	fptr = fopen(path, "r");
	check_file(path, fptr);
	fscanf(fptr,"%d",&year);
	fclose(fptr);

	sprintf(month,"%c%c",dvec[0],dvec[1]);
	sprintf(day,"%c%c",dvec[2],dvec[3]);
	sscanf(month,"%d",&mo);
	sscanf(day,"%d",&da);

	julian = get_julian(mo,da,year);
	printf("julian = %d \n",julian);

	optr = fopen(output_file,"w");
	check_file(output_file, optr);
	fprintf(optr,"%d",julian);
	fclose(optr);

}
	
int main(int argc, char **argv)
{
	char	input_file[20],
			dvec[20];

	if(argc != 2)
	{
		fprintf(stderr, "Usage:  %s flow_archive_file\n", argv[0]);
		return -1;
	}
	
	strcpy(input_file, argv[1]);
	read_flow_archive(input_file,dvec);
	output_date_file(dvec);
	output_julian_file(dvec);

	return 0;		
} 
