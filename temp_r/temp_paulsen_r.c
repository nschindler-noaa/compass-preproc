/* 
 * Modified May 2006 by WNB, CBR 
 * Shaw model replaced by the Paulsen model which has more parameters.
 * these are hardcoded in the dam structure below.
 * POD documentation not used and removed.
 * inc_temps not used and both the function and the calls have been removed.
 * Backcasting is removed.
 * Forecasting at tail end of season removed.
 *
 *
 */

//#include <math.h>
#include <iostream>
#include <string>
//#include <stdio.h>
//#include "nrutil.h"
//#include <pwd.h>
//#include <time.h>
#include <sys/types.h>
#include <errno.h>
#include <stdlib.h>

#include <QFile>
#include <QDir>
#include <QString>
#include <QTime>

using namespace std;

#define USAGE     "Usage: temp_paulsen_r.exe temp_directory mean_input_directory year flowpred_file\n" 
#define PROGNAME  "temp_paulsen_r.exe"

#define MAXLINE       500
#define COMMENT_CHAR  '#'
/* Need to define BACKCASTFLAG with the same value in temp_update_temp.c also. */
#define BACKCASTFLAG  101.0
/* the USEMEANFLOW flag indicates that no flow prediction is available
 * for this station and so the historic mean flows should be substituted...
 * This parameter needs to be defined in   temp_r_read_dat.c   also.                  */
#define USEMEANFLOW   1001

#define NDAMS         16    /* number of dams and stations to loop through  */

/* Here is the new dam structure based on Charlie Paulsen Temp write-up 05-11-06.doc */
struct dam {	
	char *name;
	float A0_intercept;
	float B1_flow;
	float B2_flow_square;
	float B3_flow_day;
	float B4_flow_day_average;
	float B5_temp_day_average;
};

/* the MCN values are substituted for CHJ, PRD and other dams on the Columbia etc. for testing purposes */
struct dam dams[] = {	
	{"CHJ",4.12965,-0.05571,-0.00016933,-0.00428,0.00462,0.99836},  
    {"IHR",2.34844,-0.08042,0.0006054,-0.00601,0.00591,0.99932}, 
    {"LMN",0.78727,-0.0264,0.0001684,-0.00688,0.00711,1.00279}, 
    {"LGS",1.24208,-0.02648,0.0001238,-0.01015,0.00864,0.98725}, 
    {"LWG",1.69431000,-0.06328000,0.0005415000,-0.01118000,0.01101000,0.99806000}, 
    {"BON",2.88159,-0.02864,0.0000658,-0.00181,0.00197,1.00149}, 
    {"TDA",2.91641,-0.02605,0.0000496,-0.00173,0.00188,1.00103}, 
    {"JDA",5.28766,-0.04972,0.0001107,-0.00312,0.00285,0.99371}, 
    {"MCN",4.64304,-0.04157,.00008808,-0.004,0.00379,0.99203}, 
    {"PRD",4.12965,-0.05571,-0.00016933,-0.00428,0.00462,0.99836},  
    {"WAN",4.12965,-0.05571,-0.00016933,-0.00428,0.00462,0.99836},  
    {"RRH",4.12965,-0.05571,-0.00016933,-0.00428,0.00462,0.99836},  
    {"RIS",4.12965,-0.05571,-0.00016933,-0.00428,0.00462,0.99836},  
    {"WEL",4.12965,-0.05571,-0.00016933,-0.00428,0.00462,0.99836},  
	{"13334300",1.36669,-0.07423,0.00087618,-0.01350,0.01361,0.99922},   
	{"13341050",1.61592,-0.26790,0.01034,-0.07476,0.07459,0.99901}
};

/* This function aborts program run */
void abort_run()
{
	extern void exit();

	cerr << PROGNAME << ":  Aborting run... " << endl;
//	fprintf(stderr, "%s:  Aborting Run... \n", PROGNAME);
	exit(1);
}

/* Returns error if couldn't open file: filename */
void check_file (char *filename, FILE *fptr)
{
	if (fptr == 0) 
	{
		cout << "Can't open " << filename << "." << endl;
//		printf ("Can't open %s \n",filename);
		abort_run();
	}
	else 
	{
		cout << "Opened file " << filename << "." << endl;
//		printf ("Opened file %s \n", filename);
	}
}

/* This function makes sure j, the number of data points in 
 * filename, is 365
*/
void check_365 (char *filename, int j)
{
	if (j != 365) 
	{
		cout << "Not enough data. Need 365, but only " ;
		cout << j << " lines in " << filename << endl;
//		printf (" Not enough data (%i lines) in %s \n", j, filename);
		abort_run();
	}
}

/* prints a couple of lines of header information to ofp */
int print_header (FILE *ofp)
{
	extern uid_t getuid ();
	extern int gethostname ();
 
	struct passwd *pwd;
	time_t current_time;
	char hostname_buffer[256];
 
	pwd = getpwuid (getuid ());
	time (&current_time);
	gethostname (hostname_buffer, sizeof(hostname_buffer));
 
	if (putc (COMMENT_CHAR, ofp) == EOF)
	return -1;
	if (putc ('\n', ofp) == EOF)
	return -1;
 
	fprintf (ofp, "%c Written by user %s (%s) on %s", COMMENT_CHAR,
		(pwd && pwd->pw_name) ? pwd->pw_name   : "<Unknown>",
		(pwd && pwd->pw_gecos) ? pwd->pw_gecos : "<Unknown>",
		ctime (&current_time));
	fprintf (ofp, "%c\ton host %s\n", COMMENT_CHAR, hostname_buffer);
	return 0;
}
 

/* Reads in mean temperature data and year to date temperature.
 * temp_pred is set to be the mean when no current data is available.
 * If the last day is julian day 70 or later, predict up to 100 days of
 * additional data using meantemp.
 * Return  the last day of year to date temperature. 
*/
int init_temps (float *meantemp, float *temp_pred, char *temp_file, char *mean_file)
{
	char foo[MAXLINE];
	int julian,lastday,skip;
	FILE *temp_fptr,*mean_fptr;
	float temp;
	/* added parameters for backCasting  DS */
	int firstdata;  /* will be first jdate of valid data  */
	
	/* Read in mean data */	
	mean_fptr = fopen (mean_file, "r");
	check_file (mean_file, mean_fptr);
	fgets (foo, MAXLINE, mean_fptr);
	while (((fscanf(mean_fptr, "%d %f %d", &julian, &temp, &skip)) !=EOF ))
	{
		meantemp[julian]=temp;
		temp_pred[julian] = meantemp[julian];
	}
	check_365 (mean_file, julian);
	fclose (mean_fptr);

	/* Read in year to date data */	
	temp_fptr = fopen (temp_file, "r");
	check_file (temp_file, temp_fptr);
	lastday = 0;
	while (((fscanf (temp_fptr, "%d %f ", &julian, &temp)) !=EOF )) 
	{
		if (temp > 0.0) 
		{
			temp_pred[julian] = temp;
			lastday = julian;
			/* check if BackCasting is needed  */
			if (temp == BACKCASTFLAG) 
			{
			    firstdata = julian + 1;
			}
		}
	}
	fclose(temp_fptr);

	return(365);

		
	/* ==================================================== 
	* BackCasting  ....DS2002  
	* Removed WNB May 2006                          
	
	* If a significant amount of data continue on current trend
	* until start of flow related predicitons 
	* increment method removed WNB May 2006
	*
	* =====================================================
	*/
}

/* Reads in mean flows, predicted flows, and year to date flows.
 * flow_pred is set to year-to-date values where available and
 * predicted flow values otherwise.	 
*/ 
void init_flows (float *meanflow, float *flow_pred, char *flowpred_fh, FILE *flow_file, char *mean_file, char *dam)
{
	char  foo[MAXLINE];
	int   julian,
		  jj,
		  skip;
	FILE  *flow_fptr,
		  *mean_fptr;
	float flow;

	int read_flowpred ();

	/* Read in mean data */ 
	mean_fptr = fopen (mean_file, "r");
	check_file (mean_file, mean_fptr);
	fgets (foo, MAXLINE, mean_fptr);
	while ((fscanf (mean_fptr, "%d %f %d", &julian, &flow, &skip)) != EOF)
	{
		meanflow[julian]=flow;
	}
	check_365 (mean_file, julian);
	fclose (mean_fptr);

	/* Read in flow prediction file */
	julian = read_flowpred (flowpred_fh, dam, flow_pred);
	if (julian == USEMEANFLOW) 
	{
	    for (jj = 0; jj < 365; jj++) 
		{
		flow_pred[jj] = meanflow[jj];
	    }
	}
	else
	{
	    check_365("flowpred", julian);
	}
 
	/* Read in year to date flow data */
	flow_fptr = fopen (flow_file, "r");
	check_file (flow_file, flow_fptr);
	while ((fscanf (flow_fptr, "%d %f ", &julian,&flow)) != EOF) 
	{
		if ( flow>0.0)
		{
			flow_pred[julian] = flow;
		}
	}
	fclose (flow_fptr);
}

/* This makes a temperature prediction based on a function of flow.*/	
void predict1_temp(float *flow_pred, float *meanflow, float *meantemp, float *temp_pred, struct dam *thisdam, int START_PRED)
{
	int	j,jj;
	float yearmeanflow;
	float	A0_INTERCEPT,
			B1_FLOW,
			B2_FLOW_SQUARE,
			B3_FLOW_DAY,
			B4_FLOW_DAY_AVERAGE,
			B5_TEMP_DAY_AVERAGE;
		
	A0_INTERCEPT		= thisdam->A0_intercept;
	B1_FLOW             = thisdam->B1_flow;
	B2_FLOW_SQUARE      = thisdam->B2_flow_square;
	B3_FLOW_DAY         = thisdam->B3_flow_day;
	B4_FLOW_DAY_AVERAGE = thisdam->B4_flow_day_average;
	B5_TEMP_DAY_AVERAGE = thisdam->B5_temp_day_average;

	yearmeanflow=0;	
	for (jj=1; jj<=365; jj++) 
	{
		yearmeanflow += flow_pred[jj];
	}
	yearmeanflow = yearmeanflow / 365;
	
	/* for(j = START_PRED ; j <= 365 ; j++)*/
	for(j = 0 ; j <= 365 ; j++)
	{
		temp_pred[j] = A0_INTERCEPT + 
				B1_FLOW * yearmeanflow + 
				B2_FLOW_SQUARE * yearmeanflow * yearmeanflow + 
				B3_FLOW_DAY * flow_pred[j] +
				B4_FLOW_DAY_AVERAGE * meanflow[j] +  
				B5_TEMP_DAY_AVERAGE * meantemp[j]; 
	}
}

int main (int argc, char **argv)
{
	char	temp_file[MAXLINE],
			flow_file[MAXLINE],
			mean_flow_file[MAXLINE],
			mean_temp_file[MAXLINE],
			out_file[MAXLINE],
			*mean_dir,
			*flowpred_file,
			*out_dir, /* added to control local output WNB*/
			*temp_dir;
	FILE	*flowpred_fh,
			*optr;
	float	*meantemp,
			*meanflow,
			*flow_pred,
			*temp_pred;
	int	init_temps(),
			START_PRED,
			ph_out;
	int	i,
			j,
			last_day,
			YEAR;
	int	print_header ();
	struct dam *thisdam;
	void abort_run ();
	fpos_t	pos;

	if (argc != 5)
	{
		fprintf (stderr, "%s", USAGE);
		return 1;
	}
	temp_dir = argv[1];
	mean_dir = argv[2];
	flowpred_file = argv[4];
	out_dir = "temp_r.out.dir";   /*  WNB temporary to control local output */
	
	errno = 0;
	YEAR = strtol (argv[3], (char**) NULL, 10);
	if (YEAR == 0 || errno != 0)
	{
		fprintf (stderr, "Error converting year to an integer: %s\n", argv[3]);
		return 1;
	}

	meantemp = vector (1,365);
	meanflow = vector (1,365);
	flow_pred = vector (1,365);
	temp_pred = vector (1,365);

	flowpred_fh = fopen (flowpred_file, "r");
	check_file (flowpred_file, flowpred_fh);
	if (fgetpos (flowpred_fh, &pos) != 0)
	{
		fprintf (stderr, "Error getting position in %s", flowpred_file);
		perror (NULL);
		exit (1);
	}

	/* Loop through each dam */
	for (i = 0; i < NDAMS; i++)
	{
		thisdam = &dams[i];	 
	
		fsetpos (flowpred_fh, &pos);
		/* data files*/
		sprintf (flow_file, "%s/flow%s.%d", temp_dir, thisdam->name, YEAR);
		sprintf (temp_file, "%s/temp%s.%d", temp_dir, thisdam->name, YEAR);
		sprintf (mean_temp_file, "%s/meantemp.%s", mean_dir, thisdam->name);
		sprintf (mean_flow_file, "%s/meanflow.%s", mean_dir, thisdam->name);
		 
		/* Read in historical mean temperatures into meantemp, 
		 * and observed data into temp_pred.  if last_day > 70 then 
		 * predict up to 100 days of data and write that into temp_pred 
		 * too. Holes in observed data in temp_pred are filled in with 
		 * values from meantemp. 
		*/
		last_day = 1 + init_temps (meantemp,temp_pred,temp_file,mean_temp_file);

		/* START_PRED used to be set by the forecast day.
		NOW is independent of that so that a complete water year is forecast
		START_PRED = (last_day > 100) ? last_day : 100 ;  
		START_PRED = (last_day > 1) ? last_day : 1 ;
		*/
		START_PRED = 0;
		
		/* Read in historical mean flows, predicted flows, and observed
		 * YTD flows. Missing data in observed flows is filled in with
		 * predicted flows.
		*/ 
		init_flows (meanflow, flow_pred, flowpred_fh, flow_file, mean_flow_file,
			thisdam->name);
		
		/* Use flow prediction data, meanflow, and meantemp to predict
		 * temperature data for days in the range [ START_PRED, 272 ].
		 * unless modified above. 
		*/
		predict1_temp (flow_pred, meanflow, meantemp, temp_pred, thisdam, 
			START_PRED);


		/* Output */
		/* The old one that SLI was using. change it be a local one for Nick
		* sprintf(out_file,"%s/rTEMP_%s", temp_dir, thisdam->name); 
		* UNIX side control
		* 
		* WNB temporary to control local output 
		* sprintf(out_file,"%s/rTEMP_%s", out_dir, thisdam->name);  
		*/
		sprintf (out_file,"%s/rTEMP_%s", temp_dir, thisdam->name); 
		optr = fopen (out_file,"w");
		check_file (out_file,optr);
		ph_out = print_header (optr);
		if (ph_out != 0)
		{
			printf ("Write error occured in output file \n");
			abort_run ();
		}
		fprintf (optr, " \n");
		/* CRiSP expects Dec31 from previous year so Jan1 is repeated
		*/
		fprintf (optr, "%f \n", temp_pred[1]);
		for (j = 1; j <= 364; j = j+4) 
			fprintf (optr, "%f %f %f %f\n", temp_pred[j],
				temp_pred[j+1], temp_pred[j+2], temp_pred[j+3]);
		fprintf (optr, "%f \n", temp_pred[365]);
		fclose (optr);
	}
	return 0;
}
