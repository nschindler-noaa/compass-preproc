/* 
 * Modified May 2006 by WNB, CBR 
 * Shaw model replaced by the Paulsen model which has more parameters.
 * these are hardcoded in the dam structure below.
 * POD documentation not used and removed.
 * inc_temps not used and both the function and the calls have been removed.
 * Backcasting is removed.
 * Forecasting at tail end of season removed.
 *
 * modified Mar 2010 by NES, NOAA
 * updated to use Qt functions
 */
//#include <iomanip>
//#include <math.h>
#include <iostream>
using namespace std;
//#include <string>
//#include <stdio.h>
//#include "nrutil.h"

/*#ifdef WIN32
int gethostname (char *buffer, size_t& bufsize)
{
    _dupenv_s(&buffer, &bufsize, "COMPUTERNAME");
	if (buffer == NULL)
		return -1;
	return 0;
}
#else*/
//#include <pwd.h>
//#endif

//#include <time.h>
//#include <sys/types.h>
//#include <errno.h>
//#include <stdlib.h>

#include <QtGlobal>
#include <QIODevice>
#include <QFile>
#include <QDir>
#include <QString>
#include <QTime>

//using namespace std;

const char *version = "Version 1.1";
const char *usage =
        "Usage: temp_paulsen_r.exe temp_directory mean_input_directory year flowpred_file\n"
        "           temp_directory       - directory for temperature forecast files \n"
        "           mean_input directory - directory of historic mean temp values \n"
        "           year                 - specific year for forecast \n"
        "           flowpred_file        - predicted flow for specific year \n";
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

#ifndef PATHSEP
#ifdef WINVER
#define PATHSEP "\\"
#else
#define PATHSEP "/"
#endif
#endif

/* Here is the new dam structure based on Charlie Paulsen Temp write-up 05-11-06.doc */
struct dam {	
    const char *name;
	double A0_intercept;
	double B1_flow;
	double B2_flow_square;
	double B3_flow_day;
	double B4_flow_day_average;
	double B5_temp_day_average;
};

/* the MCN values are substituted for CHJ, PRD and other dams on the Columbia etc. for testing purposes 
 *  changed CHJ, PRD, RIS, RRH, WAN, and WEL B2 flow_sqr to positive NES 2010-03-19 */
struct dam dams[] = {	
/*   name    A0 int    B1 flow   B2 flow_sqr  B3 day_fl  B4 av_fl  B5 av_tmp */
	{"CHJ",  4.12965, -0.05571,  0.00016933,  -0.00428,  0.00462,  0.99836},  
    {"IHR",  2.34844, -0.08042,  0.0006054,   -0.00601,  0.00591,  0.99932}, 
    {"LMN",  0.78727, -0.0264,   0.0001684,   -0.00688,  0.00711,  1.00279}, 
    {"LGS",  1.24208, -0.02648,  0.0001238,   -0.01015,  0.00864,  0.98725}, 
    {"LWG",  1.69431, -0.06328,  0.0005415,   -0.01118,  0.01101,  0.99806}, 
    {"BON",  2.88159, -0.02864,  0.0000658,   -0.00181,  0.00197,  1.00149}, 
    {"TDA",  2.91641, -0.02605,  0.0000496,   -0.00173,  0.00188,  1.00103}, 
    {"JDA",  5.28766, -0.04972,  0.0001107,   -0.00312,  0.00285,  0.99371}, 
    {"MCN",  4.64304, -0.04157,  0.00008808,  -0.004,    0.00379,  0.99203}, 
    {"PRD",  4.12965, -0.05571,  0.00016933,  -0.00428,  0.00462,  0.99836},  
    {"WAN",  4.12965, -0.05571,  0.00016933,  -0.00428,  0.00462,  0.99836},  
    {"RRH",  4.12965, -0.05571,  0.00016933,  -0.00428,  0.00462,  0.99836},  
    {"RIS",  4.12965, -0.05571,  0.00016933,  -0.00428,  0.00462,  0.99836},  
    {"WEL",  4.12965, -0.05571,  0.00016933,  -0.00428,  0.00462,  0.99836},  
	{"ANA",  1.36669, -0.07423,  0.00087618,  -0.01350,  0.01361,  0.99922},   // USGS 13334300
	{"PEC",  1.61592, -0.26790,  0.01034,     -0.07476,  0.07459,  0.99901}    // USGS 13341050
};

enum FileType {FLOW_FC, TEMP_FC, FLOW_OB, TEMP_OB, FLOW_AV, TEMP_AV };

/* This function aborts program run */
void abort_run()
{
	cerr << PROGNAME << ":  Aborting run... " << endl;
	exit(1);
}

/* Returns error if couldn't open file: filename */
void check_file (QString filename, QFile *fptr)
{
	if (!fptr->exists())
	{
        cout << "File " << filename.toUtf8().data() << " does not exist" << endl;
		abort_run();
	}
	else if (!fptr->isOpen() && !fptr->open(QIODevice::ReadOnly)) 
	{
		fptr->close();
        cout << "Can't open " << filename.toUtf8().data() << endl;
		abort_run();
	}
	else if (fptr->atEnd()) 
	{
        cout << "File " << filename.toUtf8().data() << " is empty" << endl;
		abort_run();
	}
	else 
	{
        cout << "Opened file " << filename.toUtf8().data() << endl;
	}
}

/* This function makes sure j, the number of data points in 
 * filename, is 365
*/
void check_365 (QString filename, int j)
{
	if (j < 365) 
	{
		cout << "Not enough data. Need 365, but only " ;
        cout << j << " lines in " << filename.toUtf8().data() << "." << endl;
		abort_run();
	}
}
/* Prints a short intro and file name or not to outfile */ 
int output_file_name (int type, QString fname, QFile *outf)
{
	QString comment ("# ");
	switch (type)
	{
	case FLOW_FC: // Flow forecast
		if (fname.isEmpty())
			comment.append ("No Flow Forecast file");
		else
			comment.append ("Flow Forecast ");
		break;
	case TEMP_FC: // Temp forecast
		if (fname.isEmpty())
			comment.append ("No Temp Forecast file");
		else
			comment.append ("Temp Forecast ");
		break;
	case FLOW_OB: // Flow observation
		if (fname.isEmpty())
			comment.append ("No Flow Observation file");
		else
			comment.append ("Flow Obsv ");
		break;
	case TEMP_OB: // Temp observation
		if (fname.isEmpty())
			comment.append ("No Temp Observation file");
		else
			comment.append ("Temp Obsv ");
		break;
	case FLOW_AV: // Flow mean
		if (fname.isEmpty())
			comment.append ("No Flow Mean file");
		else
			comment.append ("Flow Mean ");
		break;
	case TEMP_AV: // Temp mean
		if (fname.isEmpty())
			comment.append ("No Temp Mean file");
		else
			comment.append ("Temp Mean ");
	}
	comment.append (fname);
	comment.append (" \n");
	if (!outf->isOpen())
		outf->open(QIODevice::WriteOnly);
    outf->write (comment.toUtf8());

	return comment.size();
}

/* prints a couple of lines of header information to ofp */
int print_header (QFile *ofp)
{
	QString header;
    QByteArray username, fullname, hostname;
/*#ifdef WIN32
	char *idValue;
	size_t len;
	char *hostname_buffer;

	_dupenv_s(&idValue, &len, "USER");
	if (idValue == NULL)
		_dupenv_s (&idValue, &len, "USERNAME");
	username = QString (idValue);
	free (idValue);

	_dupenv_s(&hostname_buffer, &len, "COMPUTERNAME");
#else*/
//	char hostname_buffer[256];
//	extern int gethostname ();
//	extern uid_t getuid ();
//	struct passwd *pwd;
//	pwd = getpwuid (getuid ());
//	username = QString (pwd->pw_name);
//	fullname = QString (pwd->gecos);
//	gethostname (hostname_buffer, sizeof(hostname_buffer));
//#endif\
    username = qgetenv("user");
    if (username.isEmpty())
        username = qgetenv("username");
    fullname = qgetenv("username");
    hostname = qgetenv("hostname");
//	hostname = QString (hostname_buffer);

	// File name
	// program name
    header.append (QString("#\n"));
    header.append (QString("# Forecast Temperatures produced by %1 \n").arg(QString (PROGNAME)));
	// time
    header.append (QString("# %1 %2 \n").arg(QDate::currentDate().toString(), QTime::currentTime().toString()));
    // user and host info, if exists
	if (!username.isEmpty())
	{
        header.append (QString("# Written by user %1 ").arg(QString(username)));
		if (!fullname.isEmpty())
		{
            header.append (QString(" (%1) \n").arg(QString(fullname)));
		}
		else
			header.append ("\n");
		if (!hostname.isEmpty())
		{
            header.append (QString("#         on host %1 \n").arg(QString(hostname)));
		}
	}
	else if (!hostname.isEmpty())
	{
        header.append (QString("# Written on host %1 \n").arg(QString(hostname)));
	}
	//header.append ("# \n");
	
    if (!ofp->isOpen())
        ofp->open (QIODevice::WriteOnly);
    ofp->write (header.toUtf8());

	return header.size();
}

/* Reads in mean temperature data and year to date temperature.
 * temp_pred is set to be the mean when no current data is available.
 * If the last day is julian day 70 or later, predict up to 100 days of
 * additional data using meantemp.
 * Return  the last day of year to date temperature. 
*/
int init_temps (float *meantemp, float *temp_pred, QString temp_file, QString mean_file)
{
	int julian = 0, lastday;
	float temp;
	/* added parameters for backCasting  DS */
	int firstdata;  /* will be first jdate of valid data  */
	QFile temp_fptr(temp_file);
	QFile mean_fptr(mean_file);
	
	/* Read in mean data */	
	mean_fptr.open(QIODevice::ReadOnly);
	check_file (mean_file, &mean_fptr);
	mean_fptr.readLine (MAXLINE);
	while (!mean_fptr.atEnd()) 
	{
		QString linestr (mean_fptr.readLine (MAXLINE));
		QStringList tokens (linestr.split (' ', QString::SkipEmptyParts));
		julian = tokens[0].toInt();
		meantemp[julian]  = tokens[1].toFloat();
		temp_pred[julian] = meantemp[julian];
	}
	check_365 (mean_file, julian);
	mean_fptr.close();

	temp_fptr.open (QIODevice::ReadOnly);
	
	lastday = 0;
	while (!temp_fptr.atEnd())  
	{
		QString linestr (temp_fptr.readLine (MAXLINE));
		QStringList tokens (linestr.split (' ', QString::SkipEmptyParts));
		julian = tokens[0].toInt();
		temp   = tokens[1].toFloat();
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
	temp_fptr.close();

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
void init_flows (float *meanflow, float *flow_pred, QString flowpred_file, 
                 QFile *fl_file, QString mean_file)
{
	char  line[MAXLINE];
	int   julian,
		  jj;
//		  skip;
	QFile flow_fptr,
		  mean_fptr;
	float flow;

	extern int readFlowPredFile (QFile * flowpredfile, QString damAbbv, float *flow_predict);
	QString damAbbv (mean_file.right (3));

	/* Read in mean data */ 
	mean_fptr.setFileName (mean_file);
	mean_fptr.open(QIODevice::ReadOnly);
	check_file (mean_file, &mean_fptr);
	mean_fptr.readLine (line, MAXLINE);
	while (!mean_fptr.atEnd() )
	{
		QString linestr (mean_fptr.readLine (MAXLINE));
		QStringList tokens (linestr.split (' ', QString::SkipEmptyParts));
		julian = tokens[0].toInt();
		meanflow[julian] = tokens[1].toFloat();
	}
	check_365 (mean_file, julian);
	mean_fptr.close();

	flow_fptr.setFileName (flowpred_file);
	flow_fptr.open (QIODevice::ReadOnly);
	check_file (flowpred_file, &flow_fptr);
	/* Read in flow prediction file */
	julian = readFlowPredFile (&flow_fptr, damAbbv, flow_pred);
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
	flow_fptr.close();
 
	/* Read in year to date flow data */
    if (fl_file->open (QIODevice::ReadOnly))
    {
        fl_file->readLine (line, MAXLINE);
        while (!fl_file->atEnd())
        {
            QString linestr (fl_file->readLine (MAXLINE));
            QStringList tokens (linestr.split (' '));
            julian = tokens[0].toInt();
            flow = tokens[1].toFloat();
            if (flow > 0.0)
            {
                flow_pred[julian] = flow;
            }
        }
        fl_file->close();
    }
}

/* This makes a temperature prediction based on a function of flow.*/	
void predict1_temp(float *flow_pred, float *meanflow, float *meantemp, 
				   float *temp_pred, struct dam *thisdam, int START_PRED)
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
	for(j = START_PRED ; j <= 365 ; j++)
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
	QString temp_file, flow_file, mean_flow_file, mean_temp_file;
    QString yearstr, outfile;
	QString mean_dir, flowpred_file, 
			out_dir, /* added to control local output WNB*/
			temp_dir;
	QFile   flowpred_fh,
			optfile;
	float	*meantemp,
			*meanflow,
			*flow_pred,
			*temp_pred;
	int		START_PRED,
			ph_out;
    int	errno, i, j, last_day, year;
	struct dam *thisdam;
//	fpos_t	pos;

	if (argc != 5)
	{
        fprintf (stderr, "%s", usage);
		return 1;
	}
	temp_dir = QString (argv[1]);
	mean_dir = QString (argv[2]);
	flowpred_file = QString (argv[4]);
	/* temporary to control local output - WNB */
	out_dir = QString ("temp_r.out.dir");   
	
	errno = 0;
    yearstr = QString (argv[3]);
    year = yearstr.toInt();
	if (year == 0 || errno != 0)
	{
		fprintf (stderr, "Error converting year to an integer: %s\n", argv[3]);
		return 1;
	}

	meantemp  = (float *) calloc (365, sizeof (float));
	meanflow  = (float *) calloc (365, sizeof (float));
	flow_pred = (float *) calloc (365, sizeof (float));
	temp_pred = (float *) calloc (365, sizeof (float));

	flowpred_fh.setFileName (flowpred_file);
	check_file (flowpred_file, &flowpred_fh);
/*	if (flowpred_fh.atEnd()) 
	{
		flowpred_fh.close();
		fprintf (stderr, "Error getting position in %s", flowpred_file);
		perror (NULL);
		exit (1);
	}*/

	/* Loop through each dam */
	for (i = 0; i < NDAMS; i++)
	{
		thisdam = &dams[i];

		/* Output header */
		/* The old one that SLI was using. change it be a local one for Nick
		* sprintf(out_file,"%s/rTEMP_%s", temp_dir, thisdam->name); 
		* UNIX side control
		* 
		* WNB temporary to control local output 
		* output file name
		* {temp dir}/tempFC.{dam name}.19xx
		*/
        outfile = QString(QString("%1%2").arg(temp_dir, PATHSEP));
        outfile.append (QString("tempFC.%1.%2").arg(thisdam->name, yearstr));
//        outfile.append ();(QString("%1%2tempFC.%3.%4").arg(temp_dir, PATHSEP, QString(thisdam->name), QString::number(year)));
#ifdef WINVER
        outfile.replace ('/', '\\');
#endif
/*        outfile.append (QString ("tempFC."));
        outfile.append (QString (thisdam->name));
		outfile.append (QString ("."));
        outfile.append (QString::number (year));*/
		optfile.setFileName (outfile);
		optfile.open (QIODevice::WriteOnly);
		ph_out = print_header (&optfile);
		if (ph_out == 0)
		{
			printf ("Write error occured in output file \n");
			abort_run ();
		}

		/* data file names*/
        flow_file = QString(QString("%1%2").arg(temp_dir, PATHSEP));
        flow_file.append (QString("flow.%1.%2").arg(thisdam->name, yearstr));
        temp_file = QString(QString("%1%2").arg(temp_dir, PATHSEP));
        temp_file.append (QString("temp.%1.%2").arg(thisdam->name, yearstr));
        mean_flow_file = QString(QString("%1%2").arg(mean_dir, PATHSEP));
        mean_flow_file.append (QString("meanflow.%1").arg(thisdam->name));
        mean_temp_file = QString(QString("%1%2").arg(mean_dir, PATHSEP));
        mean_temp_file.append (QString("meantemp.%1").arg(thisdam->name));
#ifdef WINVER
        flow_file.replace ('/', '\\');
        temp_file.replace ('/', '\\');
        mean_flow_file.replace ('/', '\\');
        mean_temp_file.replace ('/', '\\');
#endif

//        flow_file = temp_dir + PATHSEP + QString ("flow") + QString (thisdam->name) + QString (".") + QString::number (year);
//        temp_file = temp_dir + PATHSEP + QString ("temp") + QString (thisdam->name) + QString (".") + QString::number (year);
//        mean_temp_file = mean_dir + PATHSEP + QString ("meantemp.") + QString (thisdam->name);
//        mean_flow_file = mean_dir + PATHSEP + QString ("meanflow.") + QString (thisdam->name);
		 
		/* Read in historical mean temperatures into meantemp, 
		 * and observed data into temp_pred.  if last_day > 70 then 
		 * predict up to 100 days of data and write that into temp_pred 
		 * too. Holes in observed data in temp_pred are filled in with 
		 * values from meantemp. 
		*/
		QFile temp_fh (temp_file);
		if (temp_fh.exists())
			output_file_name (TEMP_OB, temp_fh.fileName(), &optfile);
		else
		{
			optfile.write ("# No Temp Obs for ");
			optfile.write (thisdam->name);
			optfile.write (" ");
			optfile.write (argv[3]);
			optfile.write ("\n");
		}
		QFile mean_temp_fh (mean_temp_file);
		output_file_name (TEMP_AV, mean_temp_fh.fileName(), &optfile);
		// do the work
		last_day = 1 + init_temps (meantemp, temp_pred, temp_file, 
			mean_temp_file);

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
		// print header file info
		QFile flow_fh (flow_file);
		if (flow_fh.exists())
			output_file_name (FLOW_OB, flow_fh.fileName(), &optfile);
		else
		{
			optfile.write ("# No Flow Obs for ");
			optfile.write (thisdam->name);
			optfile.write (" ");
			optfile.write (argv[3]);
			optfile.write ("\n");
		}
		QFile mean_flow_fh (mean_flow_file);
		output_file_name (FLOW_AV, mean_flow_fh.fileName(), &optfile);
		// do the work
		init_flows (meanflow, flow_pred, flowpred_fh.fileName(), &flow_fh, 
			mean_flow_file);
		
		/* Use flow prediction data, meanflow, and meantemp to predict
		 * temperature data for days in the range [ START_PRED, 272 ].
		 * unless modified above. 
		*/
		predict1_temp (flow_pred, meanflow, meantemp, temp_pred, thisdam, 
			START_PRED);

		/* Output values */
		optfile.write("#\n");
		/* compassb expects Dec31 from previous year so Jan1 is repeated
		*/
        optfile.write (QString::number(temp_pred[1],'f',6).toUtf8().data());
		optfile.write (" \n");
		for (j = 1; j <= 365; j++) 
		{
            optfile.write (QString::number(temp_pred[j],'f',6).toUtf8().data());

			if (j % 4 == 0)
				optfile.write ("\n");
			else
				optfile.write (" ");
		}
		optfile.write ("\n");
		optfile.close(); 
	}
	return 0;
}
