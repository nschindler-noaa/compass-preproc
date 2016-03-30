/*
	tab width set to 3 spaces
*/


/* POD documentation

=head1 NAME

temp_r.exe - Predict temperature to the end of the year

=head1 SYNOPSIS

temp_r.exe TEMP_DIRECTORY MEAN_INPUT_DIRECTORY YEAR FLOWPRED_FILE

=head1 DESCRIPTION

B<temp_r.exe> uses observed and predicted flow data, and observed
and predicted temperature data to predict a years worth of temperature
data for LGR, PRD, and TDA.  (Actually there are 366 data points in
the rTEMP's because CRiSP wants a December 31 in there, so there are
two data points from January 1.)

=head1 The temperature prediction steps

=over

=item * from F<meantemp.DAM>

Initially all missing days are filled in with values from
F<meantemp.DAM>.

=item * based on last day of data and data from F<meantemp.DAM>

If there is data after March 10 (julian day 70), up to 100 days will
be projected forward based on the last 3 days of observed data and the
data in F<meantemp.DAM>. F<meantemp.DAM> has been read into into
the I<meantemp> variable in the code snippet below. Here is the
heart of this stage of prediction:

 if(last_day_of_data > 70 && last_day_of_data < 365) {
    // Average the last 3 days of observed data to get the first day of
    // predicted data 
    julian = 1 + last_day_of_data;
    temp_pred[julian] = 0;
    for(j = 1 ; j <= 3 ; j++)
       temp_pred[julian] += temp_pred[julian-j];
    temp_pred[julian] = temp_pred[julian]/3.0;

    // Find the increments of increase/decrease in the mean temp data 
    for(j = 2 ; j <= 365 ; j++)
      inc[j] = meantemp[j] - meantemp[j-1];
    // Predict no more than 100 days of data using these increments and
    // the first day of data based on the last 3 days of observed data 
    for(j = last_day_of_data + 1 ; j < 100 ; j++)
      temp_pred[j] = temp_pred[j-1]+inc[j];
 }

=item * based on flow data and temperature data

Missing days in the range 100 to 272 are predicted using the flow
and temperature data with this algorithm:

 for(j = 1 + last_day_of_data ; j <= 272 ; j++)
    temppred[j] = meantemp[j]+B0_FLOW+B1_FLOW*(flowpred[j]-meanflow[j]);

Data from F<summary.alt1> is first read into the I<flowpred> variable,
and then any days of data from F<flowDAM.YEAR> is written into the
I<flowpred> variable. The I<meantemp> variable is data from
F<meantemp.DAM>. Here are the values of B0_FLOW and B1_flow for the
locations (updated DS&NB 4/02):

Location	B0      B1 
       ---------------
CHJ		0.0	0.00187884532 
IHR		0.0	-0.0145351785 
LMN		0.0	-0.0099626503 
LGS		0.0	-0.0160505825 
LWG		0.0	-0.0152362973 
BON		0.0	-0.0043770060 
TDA		0.0	-0.0015191452 
JDA		0.0	-0.0055892750 
MCN		0.0	-0.0076976137 
PRD		0.0	-0.0085965643 
WAN		0.0	-0.0025145659 
RRH		0.0	-0.0102809333 
RIS		0.0	-0.0079651068 
WEL		0.0	-0.0009238544
13334300	0.0	-0.00001908619  Anatone
13341050	0.0	-0.00007100836  Peck  		

=item * Re-do some work

The first 3 weeks of missing data are predicted (again) using the same
exact algorithm as in step 2 above. I believe this no longer occurs as
of 2002 (SI).

=item * Fade back to other predictions

The next 4 weeks (after the 3 weeks in the previous step) are
predicted by gradually switching from the means of predicting in the
previous step to the already-predicted data. The I<temppred>
variable is the predicted and observed data thus far, the I<inc>
variable is the same array as in the second step.

 k = 0;
 for(j = last_day_of_data + 28 ; j <= last_day_of_data + 49 ; j++) {
   temppred[j] = (21.0-k)/21.0*(temppred[j-1]+inc[j]) + k/21.0*temppred[j];
   k++;
 }

=back

=head1 INPUT FILES

    TEMPORARY-DIR/summary.alt1 (from CRiSP)
    TEMPORARY-DIR/flowDAM.YEAR (from update_temps.exe)
    TEMPORARY-DIR/tempDAM.YEAR (from update_temps.exe)
    data/temperature/predictions/meanflow.DAM (permanent, updated annually)
	//daily historical mean for all years
    data/temperature/predictions/meantemp.DAM (permanent, updated annually)
	//daily historical mean for all years

=head1 OUTPUT FILES

    TEMPORARY-DIR/rTEMP_DAM

=cut

*/

/* This program must be compiled with "nrutil.c".  This program computes 
 * temperature predictions for all dams listed in "struct dam dams"; currently
 * 16 locations throughout Snake & Columbia River.
*/

#include <math.h>
#include <stdio.h>
#include "nrutil.h"
//#include <pwd.h>
#include <time.h>
#include <sys/types.h>
#include <errno.h>
#include <stdlib.h>

#define USAGE         "Usage:  temp_r.exe temp_directory mean_input_directory year flowpred_file\n" 
#define PROGNAME      "temp_r.exe"

#define MAXLINE       500
#define COMMENT_CHAR  '#'
/* Need to define BACKCASTFLAG with the same value in temp_update_temp.c also. */
#define BACKCASTFLAG  101.0
/* the USEMEANFLOW flag indicates that no flow prediction is available
 * for this station and so the historic mean flows should be substituted...
 * This parameter needs to be defined in   temp_r_read_dat.c   also.                  */
#define USEMEANFLOW   1001

#define NDAMS         16    /* number of dams and stations to loop through  */

struct dam {
	char *name;
	float B0_flow;
	double B1_flow;
};
/* These coeff come from analysis summer and winter through 2001*/
/* If you add a location, make sure to update dam loop upper limit NDAMS above */
/* New regression paramters... B0_Flow = 0.0 for all dams DS&NB 4/02 */
struct dam dams[] = {	
	{"CHJ",0.0,0.00187884532}, 
	{"IHR",0.0,-0.0145351785}, 
	{"LMN",0.0,-0.0099626503}, 
	{"LGS",0.0,-0.0160505825}, 
	{"LWG",0.0,-0.0152362973}, 
	{"BON",0.0,-0.0043770060}, 
	{"TDA",0.0,-0.0015191452}, 
	{"JDA",0.0,-0.0055892750}, 
	{"MCN",0.0,-0.0076976137}, 
	{"PRD",0.0,-0.0085965643}, 
	{"WAN",0.0,-0.0025145659}, 
	{"RRH",0.0,-0.0102809333}, 
	{"RIS",0.0,-0.0079651068}, 
	{"WEL",0.0,-0.0009238544},
	{"13334300",0.0,-0.00001908619},  /* Anatone */
	{"13341050",0.0,-0.00007100836}    /* Peck   */		
};


/* This function aborts program run */
void abort_run ()
{
	extern void exit ();

	fprintf (stderr, "%s:  Aborting Run... \n", PROGNAME);
	exit (1);
}

/* Returns error if couldn't open file: filename */
void check_file (char *filename, FILE *fptr)
{
	if (fptr == 0) 
	{
		printf ("Can't open %s \n",filename);
		abort_run ();
	}
	else 
	{
		printf ("Opened file %s \n", filename);
	}
}

/* This function makes sure j, the number of data points in 
 * filename, is 365
*/
void check_365 (char *filename, int j)
{
	if (j != 365) 
	{
		printf (" Not enough data (%i lines) in %s \n", j, filename);
		abort_run ();
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
	int j,julian,lastday,skip;
	FILE *temp_fptr,*mean_fptr;
	float temp,*inc;
	/* added parameters for backCasting  DS */
	int firstdata;  /* will be first jdate of valid data  */
	int nd = 3;     /* num days to ave for first point - if poss. re-use */
	float tempdif;  /* difference between first prediction and hist data */
	float tmptemp;

	
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
	/* ====================================================*/
	/* BackCasting  ....DS2002                            */
	/*  back-prediction needed?  */
	if (firstdata > 0) 
	{
    
	/* find first back-predicted point as average   */
	/* of first 3 data points if available          */
    
	    if (firstdata >= lastday - 1) 
		{
		nd = 2;
		if (firstdata == lastday) 
		{
		    nd = 1;
		}
	    }
	    julian = firstdata - 1;
	    temp_pred[julian] = 0.0;
	    for (j = 1; j <= nd; j++) 
		{
		temp_pred[julian] += temp_pred[julian + j];
	    }
	    temp_pred[julian] = temp_pred[julian]/nd;

	    /* back-predict 10 days by shifting historical data parallel */
	    nd = 10;
	    if (julian < nd) 
		{
		nd = julian;
	    }
	    tempdif = temp_pred[julian] - meantemp[julian];
	    for (j =1; j < nd; j++) 
		{
		tmptemp = meantemp[julian-j] + tempdif;
		temp_pred[julian-j] = (tmptemp >= 0.0) ? tmptemp : 0.0;
	    }

	    /* back-predict next 20 days by slowly migrating to 
	     * historical data */
	    julian = julian - nd;
	    nd = 20;
	    if (julian < nd) 
		{
		nd = julian;
	    }
	    for (j = 0; j < nd; j++)
		{
		tmptemp = meantemp[julian-j] + tempdif*(20-j)/20.0;
		temp_pred[julian-j] = (tmptemp >= 0.0) ? tmptemp : 0.0;
	    }

	    /* set the rest of the back-predicted temperatures to historic
	     * levels */
	    for (j = julian-nd; j > 0; j--)
		{
		temp_pred[j] = meantemp[j];
	    }
	}

	/* End BackCasting  .....                               */
	/* =====================================================*/
	
	/* If a significant amount of data continue on current trend
	 * until start of flow related predicitons 
	*/
	if (lastday > 70 && lastday < 365)
	{
		/* To start, use yesterdays temperatue for an estimator */
		/* for today.  Average last 3 days as a safety against */
		/* bad data.*/

		julian = lastday + 1;
		temp_pred[julian] = 0;
		for(j = 1 ; j <= 3 ; j++)
		{
			temp_pred[julian] += temp_pred[julian-j];
		}
		temp_pred[julian] = temp_pred[julian]/3.0;
 
		/* Use the average increments to augment temperature*/ 
 
		inc = vector (1, 365);
		inc[1] = 0.0;
		for (j = 2 ; j <= 365 ; j++) 
		{
			inc[j] = meantemp[j] - meantemp[j-1];
		}
		
		for (j = julian + 1 ; j < 100 ; j++)
		{
			temp_pred[j] = temp_pred[j-1]+inc[j];
		}
	}

	free_vector (inc, 1, 365);

	return (lastday);
}

/* Reads in mean flows, predicted flows, and year to date flows.
 * flow_pred is set to year-to-date values where available and
 * predicted flow values otherwise.	 
*/ 
void init_flows (float *meanflow, float *flow_pred, char *flowpred_fh, FILE *flow_file, char *mean_file, char *dam)
{
	char	foo[MAXLINE];
	int	julian,jj,
			skip;
	FILE	*flow_fptr,
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

/* This makes a temperature prediction based on a linear function of flow.*/	
void predict1_temp(float *flow_pred, float *meanflow, float *meantemp, float *temp_pred, struct dam *thisdam, int START_PRED)
{
	int	j;
	float	B0_FLOW,
			B1_FLOW;

	B1_FLOW = thisdam->B1_flow;
	B0_FLOW = thisdam->B0_flow;
	
	for (j = START_PRED ; j <= 272 ; j++)
	{
		temp_pred[j] = meantemp[j]+ B0_FLOW + B1_FLOW*(flow_pred[j]-meanflow[j]);
	}
}

/* This function first continues the trend of last 5 days of data to 
 * get first prediction on day START_PRED. The next four weeks are
 * found by using mean temperature increments.	The next three weeks is a 
 * weighted average of the mean incremented temperature and the
 * flow adjusted temperature already in temppred[]. The remaining
 * data remains as the flow adjusted temperature.
*/ 
/* Actually the code is currently set up in predict1_temp so that 
 * START_PRED>=100 because the data before that is not necesarily reliable.
 * But this can be changed so that START_PRED>=1, and in that case
 * inc_temp needs no changing.
*/
void inc_temp(float *meantemp, float *temppred, int START_PRED)
{
	float	*inc;
	int	j,
		k;

/* Seems a bit squirrly. Currently using another starting point.
 * If START_PRED>=5: Use regression to start the first day of 
 * predictions as a continuation of the least-squares line through 
 * the last n=5  days of data if START_PRED<5, its too early in year
 * so just use the meandata already in temp_pred to begin
 * temperature prediction on julian day START_PRED.
	*  if( START_PRED>=5){
	* n = 5;
	* sum_X = 0.0;
	* sum_Xsq = 0.0;
	* sum_XY = 0.0;
	* sum_Y = 0.0;
	* for(k=1; k<=n; k++){
	* 	sum_X += k;
	* 	sum_Xsq += k*k;
	* 	sum_Y += temppred[START_PRED-k];
	* 	sum_XY += k*temppred[START_PRED-k];
	* }
	* b1 = ( sum_XY-(sum_X*sum_Y/n) ) / ( sum_Xsq-(sum_X/n) );
	* b0 = (sum_Y - b1*sum_X)/n;
	* temppred[START_PRED] = b1*(n+1) + b0;
	*   }
*/ 

	/* I don't think this function is correct... but...
	 * Anyway, this 'if' below is my attempt at stopping it from crashing.
	 *	-- gabriel
	*/
	if (START_PRED + 49 > 365)
	{
		return;
	}

	/* To start, use yesterdays temperatue for an estimator for today!
	 * Average last 3 days as a safety against bad data 
	 *	Recall: START_PRED >= 100
	*/

	temppred[START_PRED] = 0;
	for (j = 1 ; j <= 3 ; j++)
	{
		temppred[START_PRED] += temppred[START_PRED-j];
	}
	temppred[START_PRED] = temppred[START_PRED] / 3.0;
 
	/* Use the average increments to augment temperature for
	 * the first four weeks of the predictive interval 
	*/
 	inc = vector (1, 365);
	inc[1] = 0.0;
	for (j = 2; j <= 365; j++) 
	{
		inc[j] = meantemp[j] - meantemp[j-1];
	}
	for (j = START_PRED+1; j < START_PRED+28; j++)
	{
		temppred[j] = temppred[j-1]+inc[j];
	}
 
	/* Slowly progress back to the flow adjusted mean temperature
	 * prediction
	*/
	k = 0;
	for (j = START_PRED+28; j <= START_PRED+49; j++)
	{
		temppred[j] = (21.0-k)/21.0*(temppred[j-1]+inc[j]) + k/21.0*temppred[j];
		k++;
	}

	free_vector (inc, 1, 365);
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
		last_day = 1 + init_temps (meantemp, temp_pred, temp_file, mean_temp_file);

		START_PRED = (last_day > 100) ? last_day : 100 ;

		/* Read in historical mean flows, predicted flows, and observed
		 * YTD flows. Missing data in observed flows is filled in with
		 * predicted flows.
		*/ 
		init_flows (meanflow, flow_pred, flowpred_fh, flow_file, mean_flow_file,
			thisdam->name);
		
		/* Use flow prediction data, meanflow, and meantemp to predict
		 * temperature data for days in the range [ START_PRED, 272 ].
		*/
		predict1_temp (flow_pred, meanflow, meantemp, temp_pred, thisdam, 
			START_PRED);

		/* Using the last 3 days of observed data in 'temp_pred',predict
		 * the next 3 weeks of data, then, while predicting the next
		 * 4 weeks of data, transition back to the predicted data
		 * already in 'temp_pred'. 
		*/
		inc_temp (meantemp, temp_pred, START_PRED);

		/* Output */
		sprintf (out_file, "%s/rTEMP_%s", temp_dir, thisdam->name);
		optr = fopen (out_file, "w");
		check_file (out_file, optr);
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
