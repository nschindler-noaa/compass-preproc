/*
	tab width at 3 spaces
*/

/* POD documentation

=head1 NAME

gas_update_gas.exe - Predict gas values for the whole year.

=head1 SYNOPSIS

gas_update_gas.c CURRENT-YEAR DAM1 [ DAM2 [...]]

=head1 DESCRIPTION

DAM == CHJ DWR LWG

outputgasDAM.YEAR has a column with the date and another with the
amount of dissolved gas in the water (dis_gas).   gasavg_DAM holds 12
numbers: the average value for each month.   spillDWR.YEAR has a
julian data column and a spill column.   The spill data is only used
for DWR.   I don't know where mysterygasDAM.YEAR is used... thus the
name.   outputgas_DAM and mysterygasDAM.YEAR are the same except
output_gas has predicted data in spots where there was no observed
data.

B<gas_update_gas.exe> does its predictions very similarly to what
temp_update_temps.exe does.   Leading missing or invalid data points
are filled in using the first valid data point, holes between valid
data points are linearly interpolated between the two surrounding data
points, missing data after the last valid data point are filled in
with historical mean values.

Basically gas_update_gas.exe takes outputgasDAM.YEAR and predicts
a value for any any missing datum in the file and writes that to
outputgas_DAM.

=head1 INPUT FILES

      TEMPORARY-DIR/spillDWR.YEAR (ingres data file)
      data/gas/gasavg/gasavg_DAM (existing)
      TEMPORARY-DIR/outputgasDAM.YEAR (ingres data file)

=head1 OUTPUT FILES

      TEMPORARY-DIR/mysterygasDAM.YEAR 
      TEMPORARY-DIR/outputgas_DAM, 

=cut


*/

/*
	update_gas.c:  Because there could be bogus data points in DART,
	some data points in /home/realtime/gas/crisp_data/test/gasdam.yr 
	may be changed from the original data.  
	The point of this program is to screen out any bogus data coming in
	ingres and update the most current,valid data from Ingres. 
*/


#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "nr.h"
#include "nrutil.h"
#include "julian.h"
 
#define USAGE	"Usage: gas_update_gas.c CURRENT-YEAR TEMPORARY-DIR GAS-AVG-DIR STATION [ STATION [...]]\n"
#define PROGNAME "gas_update_gas.exe"

#define MAXLINE 500

void abort_run()
/* This function aborts program run */
{
        extern void exit();
 
        fprintf(stderr, "%s: Aborting Run... \n", PROGNAME);
        exit(1);
}
 
/* Returns error if couldn't open file: filename */
void check_file(char *filename, FILE *fptr)
{
	if(fptr==0) 
	{
   	printf("Can't open %s \n",filename);
      abort_run();
	}
   else
	{
   	printf("Opened file %s \n", filename);
   }
}

/*
	Returns 1 if gas is a valid value for %tdg;
   otherwise returns 0
   This procedure rules out absurd values, however does allow
   for very extreme values of gas not observed in
   recent history on mainstem columbia and snake rivers.
*/
int valid_gas(float gas)
{
	if( (gas<0) || (gas>=50.0) )
   	return(0);
   else
   	return(1);
}
 


/* reads a file full of names and reads the names into the character array
	list.	 This function returns the length of the list n.
*/
int read_station_list(char **array, int num, char list[100][MAXLINE])
{
	int	n;
	
	/*
		I don't know why the original author has it starting
		at 1 instead of 0, but I'll stay with it so I don't
		have to modify any more code.	  So we'll waste a word...
	*/
	for(n = 1 ; n <= num && n < 100 ; n++)
	{
		strcpy(&list[n][0], array[n-1]); 
	}

	if(n == 100)
	{
		fprintf(stderr, "Max number of stations exceeded\n");
	}

	return num;
}

void get_dwr_sp(float vec[367], char *input_file)
{
   char	foo[MAXLINE],
			newline[MAXLINE];
   FILE *fptr;
   float spill;
   int nvar,julian;

   fptr = fopen(input_file,"r");
    
   check_file(input_file,fptr);

   fgets(foo,MAXLINE,fptr);
   fgets(foo,MAXLINE,fptr);
   fgets(foo,MAXLINE,fptr);
	
   while( (fgets(newline,MAXLINE,fptr )) !=NULL ) 
	{
		nvar = sscanf(newline,"%d %f",&julian,&spill);
		if (nvar==2) 
		{
			vec[julian] = spill;		
			if(julian == 1) 
			{
				vec[0] = spill;
			}
		}
	}

	fclose(fptr);
}

static struct year_data
{
    char * month;
    unsigned char num_days;
} year_data[] =
{
    {"January", 31},
    {"February",28},
    {"March", 31},
    {"April", 30},
    {"May", 31},
    {"June", 30},
    {"July", 31},
    {"August", 31},
    {"September", 30},
    {"October", 31},
    {"November", 30},
    {"December", 31},
    {NULL, 0}
};
 

int main(int argc, char **argv)
{
	char	gas_file[100],
			avg_file[100],
			gas_data[100],
			gasstn[100][MAXLINE],
			foo[MAXLINE],
			input_file[MAXLINE],
			newline[MAXLINE],
			*temp_dir;
	FILE	*fptr,
			*gnew_fptr,
			*gas_fptr;
	float	dwr_sp[367],
			gas,
			dif,
			gvec[367],		/* gvec == Gas VECtor? */
			cr_gvec[367*2],	/* cr_gvec == CRisp Gas VECtor? */
			gas_avg[13];
	int	nvar,
			days,
			step,
			YEAR,
			mo,
			month,
			i,
			n,
			num_gas,
			day,
			da,
			d_aft,
			d_bef,
			julian,
			firstday,
			lastday,
			nval_gas,
			dwr_cmp;

	if(argc < 5)
	{
		fprintf(stderr, "%s", USAGE);
		return 1;
	}

	errno = 0;
	/* YEAR = gogas.pl::$dates{year} */
	YEAR = strtol(argv[1], (char**)NULL, 10);
	if(YEAR == 0 || errno != 0)
	{
		fprintf(stderr, "Error converting year to an integer: %s\n", argv[1]);
		return 1;
	}
	if(is_leapyr(YEAR))
		year_data[1].num_days++;

	/* temp_dir = gogas.pl::$temp_dir */
	temp_dir = strdup(argv[2]);

	num_gas = read_station_list(argv + 4, argc - 4, gasstn);	

	for(i=1; i<=num_gas; i++)
	{

		/* dwr_cmp == 0 if gasstn[i]==DWR */
		dwr_cmp = strncmp("DWR",gasstn[i],3);

		/* Input datafile from Ingres 
		 * gas_data = $REALDIR/tmp/outputgasDAM.YYYY
		 * Header: three lines.
		 * Two columns: mm/dd, decimal. */
		sprintf(gas_data, "%s/outputgas%s.%d", temp_dir, gasstn[i],YEAR);
	
		/* Output datafile, screened and formatted..  (I don't know what
		 * this file is used for, in fact I doubt it is used by anything.)
		 * gas_file = $REALDIR/tmp/mysterygasDAM.YYY 
		 * Header: zero lines.
		 * Two columns: julian date, decimal. */
		sprintf(gas_file, "%s/mysterygas%s.%d", temp_dir, gasstn[i],YEAR);
	
		/* Monthly Average datafile taken from COE Dissolved Gas reports to 
		 * be used when there is missing data.  The monthly averages from
		 * 1995 were used as a representative year for 1998.
		 * avg_file = gogas.pl::$REAL_DIR/data/gas/gasavg/gasavg_DAM
		 * Header: zero lines.
		 * One column: decimal. */
		sprintf(avg_file,"%s/gasavg_%s", argv[3], gasstn[i]);
	
		/* Initialize cr_gvec[] with monthly average data. */
		fptr = fopen(avg_file,"r");
		check_file(avg_file,fptr);
		days=1;
		for(mo=1; mo<=12; mo++)
		{
			fscanf(fptr,"%f",&gas);
			gas_avg[mo] = gas;
	
			if (days==1)
			{
				for(step=0; step<2; step++)  
					cr_gvec[step] = gas;
			}
	
			for(step=2*days; step < 2*(days+year_data[mo-1].num_days);step++)
				cr_gvec[step] = gas;
			days += year_data[mo-1].num_days;
		}
		fclose(fptr);
	
		for(day=0; day<=366; day++)
		{
			gvec[day] = -9.0;
			dwr_sp[day] = -9.0;
		}

		/* Open New gas data */
		lastday = 0;
		firstday=0;
		nval_gas=0;
		gnew_fptr = fopen(gas_data, "r");
		check_file(gas_data,gnew_fptr);
	   fgets(foo,MAXLINE,gnew_fptr);
	   fgets(foo,MAXLINE,gnew_fptr);
	   fgets(foo,MAXLINE,gnew_fptr);
		while( (fgets(newline,MAXLINE,gnew_fptr )) !=NULL ) 
		{
			nvar = sscanf(newline,"%d/%d %f",&month,&day,&gas);
			if (nvar<3)
				gas = -9.0;
			julian = get_julian(month, day, YEAR);
			if(firstday==0)
			    firstday = julian;
			lastday = julian;
	
			if(gas>0) 
				gvec[julian] = gas-100.0;
	
			if(valid_gas(gvec[julian])==1)
				nval_gas++;
		}
		fclose(gnew_fptr);
	
		/* Data filling in and screening for absurd river gas values in gvec[].
		 * At the completion of this for() loop, gvec[] will contain "valid"
		 * values for days 1 to lastday. */
	   if( (nval_gas>0) && (firstday!=lastday))
		{
			/* If firstday is not valid, find the first valid day X,
			 * and set all days from firstday to day X-1 to the value
			 * of day X. */
		   if(valid_gas(gvec[firstday])==0) 
			{
				d_aft = firstday + 1;
				while((valid_gas(gvec[d_aft])==0) && (d_aft<lastday) )
					d_aft++;
				for(day = firstday; day< d_aft; day++)
					gvec[day] = gvec[d_aft];
		   }
	
			/* If last day of data is bogus, then find the closest day X
			 * previous to lastday that is valid, and set day X+1 to
			 * lastday to the value of day X. */
		   if(valid_gas(gvec[lastday])==0 ) 
			{
				d_bef = lastday - 1;
				while((valid_gas(gvec[d_bef])==0) && (d_bef>firstday) )
					d_bef--;
				for(day = lastday; day>d_bef; day--)
					gvec[day] = gvec[d_bef];
		   }
	
		   da = firstday+1;
			/* Now for any range of days between firstday and lastday that
			 * do not have valid values, interpolate new values for all
			 * of those ranges. */
		   while (da<lastday)
			{
				if(valid_gas(gvec[da])==0) 
				{
					d_bef = da-1;
					d_aft = da+1;
				  	while((valid_gas(gvec[d_aft])==0) &&(d_aft<lastday) ) 
						d_aft++;
					dif = gvec[d_aft] - gvec[d_bef];
					n = d_aft - d_bef;
					for(day= da; day<=d_aft; day++)
						gvec[day] = gvec[day-1] + dif/n;
					da= d_aft;
				}
				da++;	
			}
	   }
	
	   /* no valid data scanned in to gvec[], this ensures bad data 
		 * flagged with -9 and not other invalid scanned in values */
	   if(valid_gas(gvec[firstday])==0 )
		{
			for(da=firstday; da<=lastday; da++)
				gvec[da] = -9.0;
	   }
	
		/* 
			USE DWR station data ONLY if spill is zero, because
		   DWR station is in the tailwater 
		*/
		if(dwr_cmp==0)
		{
			sprintf(input_file,"%s/spillDWR.%i", temp_dir, YEAR);
			get_dwr_sp(dwr_sp, input_file);
		}
	
		/* Create new gas output data file 
		 * Fill cr_gvec[] in with valid values in gvec[]. 
		 * Assuming that the input dates to gvec[] is leap-year smart,
		 * this loop will fill cr_gvec[] with off-by-one days from gvec[]. */
	   gas_fptr = fopen(gas_file,"w");
		check_file(gas_file,gas_fptr);
	   for(day=0; day<=lastday; day++)
		{
			fprintf(gas_fptr,"%d 	%8.2f \n", day,gvec[day]);
		
		   /* Set day 0 timeperiods== day1 */
			if(dwr_cmp ==0)
			{
		   	if(day==1)
				{
			   	for(step=0; step<2; step++)
					{
						if(gvec[day]>0 && dwr_sp[day]==0) 
							cr_gvec[step] = gvec[day];
					}
				}
			}
			else if(day==1)
			{
				for(step=0; step<2; step++)
				{
			   	if(gvec[day]>0) 
						cr_gvec[step] = gvec[day];
				}
			}
	
		   /* update crisp input with year to date info */
			if(dwr_cmp ==0)
			{
				for(step=0; step<2; step++)
				{
			   	if(gvec[day]>0 && dwr_sp[day]==0) 
						cr_gvec[2*day+step] = gvec[day];
				}
			}
			else 
			{
				for(step=0; step<2; step++)
				{
					if(gvec[day]>0) 
						cr_gvec[2*day+step] = gvec[day];
				}
			}
	   }
		   
	   fclose(gas_fptr);
	
		/* Creates crisp inputgas_*** datafile */
		/* This filename was changed to outputgas in the CRiSP datafile*/
	
	   sprintf(gas_file,"%s/outputgas_%s", temp_dir, gasstn[i]);
	   gas_fptr = fopen(gas_file,"w");
	     check_file(gas_file,gas_fptr);
		/* days = is_leapyr(YEAR) ? 367*2 : 366*2; */
		days = 366*2;
	     for(step=0; step<days; step++)
	     	fprintf(gas_fptr,"[%d:%d]    %8.2f \n", step,step,cr_gvec[step]);
	   fclose(gas_fptr);
	}

	return 0;
}
	
