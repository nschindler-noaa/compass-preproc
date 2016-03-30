/*
COMPASS file generation process

Generates a COMPASS .dat file that incorporates new flow.archive file, 
temperature data generated from flow data, and transport data generated 
from MAF record and operational rules. 

1. process modulated flow, spill, elevation data from HYDSIM/Paulsen SAS process
	1. parse HYDSIM modulated SAS file
	2. produce "flow.archive" structured file for COMPASS

2. generate temperature data based on HYDSIM data
	requires: 
	1. temp_paulsen_r.exe
	2. flow pred file
	3. historical mean files

3. generate COMPASS transport files that match operational rules, HYDSIM inputs (MAF), and HYDSIM data
	requires:
	1. MAF input file used by HYDSIM to generate forecast
	2. Rules.pm - operational rules for start/stop dates and low/med/high flow years
		rules can be more complicated, but in recent years this is it

# DAM->SCENARIO->PARAM = array (start:end:value)
Example 1 (3 tier tranportation)
$rules{'LWG'}{'2008biopfinal'}{'transport:start_date'} = "93:111:111"; # April 3, April 21
$rules{'LWG'}{'2008biopfinal'}{'transport:range_lo_maf'} = "12.8";
$rules{'LWG'}{'2008biopfinal'}{'transport:range_hi_maf'} = "14.6";
$rules{'LWG'}{'2008biopfinal'}{'transport:flow_dam'} = "LWG";

Example 2 (2 tier transportation)
$rules{'LWG'}{'2008ops'}{'transport:start_date'} = "111:122:122"; # April 20, May 1 (leap year)
$rules{'LWG'}{'2008ops'}{'transport:range_lo_maf'} = "13.01";
$rules{'LWG'}{'2008ops'}{'transport:range_hi_maf'} = "13.01";
$rules{'LWG'}{'2008ops'}{'transport:flow_dam'} = "LWG";

4. generate COMPASS DAT file that has all data and settings incorporated
	requires:
	1. multiple runs of COMPASS
	2. write specific version of *.ctrl file to run monte carlo "flow.archive" run
		requires:
		1. file: generated .riv file w/ temperature
		2. file: include.powerhouse (exists)
			sets COMPASS parameters artificially high to not modify HYDSIM values 
		3. file: debug_all.etc
		4. file: .compass-alts
		5. dir: datmaker/
		**note: during this run, COMPASS may issue warning messages about some values 
		and make slight modifications to the values written out to DAT file. The 
		following scenario run makes sure that all values are included and all 
		parameters are within acceptable range and will not produce messages when run.
	3. rerun COMPASS in scenario mode
		requires:
		1. DAT file written out by monte carlo "flow.archive" run
		2. file: base.etc

5. Generate 70 year scenario and release files for both scpecies. 


Usage======================================================================
compass_pp -alt=ALT_NAME -hydro=HYDSIM_NAME -scenario=SCN_NAME 
			[ -hydsim=HYDSIM_PRT [-d] ] [ -spill=DAY_SPILL ] [ -dam=DAM_PARAMS ]
			[ -mod=ALL|FLOW ] [ -trans=MAF|FLOW|TEMP ] [ -debug=TEMP|SPILL|ALL ]
			[ -data=DATA_DIR ] [ -out=OUTPUT_DIR ] [ -help ]
	
Required===================================================================
ALT_NAME	alternative used in the Rules.pm module to specify spill and
		    other operations
		    no spaces allowed in name
		      example: --alt=base2004-S1

HYDSIM_NAME	HYDROSIM model files, must reside in HydSim/HYDSIM_NAME 
		    directory. If HYDROSIM_PRT is defined, this is a name to track
		    the current HYDROSIM files.
		    no spaces allowed in name
		      example: --hydsim=base2004

SCN_NAME	unique name used to identify the scenario and to archive
		    intermediate and final files from the run
		    no spaces allowed in name
		      example: --scenario=base2004-S1

Optional===================================================================

HYDROSIM_PRT HYDROSIM output, not flow modulated. If this is not used,
            the program assumes that the flow modulated file(s) reside in
		    the HydSim/HYDSIM_NAME directory.
             
-d          Specifies that the HYDROSIM output is in daily values. 
            Otherwise, the default is in monthly values.

DAY_SPILL   HYDSIM daily spill fraction in each month. If this is not
            specified, the program uses a default file. Only used if
            HYDSIM_PRT is specified. No spaces allowed in name.
              example: -spill=PImotion_in_month_spill_2008_12.csv

DAM_PARAMS  Name of the dam parameters to use. This is the
            unique part of the files in the 'step3' directory.
            No spaces allowed in name.
              example: -dam=2004_Base

mod	        flag to indicate that HYDSIM flow, spill, elev in single file
		    with daily modulated flow, monthly spills, seasonal elev
		      --mod=ALL use all inputs
		      --mod=NOSPILL use only flow input

trans		flag to indicate what determines transport dates
			  --trans=MAF turns on option to set transport based on
		    reverse engineering of spill to get MAF forecast
			  --trans=FLOW turns on option to modify spill operations based on spring  
		    averaged flow levels and sets transport start dates
			  --trans=TEMP turns on option to modify spill operations based on spring
		    temperatures and sets transport start dates

debug       outputs additional parameters from COMPASS runs for examining
		    temperature and other modelled parameters
			  --debug=TEMP outputs temperature and flow values
			  --debug=SPILL outputs spill values
			  --debug=ALL outputs temperature, flow, spill values

DATA_DIR    Directory that holds necessary data files. If not given,
            default is "pre-proc". No spaces allowed in name and use
            forward slash as separator ('/').
              example: -data=pre-proc;

OUTPUT_DIR  Output directory. If not given, output files will be in
            default directory. No spaces allowed in name and use
            forward slash as separator ('/').
              example: -output=Final/MOD/debug1

help		prints this information
*/
#include "pp_fileIoUtils.h"
#include "compass_pp.h"
#include "rules.h"
#include <iostream>
#include <QString>
#include <QDir>
#include <QFile>
#include <QDate>
#include <QTime>

using namespace std;

//DIRS *dirs;
#define PATHSEP  QString("/")

extern QHash<QString, QString> locationList;
extern QHash<QString, QString> flowMap;
extern QHash<QString, QString> poolMap;
extern QHash<QString, QString> months;
extern QHash<QString, QString> monthPeriods;
//QStringList flowMap;
//QStringList poolMap;

extern QStringList ops_params;
extern QStringList rules_params;
extern QStringList loc_list;
extern QStringList dams;
extern QStringList river_list;
extern QStringList hist_list;
extern QStringList spill_list;
extern QStringList trans_spring_list;
extern QStringList trans_list;
extern QStringList usgs_list;
extern QStringList control_dam_list;
extern QStringList control_month_list;
extern QStringList control_spill_month_list;

extern QStringList locations;
extern QStringList alternatives;
extern QStringList riv_order;

extern RULES water_rules[N_DAM][N_ALT];

QStringList dam_names;
QStringList output_dams;

void help_version ()
{
	cout << "COMPASS Pre-processor version 1.1" << endl << endl;
}
void help_usage ()
{
//Usage======================================================================
QString usage ("compass_pp -alt=ALT_NAME -hydro=HYDSIM_NAME -scenario=SCN_NAME \n"); 
usage.append  ("    [ -hydsim=HYDSIM_PRT [-d] ] [ -spill=DAY_SPILL ] [ -dam=DAM_PARAMS ]\n");
usage.append  ("    [ -mod=ALL|FLOW ] [ -trans=MAF|FLOW|TEMP ] [ -debug=TEMP|SPILL|ALL ]\n");
usage.append  ("    [ -data=DATA_DIR ] [ -out=OUTPUT_DIR ] [ -help ]");
    cout << usage.toUtf8().data() << endl;
}
void help_verbose ()
{
//Required===================================================================
QString verbose ("=Required================\n");
verbose.append ("    ALT_NAME     Alternative used in the Rules.pm module to specify spill and\n");
verbose.append ("                 other operations. No spaces allowed in name.\n");
verbose.append ("                   example: -alt=base2004-S1\n\n");

verbose.append ("    HYDSIM_NAME  HYDSIM model files, must reside in HYDSIM/HYDSIM_NAME\n"); 
verbose.append ("                 directory. No spaces allowed in name.\n");
verbose.append ("                   example: -hydsim=base2004\n\n");

verbose.append ("    SCN_NAME     Unique name used to identify the scenario and to archive\n");
verbose.append ("                 intermediate and final files from the run.\n");
verbose.append ("                 No spaces allowed in name.\n");
verbose.append ("                   example: -scenario=base2004-S1\n\n");

//Optional===================================================================
verbose.append ("=Optional=================\n");
verbose.append ("    HYDSIM_PRT   HYDSIM output, by month or day. If this is not used, the\n");
verbose.append ("                 program assumes that the daily flow modulated file(s) reside\n");
verbose.append ("                 in the HydSim/HYDSIM_NAME directory.\n");
verbose.append ("                 No spaces allowed in name.\n");
verbose.append ("                    example: -hydro=PImotionSpill&Res+plus.PRT\n\n");
verbose.append ("    -d           Signifies the HYDSIM output is in daily values. \n\n");

verbose.append ("    DAY_SPILL    HYDSIM daily spill fraction in each month. If this is not \n");
verbose.append ("                 specified, the program uses a default file. Only used if \n");
verbose.append ("                 HYDSIM_PRT is specified. No spaces allowed in name.\n");
verbose.append ("                    example: -spill=PImotion_in_month_spill_2008_12.csv\n\n");

verbose.append ("    DAM_PARAMS	  Name of the dam parameters to use. This is the \n");
verbose.append ("                 unique part of the files in the 'step3' directory.\n");
verbose.append ("                 No spaces allowed in name.\n");
verbose.append ("                    example: -dam=2004_Base\n\n");

verbose.append ("    mod          Flag to indicate that HYDSIM flow, spill, elev in single\n");
verbose.append ("                 file with daily modulated flow, monthly spills, seasonal\n");
verbose.append ("                 elevation.\n");
verbose.append ("                  -mod=ALL     use all inputs (default)\n");
verbose.append ("                  -mod=NOSPILL use only flow input\n\n");

verbose.append ("    trans        Flag to indicate what determines transport dates\n");
verbose.append ("                  -trans=MAF   turns on option to set transport based on\n");
verbose.append ("                        reverse engineering of spill to get MAF forecast.\n");
verbose.append ("                  -trans=FLOW  turns on option to modify spill operations\n"); 
verbose.append ("                        based on spring averaged flow levels and sets \n");
verbose.append ("                        transport start dates.\n");
verbose.append ("                  -trans=TEMP  turns on option to modify spill operations\n");
verbose.append ("                        based on spring temperatures and sets transport\n");
verbose.append ("                        start dates.\n\n");

verbose.append ("    debug        Outputs additional parameters from COMPASS runs for\n");
verbose.append ("                 examining temperature and other modelled parameters.\n");
verbose.append ("                  -debug=TEMP  outputs temperature and flow values.\n");
verbose.append ("                  -debug=SPILL outputs spill values.\n\n");
verbose.append ("                  -debug=ALL   outputs temperature, flow, spill values.\n");

verbose.append ("    DATA_DIR     Directory that holds necessary data files. If not given,\n");
verbose.append ("                 default is "pre-proc". No spaces allowed in name and use\n");
verbose.append ("                 forward slash as separator ('/').\n");
verbose.append ("                    example: -data=pre-proc\n\n");

verbose.append ("    OUTPUT_DIR   Output directory. If not given, output files will be in\n");
verbose.append ("                 default directory. No spaces allowed in name and use\n");
verbose.append ("                 forward slash as separator ('/').\n");
verbose.append ("                    example: -output=Final/MOD/debug1\n\n");

verbose.append ("    help         Prints this information (-help).\n\n");

    std::cout << verbose.toUtf8().data() << std::endl;
}
void help ()
{
	help_version ();
	help_usage ();
	help_verbose ();
	exit(1);
}
void help (char *string)
{
	cout << "ERROR: " << string << endl;
	help_usage ();
	exit (1);
}
SETTINGS args;

int main (int argc, char *argv[])
{
	cmd_args = &args;
	int check;

	/* Get options from command line arguments */
	check = get_args (argc, argv);
	if (check == 0 || 
		cmd_args->help)
		help();
	if (cmd_args->altern.isEmpty())
		help ("No alternative file name provided.");
	if (cmd_args->hydro.isEmpty())
		help ("No HydSim file name provided.");
	if (cmd_args->scenario.isEmpty())
		help ("No scenario file name provided.");
	if (cmd_args->mod != NOSPILL && 
		cmd_args->mod != ALL) 
		help ("NOSPILL or ALL are the only recognized options for mod");
	if (cmd_args->trans != TEMP && 
		cmd_args->trans != FLOW && 
		cmd_args->trans != MAF)
		help ("TEMP, FLOW, or MAF are the only recognized options for trans");
	if (cmd_args->debug != TEMP && 
		cmd_args->debug != SPILL && 
		cmd_args->debug != ALL)
		help("TEMP, SPILL, or ALL are the only recognized options for debug");

	// setup directories
	setDirs();
	// setup project names
	setDamNames();
	setOutputDams();
	// setup check files
	QFile modified (dirs.final + PATHSEP + cmd_args->altern + QString (".modified.txt"));
	QFile meanflow (dirs.final + PATHSEP + cmd_args->altern + QString (".meanflow.txt"));
	QFile lwgflowavg (dirs.final + PATHSEP + cmd_args->altern + QString (".lwgflowavg.txt"));
	QFile lwgflowfcst (dirs.final + PATHSEP + cmd_args->altern + QString (".lwgflowavg.forecast.txt"));
	QFile lwgtemp  (dirs.final + PATHSEP + cmd_args->altern + QString (".lwgtemp.txt"));
	QFile transflow (dirs.final + PATHSEP + cmd_args->altern + QString (".transflow.txt"));


	for (int prj = ANA; prj < N_DAM; prj++)
		rh_data[prj].name = int_to_prj (prj);

	/* If starting with the HydSim output file, modulate flow and place output in correct dir. */
	if (!cmd_args->hydsim.isEmpty())
	{
        QString cmd_line;
        cmd_line = QString(dirs.bin + PATHSEP);
		cmd_line.append ("compass_fm -f \"");
		cmd_line.append (cmd_args->hydsim);//dirs.hydsim + PATHSEP + cmd_args->hydsim);
		if (!cmd_args->dayspill.isEmpty())
		{
			cmd_line.append ("\" -p \"");
			cmd_line.append (cmd_args->dayspill);
		}
		cmd_line.append ("\" -o \"");
		cmd_line.append (dirs.hydro + PATHSEP + cmd_args->hydro);
        cmd_line.append (".csv\" ");
        if (cmd_args->daily)
        {
            cmd_line.append("-d ");
            cout << " ... Reading flow ..." << endl;
        }
        else
        {
            cout << " ... Flow modulation processing ..." << endl;
        }
        cmd_line.append("\n");
#ifdef WIN32
        cmd_line.replace ("/", "\\");
#endif
        cout << cmd_line.toUtf8().data() << endl;
        system (cmd_line.toUtf8().data());
    }

	/* 1. process modulated flow, spill, elevation data from HYDSIM/Paulsen 
	 *    SAS process.
	 */
	processModulatedFile();

	/* Fill year-based array with historical mean 
	 * Historical mean files produced outside this process from DART 
	 */
	get_mean ("flow", flowmean);


	/* 2. generate temperature data based on HYDSIM data.
	 */
	generateTemps ();

	/* 3. generate COMPASS transport files that match operational rules, 
	 *    HYDSIM inputs (MAF), and HYDSIM data.
	 */
	generateTransports(&transflow, &lwgtemp);

	/* Miscellaneous calculations */
	/* calculate spring seasonal average flow for LWG and MCN based on
	 * dates and flow forecast data */
	lwgflowavg.open (QIODevice::WriteOnly);
	springAverage (LWG, "flowFC", 93, 171, &lwgflowavg);
	springAverage (MCN, "flowFC", 100, 181, &lwgflowavg);
	lwgflowavg.close();
	/* NOT HYDSIM/BONNEVILLE option
	 * convert spill to spill percent based on rules & HYDRO flows
	 * command line option --mod not specified or --mod=NOSPILL
	 * controls whether COMPASS is allowed to modify spill and elevation 
	 * levels depending on flow */
	if (cmd_args->mod == 0 || cmd_args->mod == NOSPILL)
		convertSpill (output_dams, &modified);

	/* 4. generate COMPASS DAT file that has all data and settings incorporated
	 */
	createCompassDat();


	if (cmd_args->mod == ALL && modified.exists())
		cout << "WARNING! COMPASS has modified spill, flow values from HYDSIM. Check settings and rerun." << endl;

	/* Fact-checking and auxillary files */
	/* Write averages */
	write_period_avg ("flowFC");
	write_period_avg_spill ("compass");
	write_period_avg_spill ("hydsim");

	// must run write_period_avg_spill ("compass") before calling this routine
	//write_period_avg_spill_trans();

	if (cmd_args->debug == SPILL)
	{
		parse_summaryalt_spill();
		write_period_avg ("altspill");
	}
	/* 5. Generate 70-year .scn and .rls files
	 */
	if (!cmd_args->damParam.isEmpty())
	{
		create_final_files();
	}
	return 1;
}
int get_args (int argc, char *argv[])
{
	int i;
	QString arg;
	int ret = 0;

	arg = QString ("");
    cmd_args->daily = false;
	cmd_args->help = false;
	cmd_args->mod = 0;
	cmd_args->trans = 0;
	cmd_args->debug = 0;
    cmd_args->dayspill = QString("");
    cmd_args->hydsim   = QString("");
    cmd_args->dataDir  = QString("");
    cmd_args->outputDir = QString("");

	for (i = 1; i < argc; i++)
	{
		arg = QString (argv[i]);
        if      (arg.startsWith ("-rules", Qt::CaseInsensitive))
		{
			ret = 1;
			cmd_args->rules = arg.section ('=', 1);
		}
        else if (arg.startsWith ("-alt", Qt::CaseInsensitive))
		{
			ret = 1;
			cmd_args->altern = arg.section ('=', 1);
		}
		else if (arg.startsWith ("-hydro", Qt::CaseInsensitive))
		{
			ret = 2;
			cmd_args->hydro = arg.section ('=', 1);
		}
		else if (arg.startsWith ("-spill", Qt::CaseInsensitive))
		{
			ret = 2;
			cmd_args->dayspill = arg.section ('=', 1);
		}
		else if (arg.startsWith ("-scenario", Qt::CaseInsensitive))
		{
			ret = 3;
			cmd_args->scenario = arg.section ('=', 1);
		}
		else if (arg.startsWith ("-hydsim", Qt::CaseInsensitive))
		{
			ret = 4;
			cmd_args->hydsim = arg.section ('=', 1);
		}
		else if (arg.startsWith ("-mod", Qt::CaseInsensitive))
		{
			ret = 5;
			cmd_args->mod = get_option (arg.section ('=', 1));
		}
		else if (arg.startsWith ("-trans", Qt::CaseInsensitive))
		{
			ret = 6;
			cmd_args->trans = get_option (arg.section ('=', 1));
		}
		else if (arg.startsWith ("-help", Qt::CaseInsensitive))
		{
			ret = 7;
			cmd_args->help = true;
		}
		else if (arg.startsWith ("-debug", Qt::CaseInsensitive))
		{
			ret = 8;
			cmd_args->debug = get_option (arg.section ('=', 1));
		}
		else if (arg.startsWith ("-dam", Qt::CaseInsensitive))
		{
			ret = 9;
			cmd_args->damParam = arg.section ('=', 1);
		}
        else if (arg.startsWith ("-data", Qt::CaseInsensitive))
		{
			ret = 10;
            cmd_args->dataDir = arg.section ('=', 1);
		}
        else if (arg.startsWith ("-out", Qt::CaseInsensitive))
        {
            ret = 10;
            cmd_args->outputDir = arg.section ('=', 1);
        }
        else if (arg.startsWith ("-d", Qt::CaseInsensitive))
        {
            ret = 4;
            cmd_args->daily = true;
        }
    }
	if (cmd_args->mod)
		cmd_args->altName = cmd_args->scenario;
	else
		cmd_args->altName = cmd_args->altern;
	
	return ret;
}
int get_option (QString opt)
{
	int ret;
	if      (opt.startsWith ("all", Qt::CaseInsensitive)) 
		ret = ALL;
	else if (opt.startsWith ("spill", Qt::CaseInsensitive))
		ret = SPILL;
	else if (opt.startsWith ("nospill", Qt::CaseInsensitive))
		ret = NOSPILL;
	else if (opt.startsWith ("flow", Qt::CaseInsensitive))
		ret = FLOW;
	else if (opt.startsWith ("temp", Qt::CaseInsensitive))
		ret = TEMP;
	else if (opt.startsWith ("maf", Qt::CaseInsensitive))
		ret = MAF;
	else
		ret = ERROR;

	return ret;
}
char * return_option (int n)
{
	char * option;
	QString opt;
	switch (n)
	{
	case ALL:
		opt = QString ("all");
		break;
	case SPILL:
		opt = QString ("spill");
		break;
	case NOSPILL:
		opt = QString ("nospill");
		break;
	case FLOW:
		opt = QString ("flow");
		break;
	case TEMP:
		opt = QString ("temp");
		break;
	case MAF:
		opt = QString ("maf");
		break;
	}
	option = (char *) malloc (opt.size());
    strcpy (option, opt.toUtf8().data());
	return option;
}
void setDirs()
{
    dirs.app       = QString(QDir::currentPath());
    dirs.data      = QString(dirs.app);
    if (!cmd_args->dataDir.isEmpty())
        dirs.data.append(cmd_args->dataDir);
    if (!cmd_args->outputDir.isEmpty())
        dirs.user = QString(cmd_args->outputDir);
    else
        dirs.user = QString(dirs.app);
//#ifdef WIN32
//#endif
//	dirs.user.append (PATHSEP);
//	dirs.user.append (QString ("/temp/compass_pp"));
    dirs.bin       = QString(dirs.app);// + QString ("/bin");
    dirs.mean      = QString("%1%2%3").arg(dirs.data, PATHSEP, QString("HistMean"));
	dirs.sub       = QString ((cmd_args->mod == 0)? "/": "/MOD/");
	dirs.final     = dirs.user + QString ("/Final") + dirs.sub + cmd_args->scenario;
	dirs.flfcast   = dirs.user + QString ("/Work") + dirs.sub + cmd_args->scenario  + QString ("/FLOWFCAST");
	dirs.flarchive = dirs.user + QString ("/Work") + dirs.sub + cmd_args->scenario  + QString ("/FLOWARCHIVE");
	dirs.tpfcast   = dirs.user + QString ("/Work") + dirs.sub + cmd_args->scenario  + QString ("/TEMPFCAST");
	dirs.dat       = dirs.user + QString ("/Work") + dirs.sub + cmd_args->scenario + QString ("/DAT");
	dirs.hydsim    = dirs.user + QString ("/HydSim");
	dirs.hydro     = dirs.hydsim + PATHSEP + cmd_args->hydro;
    dirs.monte     = dirs.data  + QString ("/datmaker");
    dirs.ppData    = dirs.data  + QString ("/PreProcData");
    dirs.fmData    = dirs.data  + QString ("/FlowModData");
	checkDir (dirs.bin, true);
	checkDir (dirs.mean, true);
//	checkDir (dirs.sub, true);
	checkDir (dirs.final, true);
	checkDir (dirs.flfcast, true);
	checkDir (dirs.flarchive, true);
	checkDir (dirs.tpfcast, true);
	checkDir (dirs.dat, true);
	checkDir (dirs.hydro, true);
	checkDir (dirs.monte, true);
	checkDir (dirs.ppData, true);
	checkDir (dirs.fmData, true);
}

void checkDir (QString dir, bool create)
{
	QString msg;
	QDir check(dir);
	if (!check.exists())
	{
		if (create)
		{
			check.mkpath(dir);
		}
		else
		{
			msg = QString ("File directory ");
			msg.append (dir);
			msg.append (" does not exist.");
            help (msg.toUtf8().data());
		}
	}
}

/* Process lines from "HYDROSIM" file for day and data (Charlie Paulsen SAS file of HYDSIM)
 * Populate hydro hash with dam, year, julian and value
 * flow is daily, spill and elev are monthly values assigned to days
 * last 2 values of line not used: turb_q_daily,spill_prob */
void processModulatedFile()
{
	bool okay;
	QString err;
	int prj_id, alt_id, year, day;
	double flow, spill_day, spill_night, elev;
	alt_id = alternatives.indexOf (cmd_args->altern);
	// HydSim all in one file
	if (cmd_args->mod)
	{
		char * line = NULL;
		QString hydrofile (dirs.hydro + PATHSEP + cmd_args->hydro + QString (".csv"));
		QFile hydro (hydrofile);
		err = checkFile (hydrofile);
		if (err.isEmpty())
		{
			hydro.open (QIODevice::ReadOnly);

			// read the header
			// project,cal_yr,julian_day,daily_flow_kcfs,day_spill_proportion,night_spill_proportion,monthly_elevation_feet
			QStringList tokens = readLine (&hydro, ','); 
			while (!hydro.atEnd())
			{
				// read a data line
				tokens = readLine (&hydro, ','); 
				prj_id      = prj_to_int (tokens[0]);
				year        = tokens[1].toInt() - YEAR_INDEX;
				day         = tokens[2].toInt();
				flow        = tokens[3].toFloat();
				spill_day   = tokens[4].toFloat(&okay);
				if (!okay) spill_day = -1;
				spill_night = tokens[5].toFloat(&okay);
				if (!okay) spill_night = -1;
				elev        = tokens[6].toFloat();
				rh_data[prj_id].flow_kcfs[year][day]  = flow;
				rh_data[prj_id].spill[year][day]      = flow * (spill_day + spill_night)/2;
				if (cmd_args->mod == ALL)
				{
					rh_data[prj_id].plan_spill_day[year][day]   = spill_day;
					rh_data[prj_id].plan_spill_night[year][day] = spill_night;
					if (spill_day >= 0)
						water_rules[prj_id][alt_id].planned_spill_day[day] = 1;
					if (spill_night >= 0)
						water_rules[prj_id][alt_id].planned_spill_night[day] = 1;
				}
				if (elev > 0)
				{
					rh_data[prj_id].elev_feet[year][day]        = elev;
					water_rules[prj_id][alt_id].elevation[day] = 1;
				}
				else
					rh_data[prj_id].elev_feet[year][day]        = 0.0;
			}
			hydro.close();
		}
		else
		{
            cout << err.toUtf8().data() << endl;
		}
	}
	if (err.isEmpty())
	{
	// HydSim data in multiple files
	// daily and monthly values in separate files
	//  == NOT IN USE ==
	//else
	{
	}
	// Add value for Dec 31, prev year, by using Jan 1
	for (prj_id = 0; prj_id < N_DAM; prj_id++)
	{
		for (year = 0; year < 70; year++)
		{
			rh_data[prj_id].flow_kcfs[year][0] = 
				rh_data[prj_id].flow_kcfs[year][1];
			rh_data[prj_id].spill[year][0] = 
				rh_data[prj_id].spill[year][1];
			rh_data[prj_id].plan_spill_day[year][0] = 
				rh_data[prj_id].plan_spill_day[year][1];
			rh_data[prj_id].plan_spill_night[year][0] = 
				rh_data[prj_id].plan_spill_night[year][1];
			rh_data[prj_id].elev_feet[year][0] = 
				rh_data[prj_id].elev_feet[year][1];
		}
	}
	}
}


/* Get historical means from files found in HISTMEAN dir.
 * Populate mean hash with type, dam, julian and value */
void get_mean (QString type, double value[N_DAM][N_DYS])
{
//	char * line;
	int day;
	QString filename = dirs.mean + PATHSEP + QString("mean") + type + QString(".");
	QString readfile;
	QFile infile;
	QStringList tokens;

	for (int i = 0; i < N_DAM; i++)
	{
		readfile = filename + int_to_prj (i);
		infile.setFileName (readfile);
		if (infile.exists())
		{
			infile.open (QIODevice::ReadOnly);
			tokens = readLine (&infile, ' ');
			while (!infile.atEnd())
			{
				tokens  = readLine (&infile, ' ');
				day = tokens[0].toInt();
				value[i][day] = tokens[1].toFloat();
			}
			value[i][0] = value[i][1]; // set Dec 31 = Jan 1
			infile.close();
		}
	}
}
/* write flow.archive file with elevation, spill %, flow data
 * flow.archive file inputs flow, spill, elevation at a dam into COMPASS
 * flow.archive file is the only way to input these values at dams
 */
int writeFlowArchiveFile(QString altName, QStringList dams, int year)
{
	int alt_id = alternatives.indexOf (cmd_args->altern);
	int dam_id, flow, retval, realYear = year + YEAR_INDEX;
	QString faFileName(dirs.flarchive);
	faFileName.append ("/flow.archive."); 
	faFileName.append (QString::number (realYear));
	QFile faFile (faFileName);
	double spill_prop = 0.0;

	if (faFile.open (QIODevice::WriteOnly))
	{
		QString line;
		faFile.write ("V0.9 CRiSP Flow Archive\n");
		faFile.write ("# created from HYDROSIM \"");
        faFile.write (altName.toUtf8());
		faFile.write ("\" flows and rules\n");
		faFile.write ("# spill set to 0; use with yearly input file\n");
		faFile.write ("\n");
		faFile.write ("games   1                # number of games\n");
		faFile.write ("years   1                # number of years\n");
		faFile.write ("\n");
		faFile.write ("dams ");
        faFile.write (QString::number (dams.count()).toUtf8());
		for (int i = 0; i < dams.count(); i++)
		{
			faFile.write (" ");
            faFile.write (dams[i].toUtf8());
		}
		faFile.write ("\n\n");
		faFile.write ("periods 365 0:");
		for (int i = 1; i < 365; i++)
		{
            faFile.write (QString::number (i).toUtf8());
			faFile.write (",");
			if (i%20 == 0)
				faFile.write ("\n");
		}
		faFile.write ("365:365\n");
		faFile.write ("\n");
		faFile.write ("# Sort  Water   Power   Project Pool    Planned Overgen Total     Day     Night\n");
		faFile.write ("# Field Year    Year    Name    Elev    Spill%  Spill%  Flow(cfs) Spill%  Spill%\n");
		faFile.write ("\n");
		faFile.write ("DATA\n");
		for (int day = 1; day <= 365; day++)
		{
			for (int prj = 0; prj < dams.count(); prj ++)
			{
				dam_id = prj_to_int (dams[prj]);
				// day number
				if (day < 10)
					line.append ("00");
				else if (day < 100)
					line.append ("0");
				line.append (QString::number (day));
				// dam number
				line.append ("_0");
				if (prj+1 < 10)
					line.append ("0");
				line.append (QString::number (prj+1));
				// year
				line.append ("\t"); 
				line.append (QString::number (realYear));
				// power year
				line.append ("\t0\t");
				// project abbreviation
				line.append (dams[prj]); 
				// pool elevation
				line.append ("\t");
				line.append (QString::number (rh_data[dam_id].elev_feet[year][day], 'f', 1));
				// spill %
				line.append ("\t");
				if (rh_data[dam_id].spill[year][day] >= 0)
				{
					if (rh_data[dam_id].flowFC[year][day] > 0) 
						spill_prop = rh_data[dam_id].spill[year][day] / rh_data[dam_id].flowFC[year][day];
					line.append (QString::number (spill_prop, 'f', 1));
				}
				else
					line.append ("n/a");
				// overgen spill
				line.append ("\t");
				line.append ("0.0");
				// total flow (cfs)
				line.append ("\t");
				flow = (int)(rh_data[dam_id].flowFC[year][day] * 1000.0);
				line.append (QString::number (flow));
				// day spill % and night spill %
				line.append ("\t");
				if (rh_data[dam_id].plan_spill_day[year][day] >= 0.0 &&
					rh_data[dam_id].plan_spill_night[year][day] >= 0.0)
				{
					spill_prop = rh_data[dam_id].plan_spill_day[year][day];
					line.append (QString::number (spill_prop, 'f', 4));
					line.append ("\t");
					spill_prop = rh_data[dam_id].plan_spill_night[year][day];
					line.append (QString::number (spill_prop, 'f', 4));
					line.append ("\n");
				}
				else
					line.append ("n/a\tn/a\n");

                faFile.write (line.toUtf8());
				line.clear();
			}
		}
		faFile.close();
		retval = 0;
	}
	else
		retval = -1;
	return retval;
}

/* 2. generate temperature data based on HYDSIM data.
 *	  requires: 
 *	  a. temp_paulsen_r.exe
 *	  b. flow forecast file
 *	  c. historical mean files  
 */
void generateTemps()
{
	// write faked summary.alt1 flow file for temperature model in binDir
	writeFlowFcastFile ();
	// run temperature model
	// requires historical means and 
	// requires faked summary.alt1 flow file in binDir
	runTempModel ();
	// process temperature model output and poulate year-based struct
	getTemps ();
}
// Write Flow Forecast files from HYDROSIM w/ HISTMEAN in FLOWFCAST dir.
/* Write a faked summary.alt1 flow file for temperature model in binDir. */
void writeFlowFcastFile ()
{
	QString type ("flowFC");
	int year;
	QFile outfile;
	for (year = 0; year < 70; year++)
	{
		QString fname (dirs.flfcast + PATHSEP + type);
		fname.append (QString (".") + QString::number (year + YEAR_INDEX));
		outfile.setFileName (fname);
		if (outfile.open (QIODevice::WriteOnly))
		{
			outputFlow (CHJ, year, &outfile);
			outputFlow (IHR, year, &outfile);
			outputFlow (LMN, year, &outfile);
			outputFlow (LGS, year, &outfile);
			outputFlow (LWG, year, &outfile);
			outputFlow (BON, year, &outfile);
			outputFlow (TDA, year, &outfile);
			outputFlow (JDA, year, &outfile);
			outputFlow (MCN, year, &outfile);
			outputFlow (PRD, year, &outfile);
			outputFlow (WAN, year, &outfile);
			outputFlow (RRH, year, &outfile);
			outputFlow (RIS, year, &outfile);
			outputFlow (WEL, year, &outfile);
			outputFlow (ANA, year, &outfile);
			outputFlow (PEC, year, &outfile);
		}
		outfile.close();
	}
}
void outputFlow (int prj, int yr, QFile *ofile)
{
	int day;
	QString line;
	QString project (dam_names[prj]);
	UToSpc(project);
	double fl;

	for (day = 0; day < N_DYS; day++)
	{
		fl = rh_data[prj].flow_kcfs[yr][day];
		if (fl <= 0.0)
			fl = flowmean[prj][day];
		rh_data[prj].flowFC[yr][day] = fl;
		if (day == 0)
		{
			line = QString ("Flow for ");
			line.append (project);
			line.append (' ');
//			line.append (QString::number (fl, 'f', 2));
//			line.append (' ');
		}
		line.append (QString::number (fl, 'f', 3));
		line.append (' ');
		if ((day + 1) % 4 == 0)
		{
            ofile->write (line.toUtf8().data());
			line.clear();
			line.append ("\n\t\t\t");
		}
	}
	if (!line.isEmpty())
        ofile->write (line.toUtf8().data());
	ofile->write ("\n");
}
/* Run temperature model using HISTMEAN and faked summary.alt1 file. */
/* Run temp_r.exe and write forecast to TEMPFCAST. 
 * command line is: temp_paulsen_r.exe temp_dir mean_dir year flowfcast_file  */
void runTempModel ()
{
	int year;
	QString flowfcastfile;
	cout << " ... Temperature processing ..." << endl;
	QDir::current().remove ("msgs_temp.txt");
	QDir::current().remove ("msgs_temp_err.txt");
	for (year = 0; year < 70; year++)
	{
        flowfcastfile = dirs.flfcast + QString ("/flowFC.") + QString::number (year + YEAR_INDEX);
		QString cmdline (dirs.bin + QString ("/temp_paulsen_r.exe ") + dirs.tpfcast);
		cmdline.append (' ');
		cmdline.append (dirs.mean + QString(" ") + QString::number (year + YEAR_INDEX));
		cmdline.append (QString(" ") + flowfcastfile);
		cmdline.append (" 2>>msgs_temp_err.txt 1>>msgs_temp.txt");
#ifdef Q_WS_WIN
		cmdline.replace ("/", "\\");
#endif
        cout << cmdline.toUtf8().data() << endl;
        system (cmdline.toUtf8().data());
	}
}
/* Process temperature model output and populate year-based struct. */
/* Get temperature from temperature model in TEMPFCAST. */
void getTemps ()
{
	int dam, yr, year, day;
	QString damAbbv;
	QString temp_file_name;
	QFile temp_file;
	QString line;
	QStringList tokens;
	double temp;
	bool okay;

	for (dam = ANA; dam < N_DAM; dam++)
	{
		damAbbv = int_to_prj (dam);
		for (yr = 0; yr < 70; yr++)
		{
			year = yr + YEAR_INDEX;
			temp_file_name = (dirs.tpfcast);
			temp_file_name.append (QString ("/tempFC."));
			temp_file_name.append (damAbbv);
			temp_file_name.append (".");
			temp_file_name.append (QString::number (year));
			temp_file.setFileName (temp_file_name);
			if (temp_file.exists() && temp_file.open (QIODevice::ReadOnly))
			{
				day = 0;
				while (day <= 365)
				{
					if (temp_file.atEnd())
						break;
					line = QString (temp_file.readLine(256));
					tokens = line.split (' ', QString::SkipEmptyParts);
					if (!tokens[0].startsWith ("#"))
					for (int k = 0; k < tokens.count(); k++)
					{
						temp = tokens[k].toFloat (&okay);
						if (okay)
							rh_data[dam].temp[yr][day++] = temp;
					}
				}
				temp_file.close();
			}
			else
			{
				if (dam != DWR)
                    cout << "No TempFC file: " << temp_file_name.toUtf8().data() << endl;
			}
		}
	}
}

/* 3. generate COMPASS transport files that match operational rules, 
 *    HYDSIM inputs (MAF), and HYDSIM data.
 *    requires:
 *    a. MAF input file used by HYDSIM to generate forecast
 *    b. operational rules for start/stop dates and low/med/high flow years
 * 	     rules can be more complicated, but in recent years this is it 
 */
int generateTransports(QFile *tflowfile, QFile *critfile)
{
	// get rules and populate year-based struct
	if (getRules (dirs.ppData))
	{

	QString flfcastfile (dirs.hydro + PATHSEP + QString(flowcastFileName));
	QString maffile (dirs.ppData + PATHSEP + QString(mafFileName));

	switch (cmd_args->trans)
	{
	// command line -trans=MAF
	// the preferred option
	// Sets transports based on reverse engineering
	// of spill to get MAF forecast.
	case MAF:
		// write spring seasonal avg flow for LWG
		// used for setting transportation 
		// also used by Jim Faulkner for passage dates, post-process
		writeSpringAvgForecast (maffile);
		// modify spill ops based on flow levels and transport dates
		// then write transport file
		modifySpillTransFlow (tflowfile);
		
		// take April MAF forecast and 
		// then write transport file
		// modifySpillTransMAF (); // currently not in use
		break;

	// command line -trans=FLOW
	// not a preferred option
	// Modifies spill operations based on spring
	// averaged flow levels and sets transport start dates
	case FLOW:
		writeSpringAvgForecast (flfcastfile);
		// modify spill ops based on flow levels and transport dates
		// then write transport file
		modifySpillTransFlow (tflowfile);
		break;

	// command line -trans=TEMP
	// not a preferred option
	// Modifies spill operations based on spring
	// temperatures and sets transport start dates
	case TEMP:
		// write spring date for LWG temp > 9.5C
		// used for setting transportation
		writeSpringCrit (LWG, "tempFC", 9.5, 93, 171, critfile);
		// modified spill ops based on temperature and transport dates
		// then write transport file
		modifySpillTransTemp ();
		break;

	// command line -trans not specified
	// not a preferred option
	default:
		// write transport start date based on rules
		writeDefaultCompassTrans ();
		break;
	}
	}
	else
	{
		return -1;
	}
	return 0;
}

/* write LWG spring seasonal average flow for use in setting transport operations
 * create hash of LWG spring seasonal average flow for setting spill ops
 * based on flow
 * FACT CHECKING FILE */
void writeSpringAvgForecast (QString file)
{
	QFile flow_file (file);
	QStringList tokens;
	double flow;
	bool okay;
	int prj, year;

	flow_file.open(QIODevice::ReadOnly);
	while (!flow_file.atEnd())
	{
		tokens = readLine (&flow_file, '\t');
		flow = tokens[2].toFloat(&okay);
		if (okay)
		{
			prj = prj_to_int (tokens[0]);
			year = tokens[1].toInt() - YEAR_INDEX;
			springFlow[prj][year] = flow / 1000.0;
		}
	}
	flow_file.close();
}

/* write LWG spring date when temp exceeds 9.5C for use in 
 * setting transport operations */
void writeSpringCrit (int prj, QString type, float flow, int day1, int day2, QFile *critfile)
{
	int year, day;
	critfile->open (QIODevice::WriteOnly);
	for (year = 0; year < 70; year++)
	{
		for (day = day1; day <= day2; day++)
		{
			if (rh_data[prj].flow_kcfs[year][day] > flow)
			{
				critday[year] = day;
				QString line (cmd_args->altName);
				line.append (".");
				line.append (QString::number (year));
				line.append (".dat\t");
				line.append (QString::number (day));
				line.append ("\t");
				line.append (rh_data[prj].name);
				line.append ("\n");
                critfile->write (line.toUtf8());
			}
		}
	}
	critfile->close ();
}
/* modify spill at transport projects based on spring flow level 
*/
void modifySpillTransFlow (QFile *tflow)
{
	QString transFileName;
	QFile trans;
	int start, start_low, start_med, start_high;
	int flowDam;
	QString dam, altern;
	QString level;
	double range_low, range_high;
	int maxStart, maxStart_low, maxStart_med, maxStart_high;
	int maxEnd, maxEnd_low, maxEnd_med, maxEnd_high;
	int end, end_low, end_med, end_high;
	start = start_low = start_med = start_high = 0;
	flowDam = -1;
	range_low = range_high = 0.0;
	maxStart = maxStart_low = maxStart_med = maxStart_high = 0;
	maxEnd = maxEnd_low = maxEnd_med = maxEnd_high = 0;
	end = end_low = end_med = end_high = 0;

	int alt = alternatives.indexOf (cmd_args->altern);
	if (alt < 0)
		alt = alternatives.indexOf (QString ("2008biopfinal"));
	altern = alternatives[alt];

	for (int i = 0; i < 70; i++)
	{
		transFileName = dirs.final + PATHSEP + cmd_args->altName;
		transFileName.append (".");
		transFileName.append (QString::number (i + YEAR_INDEX));
		transFileName.append (".trans");
		trans.setFileName (transFileName);
		trans.open (QIODevice::WriteOnly);
		trans.close();
	}
	
	if (!tflow->isOpen())
		tflow->open(QIODevice::WriteOnly);

	for (int i = 0; i < trans_list.count(); i++)
	{
		dam = trans_list[i];
		int prj = prj_to_int (dam);
//		for (int alt = 0; alt < N_ALT; alt++)
		{
//			if (water_rules[prj][alt].trans_end_max[0] != 0)
		{
			//dam = int_to_prj (prj);
			start_low = water_rules[prj][alt].trans_start[0];
			start_med = water_rules[prj][alt].trans_start[1];
			start_high = water_rules[prj][alt].trans_start[2];
			if (cmd_args->trans == MAF)
			{
				range_low = water_rules[prj][alt].trans_range_lo_maf;
				range_high = water_rules[prj][alt].trans_range_hi_maf;
			}
			else
			{
				range_low = water_rules[prj][alt].trans_range_lo;
				range_high = water_rules[prj][alt].trans_range_hi;
			}
			maxStart = water_rules[prj][alt].trans_start_max[0];
			maxStart_low = water_rules[prj][alt].trans_start_max[0];
			maxStart_med = water_rules[prj][alt].trans_start_max[1];
			maxStart_high = water_rules[prj][alt].trans_start_max[2];
			maxEnd = water_rules[prj][alt].trans_end_max[0];
			maxEnd_low = water_rules[prj][alt].trans_end_max[0];
			maxEnd_med = water_rules[prj][alt].trans_end_max[1];
			maxEnd_high = water_rules[prj][alt].trans_end_max[2];
			end = water_rules[prj][alt].trans_end_max[0];

			if (start_low != 0)
			{
			for (int year = 0; year < 70; year++)
			{
				double flow = 0.0;
				flow = springFlow[prj][year];
				if (flow == 0.0)
					flow = springFlow[LWG][year];
				if (flow < range_low) 
				{   // low flow year, no spill at transport projects
					level = QString ("LOW");
					start = start_low;
					end = (end_low != 0)? end_low: 365;
					if (maxEnd_low != 0)
						zero_spill (prj, year, maxStart_low, maxEnd_low);
					else if (!cmd_args->mod)
						zero_spill (prj, year, maxStart_low, end);
				}
				else if (flow <= range_high) 
				{   // med flow year, spill until max start date, then no spill
					level = QString ("MED");
					start = start_med;
					end = (end_med != 0)? end_med: 365;
					if (maxEnd_med != 0)
						zero_spill (prj, year, maxStart_med, maxEnd_med);
					else if (!cmd_args->mod)
						zero_spill (prj, year, maxStart_med, end);
				}
				else
				{   // high flow year, no changes to spill ops
					level = QString ("HGH");
					start = start_high;
					end = (end_high != 0)? end_high: 365;
					if (maxEnd_high != 0)
						zero_spill (prj, year, maxStart_high, maxEnd_high);
				}
				if (start != 0)
					writeCompassTrans (prj, start, year, end);
                tflow->write (dam.toUtf8());
				tflow->write ("\t");
                tflow->write (QString::number (year + YEAR_INDEX).toUtf8());
				tflow->write ("\t");
                tflow->write (level.toUtf8());
				tflow->write ("\t");
                tflow->write (QString::number(start).toUtf8());
				tflow->write ("\t");
                tflow->write (QString::number (range_low, 'f', 1).toUtf8());
				tflow->write ("\t");
                tflow->write (QString::number (range_high, 'f', 1).toUtf8());
				tflow->write ("\t");
				tflow->write (return_option (cmd_args->trans));
				tflow->write ("\t");
                tflow->write (QString::number (flow, 'f', 3).toUtf8());
				tflow->write ("\n");

			}
			}
		}
		}
	}
	tflow->close();

}
/* transport projects based on April forecast of MAF April-July runoff
 * hacked subroutine, non-preferred */
void modifySpillTransMaf (QFile *tflow)
{
	int start, start_low, start_high;
	int end, end_low, end_high;
//	double flow;
	double mafTarget = 17.5;
	QString code;
	if (!tflow->isOpen())
		tflow->open (QIODevice::WriteOnly);

	for (int i = 0; i < trans_list.count(); i++)
	{
		int prj = prj_to_int (trans_list[i]);
		int alt = alternatives.indexOf (cmd_args->altern);
		if (alt < 0)
			alt = alternatives.indexOf (QString ("2008biopfinal"));

		if (water_rules[prj][alt].trans_start[0] != 0)
		{
			start_low  = water_rules[prj][alt].trans_start[0];
			start_high = water_rules[prj][alt].trans_start[2];
			end_low    = water_rules[prj][alt].trans_end_max[0];
			end_high   = water_rules[prj][alt].trans_end_max[2];
			for (int year = 0; year < 70; year++)
			{
				if (springFlow[prj][year] < mafTarget)
				{
					// no April spill, start transport as if low flow year
					code = QString("\tBELOW\t");
					start = start_low;
					end = (end_low != 0)? end_low: 365;
					if (cmd_args->mod != 0)
						// no changes to spill ops
						zero_spill (prj, year, start, end);
				}
				else 
				{
					// April spill, start transport as if med/high flow year
					// no changes to spill ops
					code = QString("\tABOVE\t");
					start = start_high;
					end = (end_high != 0)? end_high: 365;

				}
				if (start != 0)
				{
					writeCompassTrans (prj, start, year, end);
                    tflow->write (int_to_prj (prj).toUtf8());
					tflow->write ("\t");
                    tflow->write (QString::number (year).toUtf8());
                    tflow->write (code.toUtf8());
                    tflow->write (QString::number(start).toUtf8());
					tflow->write ("\n");
				}
			}
		}
	}
	tflow->close();
}

/* modify spill at transport projects based on spring temperature at LWG  */
void modifySpillTransTemp ()
{
	int start;
	int end;

	for (int i = 0; i < trans_list.count(); i++)
	{
		int prj = prj_to_int (trans_list[i]);
		int alt = alternatives.indexOf (cmd_args->altern);
		if (alt < 0)
			alt = alternatives.indexOf (QString ("2008biopfinal"));

		for (int year = 0; year < 70; year++)
		{
			start = critday[year];
			end = 365;
			zero_spill (prj, year, start, end);
			writeCompassTrans (prj, start, year, end);
		}
	}
}

/* Set spill to zero    */
void zero_spill (int dam, int yr, int start, int end)
{
	for (int day = start; day <= end; day++)
	{
		rh_data[dam].plan_spill_day[yr][day] = 0.0;
		rh_data[dam].plan_spill_night[yr][day] = 0.0;
		rh_data[dam].spill[yr][day] = 0.0;
	}
}

/* Write a .riv file to input the temperatures to COMPASS */
int writeCompassRiv ()
{
	int retval = 0;
	int year, day; 
	int prj; 
	int items = 0;
	QString filename;
	QFile rivfile;
	QString line;
	QString prj_name, prj_abbv;
	QString type, tempstr;
	int i;//QHash<QString, QString>::const_iterator i;
	double curtemp = 0.0, thistemp;

	for (year = 0; year < 70; year++)
	{
		filename.append (dirs.dat + PATHSEP + cmd_args->altName);
		filename.append (".");
		filename.append (QString::number (year+YEAR_INDEX));
		filename.append (".riv");
		rivfile.setFileName (filename);
		rivfile.open (QIODevice::WriteOnly);
		writeCompassFileHeader ("riv", &rivfile);

		for (i = 0; i < riv_order.count(); i++)
		{
			prj_name = riv_order[i];
			prj_abbv = locationList.value(prj_name);
			prj = prj_to_int (prj_abbv);
			if (prj_name.contains ("dam", Qt::CaseInsensitive))
			{
				type = QString ("dam");
				tempstr = QString ("input_temp On");
			}
			else if (prj_name.contains ("headwater", Qt::CaseInsensitive))
			{
				type = QString ("headwater");
				tempstr = QString ("water_temp");
			}
			else if (prj_name.contains ("reach", Qt::CaseInsensitive))
			{
				type = QString ("reach");
				tempstr = QString ("not_applicable");
			}
			//prj_name = dam_names[prj_to_int (output_dams[prj])];
			line.append ("\t");
			line.append (type);
			line.append (" ");
			line.append (prj_name);//.replace (" ", "_"));
			line.append ("\n");
            rivfile.write (line.toUtf8());
			line.clear();
			line.append ("\t\t");
			line.append (tempstr);
			line.append ("\n");
			for (day = 0; day <= 365; day++)
			{
				thistemp = rh_data[prj].temp[year][day];
				line.append (QString::number (thistemp, 'f', 6));
				line.append ("\n");
			}
			line.append ("\n\tend ");
			line.append (type);
			line.append (" (");
			line.append (prj_name);//.replace (" ", "_"));
			line.append (")\n\n");
            rivfile.write (line.toUtf8());
			line.clear();
		}
		rivfile.close();
		filename.clear();
	}
	return retval;
}
/* Create a COMPASS ctrl file */
int writeCompassCtrl (QString altName)
{
	int retval;
	QString ctrlName (dirs.bin);
	ctrlName.append (PATHSEP + altName);
	ctrlName.append (".ctrl");
	QFile ctrlFile (ctrlName);
	if (ctrlFile.open (QIODevice::WriteOnly))
	{
		writeCompassFileHeader ("ctrl", &ctrlFile);
		ctrlFile.write ("include base.dat\n");
		ctrlFile.write ("include ");
        ctrlFile.write (altName.toUtf8().data());
		ctrlFile.write (".riv\n");
		if (cmd_args->mod > 0 )
			ctrlFile.write ("include include.powerhouse\n");
		ctrlFile.close();
		retval = 0;
	}
	else
		retval = -1;

	return retval;
}

/* Create a COMPASS monte carlo alt file */
int writeCompassAlt (QString altName)
{
	int retval;
	QString altFilename (dirs.monte);
	altFilename.append (PATHSEP);
	altFilename.append (monteRun);
	altFilename.append (".alt");
	QFile altFile (altFilename);
	if (altFile.open (QIODevice::WriteOnly))
	{
		altFile.write ("version 3\n");
		altFile.write ("alternative ");
		altFile.write (monteRun); 
		altFile.write ("\n");
		altFile.write ("	use_flow_from archive\n");
		altFile.write ("	flow_archive_name flow.archive\n");
		altFile.write ("	use_spill_from archive\n");
		altFile.write ("	use_elev_from archive\n");
		altFile.write ("	input_files 1\n");
		altFile.write (" ");
        altFile.write (altName.toUtf8().data());
		altFile.write (".ctrl\n");
		altFile.write ("end alternative (");
		altFile.write (monteRun); 
		altFile.write (")\n");
		retval = 0;
		altFile.close();
	}
	else
	{
		retval = -1;
	}

	return retval;
}

/* Create a COMPASS transport file */
int writeCompassTrans (int prj, int start, int year, int end)
{
	int retval, realYear;
	QString filename;
	QString damName;
	QFile transFile;

	realYear = year + YEAR_INDEX;
	filename = dirs.final + PATHSEP + cmd_args->altName;
	filename.append (".");
	filename.append (QString::number (realYear));
	filename.append (".trans");
	transFile.setFileName (filename);
	if (transFile.open (QIODevice::Append))
	{
		if (transFile.size() == 0)
			writeCompassFileHeader ("trans", &transFile);

		damName = dam_names[prj];

		transFile.write ("\tdam ");
        transFile.write (damName.toUtf8());
		transFile.write ("\n");
		transFile.write ("\t\ttransport \n");
		transFile.write ("\t\t\tstart_by_date 1\n");
		transFile.write ("\t\t\tstart_date ");
        transFile.write (QString::number (start).toUtf8());
		transFile.write ("\n");
		transFile.write ("\t\t\tstart_count 0\n");
		transFile.write ("\t\t\tmax_restarts 0\n");
		transFile.write ("\t\t\trestart_by_date 0\n");
		transFile.write ("\t\t\trestart_date 0\n");
		transFile.write ("\t\t\trestart_count 0\n");
		transFile.write ("\t\t\tend_date ");
        transFile.write (QString::number (end).toUtf8());
		transFile.write ("\n");
		transFile.write ("\t\t\tend_count 0\n");
		transFile.write ("\t\t\tnum_low_days 365\n");
		transFile.write ("\t\t\thfl_passed_perc 1.00\n");
		transFile.write ("\t\t\thigh_flow 0.00\n");
		transFile.write ("\t\t\thigh_flow_species Chinook_1\n");
		transFile.write ("\t\t\ttransport_rate 100.00\n");
		transFile.write ("\t\t\trelease_point Bonneville_Tailrace\n");
		transFile.write ("\t\tend transport (");
        transFile.write (damName.toUtf8());
		transFile.write (")\n");
		transFile.write ("\tend dam (");
        transFile.write (damName.toUtf8());
		transFile.write (")\n\n");

		transFile.close();
		retval = 0;
	}
	else
		retval = -1;

	return retval;
}

int writeDefaultCompassTrans ()
{
	int retval = 0;
	int start, end = 365;
	QFile trans;
	QString transFileName;
	QString dam;

	for (int year = 0; year < 70; year++)
	{
		transFileName = dirs.final + PATHSEP + cmd_args->altName;
		transFileName.append (".");
		transFileName.append (QString::number (year + YEAR_INDEX));
		transFileName.append (".trans");
		trans.setFileName (transFileName);
		trans.open (QIODevice::WriteOnly);
		trans.close();

		for (int prj = 0; prj < N_DAM; prj++)
		{
			for (int alt = 0; alt < N_ALT; alt++)
			{
				if (water_rules[prj][alt].trans_end_max[2] != 0)
				{
					start = water_rules[prj][alt].trans_start[0];
					writeCompassTrans (prj, start, year, end);
				}
			}
		}
	}
	return retval;
}

/* write LWG spring seasonal average flow
 * create hash of LWG spring seasonal average flow for setting spill ops
 * based on flow
 * USED in setting PASSAGE */
void springAverage (int prj, QString type, int start, int end, QFile *altfile)
{
	int day, year, count;
	double sum, average;
	QString output;
	if (!altfile->isOpen())
		altfile->open(QIODevice::WriteOnly);

	for (year = 0; year < 70; year++)
	{
		count = 0;
		sum = 0.0;
		for (day = start; day <= end; day++)
		{
			sum += rh_data[prj].flow_kcfs[year][day];
			count++;
		}
		average = sum / count;
		springFlow[prj][year] = average;
		output.append (cmd_args->altName);
		output.append (".");
		output.append (QString::number (year + YEAR_INDEX));
		output.append (".dat\t");
		output.append (int_to_prj (prj));
		output.append ("\t");
		output.append (QString::number (average, 'f', 12));
		output.append ("\n");
        altfile->write (output.toUtf8());
		output.clear();
	}
//	altfile->close();
}

/* convert spill into spill percent */
void convertSpill (QStringList damlist, QFile * modfile)
{
	int year, day, prj;
//	bool rulepct;
	double flow, flowmax, rulespill, maxspill, minflow, spillp;
	double spill;
	double turbine, maxturbine, overgen, newspill, spillKCFS;
	if (!modfile->isOpen())
		modfile->open(QIODevice::WriteOnly);
	QString output;

	for (prj = 0; prj < damlist.count(); prj++)
	{
		for (year = 0; year < 70; year++)
		{
			for (day = 1; day <= 365; day++)
			{
				spill = rh_data[prj].plan_spill_day[year][day];
				flow  = rh_data[prj].flowFC[year][day];
				flowmax = rh_data[prj].flowmax[year][day];
				rulespill = 0;
				maxspill = rh_data[prj].spillmax[year][day];
				
				if (flow > flowmax)
					rulespill = rh_data[prj].flowmaxspill[year][day];
				else
					rulespill = spill;
				
				minflow = rh_data[prj].flowmin[year][day];
				spillp = 0.0;
				maxturbine = rh_data[prj].ph_cap[year][day] * rh_data[prj].ph_avail[year][day];
				if (rulespill == 0)
				{
					spillp = 0.0;
				}
				else if (rulespill > 1.0)
				{
					if ((flow - rulespill) < minflow)
					{
						spillp = (flow - minflow) / flow;
						output.append (QString("MINFLOW ") + int_to_prj(prj));
						output.append (" ");
						output.append (QString::number (year));
						output.append (" ");
						output.append (QString::number (day));
						output.append (" planned_spill_day: ");
						output.append (QString::number (spillp*100, 'f', 2));
						output.append ("% [flow=");
						output.append (QString::number (flow, 'f', 2));
						output.append ("] [minflow=");
						output.append (QString::number (minflow, 'f', 2));
						output.append ("] [rulespill=");
						output.append (QString::number (rulespill, 'f', 2));
						output.append ("] \n");
                        modfile->write (output.toUtf8());
						output.clear();
					}
					else if ((flow - rulespill) > maxturbine)
					{
						turbine = flow - rulespill;
						overgen = turbine - maxturbine;
						newspill = rulespill + overgen;
						spillp = newspill / flow;
						output.append ("OVERGEN ");
						output.append (int_to_prj(prj));
						output.append (" ");
						output.append (QString::number (year));
						output.append (" ");
						output.append (QString::number (day));
						output.append (" planned_spill_day: ");
						output.append (QString::number (spillp*100, 'f', 2));
						output.append ("% [flow=");
						output.append (QString::number (flow, 'f', 2));
						output.append ("] [maxturb=");
						output.append (QString::number (maxturbine, 'f', 2));
						output.append ("] [rulespill=");
						output.append (QString::number (rulespill, 'f', 2));
						output.append ("] [overgen=");
						output.append (QString::number (overgen, 'f', 2));
						output.append ("] \n");
                        modfile->write (output.toUtf8());
						output.clear();
					}
					else
					{
						spillp = rulespill / flow;
					}
				}
				else
				{
					spillKCFS = rulespill * flow;
					if (spillKCFS > maxspill)
					{
						spillKCFS = maxspill;
						output.append (QString("SPLLCAP "));
						output.append (int_to_prj(prj));
						output.append (" ");
						output.append (QString::number (year));
						output.append (" ");
						output.append (QString::number (day));
						output.append (" planned_spill_day: ");
						output.append (QString::number (spillKCFS, 'f', 2));
						output.append ("% [maxspill=");
						output.append (QString::number (maxspill, 'f', 2));
						output.append ("] [spill=");
						output.append (QString::number (rulespill, 'f', 2));
						output.append ("] [flow=");
						output.append (QString::number (flow, 'f', 2));
						output.append ("] \n");
                        modfile->write (output.toUtf8());
						output.clear();
					}
					if ((flow - spillKCFS) < minflow)
					{
						spillp = (flow - minflow) / flow;
						output.append (QString("MINFLOW ") + int_to_prj(prj));
						output.append (" ");
						output.append (QString::number (year));
						output.append (" ");
						output.append (QString::number (day));
						output.append (" planned_spill_day: ");
						output.append (QString::number (spillp*100, 'f', 2));
						output.append ("% [flow=");
						output.append (QString::number (flow, 'f', 2));
						output.append ("] [minflow=");
						output.append (QString::number (minflow, 'f', 2));
						output.append ("] [rulespill=");
						output.append (QString::number (rulespill, 'f', 2));
						output.append ("] \n");
                        modfile->write (output.toUtf8());
						output.clear();
					}
					else if ((flow - spillKCFS) > maxturbine)
					{
						turbine = flow - spillKCFS;
						overgen = turbine - maxturbine;
						spillp = (spillKCFS + overgen) / flow;
						output.append ("MAXTURB ");
						output.append (int_to_prj(prj));
						output.append (" ");
						output.append (QString::number (year));
						output.append (" ");
						output.append (QString::number (day));
						output.append (" planned_spill_day: ");
						output.append (QString::number (spillp*100, 'f', 2));
						output.append ("% [flow=");
						output.append (QString::number (flow, 'f', 2));
						output.append ("] [maxturb=");
						output.append (QString::number (maxturbine, 'f', 2));
						output.append ("] [rulespill=");
						output.append (QString::number (spillKCFS, 'f', 2));
						output.append ("] [overgen=");
						output.append (QString::number (overgen, 'f', 2));
						output.append ("] \n");
                        modfile->write (output.toUtf8());
						output.clear();
					}
					else 
					{
						spillp = spillKCFS / flow;
					}
				}
				rh_data[prj].plan_spill_day[year][day] = spillp;
				rh_data[prj].plan_spill_night[year][day] = spillp;
			}
		}
	}
	modfile->close();
}

/* write COMPASS ops file in DAT dir based on observed flow and spill
 * not currently in use */
int writeCompassOps (int year, QStringList damlist)
{
	int retval, realYear = year + YEAR_INDEX;
	QString filename (dirs.dat);
	QString damName;
	QFile opsFile;
	int dam, id;
	int day, flag = 0;
	float this_val, hold_val = -1000.0;
	QString line;
	
	filename.append (PATHSEP);
	filename.append (cmd_args->scenario);
	filename.append (".");
	filename.append (QString::number (realYear));
	filename.append (".ops");

	opsFile.setFileName (filename);
	if (opsFile.open (QIODevice::WriteOnly))
	{
		writeCompassFileHeader ("ops", &opsFile);
		for (dam = 0; dam < damlist.count(); dam++)
		{
			line.append ("\tdam ");
			id = prj_to_int (damlist[dam]);
			line.append (dam_names[id]);
			line.append ("\n\t\t spill_cap 1000.00\n");
            opsFile.write (line.toUtf8());
			line.clear();
			line.append ("\t\tplanned_spill_day 0:");
			for (day = 0; day <= 365; day++)
			{
				this_val = rh_data[id].plan_spill_day[year][day];
				if (this_val != hold_val)
				{
					flag++;
					line.append (QString::number (day - 1));
					line.append (" =  ");
					line.append (QString::number (hold_val, 'f', 2));
					if (day == 365)
						break;
					line.append (", ");
					if ((flag % 4) == 0)
						line.append ("\n\t");
					line.append (QString::number (day));
					line.append (":");
					hold_val = this_val;
                    opsFile.write (line.toUtf8());
					line.clear();
				}
			}
			line.append ("\n\t end dam (");
			line.append (dam_names[id]);
			line.append ("\n\n");
            opsFile.write (line.toUtf8());
			line.clear();
		}
		retval = 0;
		opsFile.close();
	}
	else
		retval = -1;

	return retval;
}

/* 4. generate COMPASS DAT file that has all data and settings incorporated
 *    requires:
 *    1. multiple runs of COMPASS
 *    2. write specific version of *.ctrl file to run monte carlo 
 *       "flow.archive" run.
 *       requires:
 *       1. file: generated .riv file w/ temperature
 *       2. file: include.powerhouse (existing) sets COMPASS parameters 
 *          artificially high to not modify HYDSIM values 
 *       3. file: debug_all.etc
 *       4. file: .compass-alts
 *       5. dir: datmaker/
 *       **note: during this run, COMPASS may issue warning messages about
 *         some values and make slight modifications to the values written
 *         out to DAT file. The following scenario run makes sure that all 
 *         values are included and all parameters are within acceptable 
 *         range and will not produce messages when run.
 *    3. rerun COMPASS in scenario mode
 *       requires:
 *       1. DAT file written out by monte carlo "flow.archive" run
 *       2. file: base.etc
 */
void createCompassDat()
{
	// write all flow archive files
	for (int year = 0; year <= 70; year ++)
		writeFlowArchiveFile(cmd_args->altern, output_dams, year);

	// write .riv files
	writeCompassRiv ();

	// write .ctrl and monte carlo .alt files
	writeCompassCtrl (cmd_args->altName);
	writeCompassAlt (cmd_args->altName);
	// run COMPASS in monte mode with flow.archive file
	// run COMPASS in scenario mode
	runCompass();
}

void setDamNames ()
{
	dam_names.append (QString ("Anatone_USGS"));
	dam_names.append (QString ("Bonneville_Dam"));
	dam_names.append (QString ("Chief_Joseph_Dam"));
	dam_names.append (QString ("Dworshak_Dam"));
	dam_names.append (QString ("Ice_Harbor_Dam"));
	dam_names.append (QString ("John_Day_Dam"));
	dam_names.append (QString ("Little_Goose_Dam"));
	dam_names.append (QString ("Lower_Monumental_Dam"));
	dam_names.append (QString ("Lower_Granite_Dam"));
	dam_names.append (QString ("McNary_Dam"));
	dam_names.append (QString ("Peck_USGS"));
	dam_names.append (QString ("Priest_Rapids_Dam"));
	dam_names.append (QString ("Rock_Island_Dam"));
	dam_names.append (QString ("Rocky_Reach_Dam"));
	dam_names.append (QString ("The_Dalles_Dam"));
	dam_names.append (QString ("Wanapum_Dam"));
	dam_names.append (QString ("Wells_Dam"));
}
void setOutputDams ()
{
	output_dams.clear ();
	output_dams.append ("BON");
	output_dams.append ("TDA");
	output_dams.append ("JDA"); 
	output_dams.append ("MCN"); 
	output_dams.append ("PRD"); 
	output_dams.append ("WAN"); 
	output_dams.append ("RIS"); 
	output_dams.append ("RRH"); 
	output_dams.append ("WEL"); 
	output_dams.append ("CHJ"); 
	output_dams.append ("IHR"); 
	output_dams.append ("LMN"); 
	output_dams.append ("LGS"); 
	output_dams.append ("LWG");
}

/* Run COMPASS multiple times to process all the input data and
 * produce clean, i.e., no error messages, yearly DAT files with
 * all data and settings included */
void runCompass()
{
	QString cmdline;
	int year;
	QString datFilename;
	QFile datFile;
	QString finFilename;
	QFile finFile;
	QString sumFilename;
	QFile sumFile;
	QString altFilename;
	QFile altFile;
	QString arcFilename;
	QFile arcFile;
	QString rivFilename;
	QFile rivFile;
	QString msgFilename;
	QFile msgFile;
	QString etcFilename;
	//QFile etcFile;
	QDir current;
	QDir thisdir (dirs.bin);
	current = QDir::current();
	QDir::setCurrent (dirs.bin);
	QString line;

	if (cmd_args->debug != 0)
	{
		etcFilename.append ("debug_");
		etcFilename.append (return_option (cmd_args->debug));
		etcFilename.append (".etc");
	}
	else
	{
		etcFilename.append ("base.etc");
	}
	//etcFile.setFileName (etcFilename);

	for (year = 0; year < 70; year++)
	{
		// file to record all messages during COMPASS runs
		msgFilename = dirs.final + PATHSEP + cmd_args->altName;
		msgFilename.append (".");
		msgFilename.append (QString::number (year + YEAR_INDEX));
		msgFilename.append (".messages.txt");
		msgFile.setFileName (msgFilename);
		thisdir.remove (msgFilename);
		//msgFile.open (QIODevice::WriteOnly);
		// copy year specific files into COMPASS run directory
		// flow.archive contains input flow, spill %, elev
		arcFilename = dirs.flarchive + PATHSEP + QString ("flow.archive.");
		arcFilename.append (QString::number (year + YEAR_INDEX));
		arcFile.setFileName (arcFilename);
		arcFilename = dirs.bin + PATHSEP + QString ("flow.archive");//arcFilename.section ('.', 0, -2);
		thisdir.remove (arcFilename);
		arcFile.copy (arcFilename);
		arcFile.setFileName (arcFilename);
		// .riv file contains input temperature
		rivFilename = dirs.dat + PATHSEP + cmd_args->altName;
		rivFilename.append (QString (".") + QString::number (year + YEAR_INDEX));
		rivFilename.append (".riv");
		rivFile.setFileName (rivFilename);
		rivFilename = dirs.bin + PATHSEP + cmd_args->altName + QString (".riv");//rivFilename.section ('.', 0, 0);
		thisdir.remove (rivFilename);
		rivFile.copy (rivFilename);
		rivFile.setFileName (rivFilename);

		// output files 
		datFilename = dirs.dat + PATHSEP + cmd_args->altName;
		datFilename.append (".");
		datFilename.append (QString::number (year + YEAR_INDEX));
		datFilename.append (".dat");
		thisdir.remove (datFilename);
		altFilename = dirs.dat + QString ("/summary.alt1.") + QString::number (year + YEAR_INDEX);
		sumFilename = dirs.dat + QString ("/summary.dat.") + QString::number (year + YEAR_INDEX);

		// run COMPASS in monte carlo mode to create .dat file with temp, gas, loss, 
		// elev change, spill
		// write out summary.alt1 depending on DEBUG setting
		// --debug=TEMP outputs temperature and flow values
		// --debug=SPILL outputs spill values
		// --debug=ALL outputs temperature, flow, spill values
		// regular mode, output settings are for passage at dams and reaches
//		cmdline = dirs.bin + PATHSEP;
		thisdir.remove ("output.dat");
		thisdir.remove ("messages.txt");
		cmdline = QString ("compassb -lw -b -c debug_all.etc -o output.dat -u >> messages.txt \n");
		//cmdline.append (datFilename + QString (" -u > "));
		//cmdline.append (msgFilename);//.out"));
		QFile messages;
		messages.setFileName ("messages.txt");//msgFilename);
		msgFile.open (QIODevice::WriteOnly);
        msgFile.write (QString::number (year + YEAR_INDEX).toUtf8());
		msgFile.write (" ... Running COMPASS monte carlo mode with flow.archive\n", 59);
        msgFile.write (cmdline.toUtf8());
#ifdef WIN32
		cmdline.replace ("/", "\\");
#endif
		//messages.close();
		cout << endl << year + YEAR_INDEX << " ... Running COMPASS monte carlo mode with flow.archive\n";
        cout << cmdline.toUtf8().data() << endl;
        system (cmdline.toUtf8().data());
		thisdir.remove (datFilename);
		QFile::copy ("output.dat", datFilename);

		// run COMPASS in scenario mode to write out summary.dat file
		if (cmd_args->debug != 0)
		{
//			cmdline = QString ("cd ") + dirs.bin;
			cmdline = QString ("compassb -bs -f output.dat -c ");
			//cmdline.append (datFilename);
			//cmdline.append (" -c ");
			cmdline.append (etcFilename);
			cmdline.append (" \n");
			//messages.open (QIODevice::WriteOnly);
            msgFile.write (QString::number (year + YEAR_INDEX).toUtf8());
			msgFile.write (" ... Running COMPASS scenario mode to create summary file\n", 60);
            msgFile.write (cmdline.toUtf8());
#ifdef WIN32
			cmdline.replace ("/", "\\");
#endif
			cout << year + YEAR_INDEX << " ... Running COMPASS scenario mode to create summary file\n";
            cout << cmdline.toUtf8().data() << endl;
            system (cmdline.toUtf8().data());
		}
		
		// run COMPASS in scenario mode to create single .dat file
		// with output settings for passage 
		finFilename = dirs.final + PATHSEP + cmd_args->altName;
		finFilename.append (".");
		finFilename.append (QString::number (year + YEAR_INDEX));
		finFilename.append (".dat");
//		cmdline = QString ("cd ") + dirs.bin;
		cmdline = QString ("compassb -bs -f output.dat -c base.etc -o final.dat ");
		//cmdline.append (datFilename);
		//cmdline.append (" -c base.etc -o ");
		//cmdline.append (finFilename);
        msgFile.write (QString::number (year + YEAR_INDEX).toUtf8());
		msgFile.write (" ... Running COMPASS scenario mode to create final .dat file\n", 63);
        msgFile.write (cmdline.toUtf8());
#ifdef WIN32
		cmdline.replace ("/", "\\");
#endif
		cout << year + YEAR_INDEX << " ... Running COMPASS scenario mode to create final .dat file\n";
        cout << cmdline.toUtf8().data() << endl;
        system (cmdline.toUtf8().data());
		messages.open (QIODevice::ReadOnly);
		line = QString (messages.readLine (90));
		while (line.size() > 0)
		{
            msgFile.write (line.toUtf8());
			line = QString (messages.readLine (90));
		}
		messages.close();
		msgFile.close();

//		QFile::copy ("messages.txt", msgFilename);
		thisdir.remove (finFilename);
		QFile::copy ("final.dat", finFilename);

		altFile.setFileName (dirs.bin + QString("/summary.alt1"));
		if (altFile.exists())
		{
			thisdir.remove (altFilename);
			altFile.copy (altFilename);
		}

		sumFile.setFileName (dirs.bin + QString("/summary.dat"));
		if (sumFile.exists())
		{
			thisdir.remove (sumFilename);
			sumFile.copy (sumFilename);
		}

	}
	QDir::setCurrent (current.path());
}

/* write averages based on param (either flowFC or altspill)
 * FACT CHECKING FILE */
void write_period_avg (QString type)
{
    int start, end, no, count;
	double value, average = 0.0;
	int damId, realYear;
	QString monthset;
	QStringList items;
	QString dam, period, printstr;
	QString prdFilename = dirs.final + PATHSEP + cmd_args->altName;
	QFile prdFile;
	prdFilename.append (".period.avg.");
	prdFilename.append (type);
	prdFilename.append (".txt");
	prdFile.setFileName (prdFilename);
	prdFile.open (QIODevice::WriteOnly);

	for (int i = 0; i < control_dam_list.count(); i++)
	{
		dam = control_dam_list[i];
		damId = prj_to_int (dam);
		for (int yr = 0; yr < 70; yr++)
		{
			realYear = yr + YEAR_INDEX;
			printstr = QString::number (realYear);
			printstr.append ("\t");
			printstr.append (dam);
            prdFile.write (printstr.toUtf8());

			for (int p = 0; p < control_month_list.count(); p++)
			{
				count = 0;
				value = 0.0;
				period = control_month_list[p];
				monthset = monthPeriods.value (period);
				items = monthset.split (':', QString::SkipEmptyParts);
				start = items[0].toInt();
				end = items [1].toInt();
                no = items[2].toInt();
				for (int j = start; j <= end; j++)
				{
					if (type.contains ("flowFC", Qt::CaseInsensitive))
					{
						value += rh_data[damId].flowFC[yr][j];
						count++;
					}
					else if (type.contains ("altspill", Qt::CaseInsensitive))
					{
						value += rh_data[damId].spillmax[yr][j];
						count++;
					}
				}
				if (count > 0)
					average = value / count;
				//average = (int)((average * 10) / 10);
				prdFile.write ("\t");
                prdFile.write (QString::number (average, 'f', 1).toUtf8());
			}
			prdFile.write ("\n");
		}
	}
	prdFile.close();
}

/* write averages based on param (either "compass" or "hydsim")
 * FACT CHECKING FILE */
void write_period_avg_spill (QString type)
{
    int start, end, no, count;
	double flow, spillpd, spillpn, spill;
	double value, average = 0.0;
	int damId, realYear;
	QString monthset;
	QStringList items;
	QString dam, period, printstr;
	QString sprdFilename;
	QFile sprdFile;
	sprdFilename = dirs.final + PATHSEP + cmd_args->altName;
	sprdFilename.append (".period.avg.spill.");
	sprdFilename.append (type);
	sprdFilename.append (".txt");
	sprdFile.setFileName (sprdFilename);
	sprdFile.open (QIODevice::WriteOnly);
	sprdFile.write ("YEAR\tDAM\tJAN\tFEB\tMAR\tAP1\tAP2\tMAY\tJUN\tJUL\tAG1\tAG2\tSEP\tOCT\tNOV\tDEC\n");
	
	for (int i = 0; i < control_dam_list.count(); i++)
	{
		dam = control_dam_list[i];
		damId = prj_to_int (dam);

		for (int yr = 0; yr < 70; yr++)
		{
			realYear = yr + YEAR_INDEX;
            sprdFile.write (QString::number (realYear).toUtf8());
			sprdFile.write ("\t");
            sprdFile.write (dam.toUtf8());

			for (int p = 0; p < control_month_list.count(); p++)
			{
				period = monthPeriods.value (control_month_list[p]);
				items = period.split (':', QString::SkipEmptyParts);
				start = items[0].toInt();
				end = items[1].toInt();
                no = items[2].toInt();
				if (type.contains ("compass", Qt::CaseInsensitive))
				{
					value = 0;
					count = 0;
					for (int j = start; j <= end; j++)
					{
						flow = rh_data[damId].flowFC[yr][j];
						spillpd = rh_data[damId].plan_spill_day[yr][j];
						spillpn = rh_data[damId].plan_spill_night[yr][j];
						spill = ((spillpd + spillpn) / 2) * flow;
						value += spill;
						count++;
					}
					if (count > 0)
						average = value / count;
					//average = (int)((average * 10) / 10);
					sprdFile.write ("\t");
                    sprdFile.write (QString::number (average, 'f', 1).toUtf8());
				}
				else if (type.contains ("hydsim", Qt::CaseInsensitive))
				{
					value = 0;
					count = 0;
					for (int j = start; j <= end; j++)
					{
						flow = rh_data[damId].flow_kcfs[yr][j];
						spillpd = rh_data[damId].plan_spill_day[yr][j];
						spillpn = rh_data[damId].plan_spill_night[yr][j];
						spill = ((spillpd + spillpn) / 2) * flow;
						value += spill;
						count++;
					}
					if (count > 0)
						average = value / count;
					//average = (int)((average * 10) / 10);
					sprdFile.write ("\t");
                    sprdFile.write (QString::number (average, 'f', 1).toUtf8());
				}
				else
				{
				}
			}
			sprdFile.write ("\n");
		}
	}
	sprdFile.close();
}

/* parse spill from summary.alt1.YYYY */
void parse_summaryalt_spill()
{
	int realYear;
	QString datFilename;
	QFile datFile;
	QString dam;
	int damId = -1, j = 0, i;//, skip;
//	float data;
	QString line;
	QStringList tokens;
	QStringList range;
	int start, end;

	for (int yr = 0; yr < 70; yr++)
	{
		realYear = yr + YEAR_INDEX;
		datFilename = dirs.dat;
		datFilename.append ("/summary.alt1.");
        datFilename.append (QString::number (realYear).toUtf8());
		datFile.setFileName (datFilename);
		datFile.open (QIODevice::ReadOnly);
		while (!datFile.atEnd())
		{
			line = QString (datFile.readLine (132));
			tokens = line.split (" ", QString::SkipEmptyParts);
			if (line.contains ("spill fraction vs. Julian"))
			{
				i = 0;
				for (; i < tokens.count(); i++)
				{
					if (tokens.at(i) == "for")
					{
						dam = tokens[++i];	
						do 
						{
							dam.append (tokens[++i]);
						} while (tokens[i] != "Dam");
						dam.append (tokens[i]);
						damId = prj_to_int (flowMap.key (dam));
						break;
					}
				}
			}
			else 
			{
				for (; i < tokens.count(); i++)
				{
					if (tokens.at(i).contains ("["))
					{
						tokens[i] = tokens[i].section ('[', 1, 1);
						tokens[i] = tokens[i].section (']', 0, 0);
						range = tokens[i].split (":");
						i += 1;
						start = range[0].toInt();
						end = range[1].toInt();
						for (j = start; j <= end; j++)
							rh_data[damId].altspillp[yr][j] = tokens[i].toFloat ();
					}
					else
					{
						rh_data[damId].altspillp[yr][j++] = tokens[i].toFloat();
					}
				}
			}
		}
	}
}

void create_final_files ()
{
	QString cmdline;

	cmdline = QString ("compass_pp3 -name=");
	cmdline.append (cmd_args->scenario);
	cmdline.append (" -input=\"");
	cmdline.append (dirs.final);
	cmdline.append ("\" -rule=");
	cmdline.append (cmd_args->altern);
	cmdline.append (" -dam=");
	cmdline.append (cmd_args->damParam);
	if (!cmd_args->outputDir.isEmpty())
	{
		cmdline.append (" -output=\"");
		cmdline.append (cmd_args->outputDir);
		cmdline.append ("\" 2>msgs_final_err.txt ");
	}
	cout << endl << " ... Produce scenario and release files ..." << endl;
#ifdef WIN32
	cmdline.replace ("/", "\\");
#endif
    cout << cmdline.toUtf8().data() << endl;
    system (cmdline.toUtf8().data());
}

