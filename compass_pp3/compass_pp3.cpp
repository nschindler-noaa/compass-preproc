/*
COMPASS file generation process

Generates COMPASS release and scenario files from pre-processed 
.dat and .trans files (and lwgflowavg.txt file). Incorporates new 
dam parameters and generic .clb file. 

1. Create new release files using the lwgflowavg.txt file and a simple
   linear regression formula.

2. Create a scenario file by including .dat file, .trans file, a 
   .dam file that has additional dam parameters, a generic .clb 
   file and the created release files.


Usage =====================================================================
compass_pp3  -name=Name -input=InputDir  -rule=RuleName  -dam=DamParams 
		     -year=ActualYear -output=OutputDir [-help]

Required ==================================================================
Name        Name for this scenario. Used in output file names.
		     no spaces allowed in name
            example -name=04Base10

InputDir	Input directory (must include .dat and .trans for year and a
			rulename.lwgflowavg.txt file).
		     no spaces allowed in name
		    example: -input=Final/MOD/debug1

RuleName	Name of the rule used to produce the input files.
		     no spaces allowed in name
		    example: -rule=base2004-S1

DamParams	Name of the dam parameter file to use. 
			 no spaces allowed in name
		    example: -dam=dam_params_2004_Base

OutputDir   Output directory if different from input directory. 
			 no spaces allowed in name
		    example: -output=Final/MOD

Optional ===================================================================

-help		Print this information
============================================================================
*/
#include <iostream>
using namespace std;

#include "../fileIoUtils.h"
#include "compass_pp3.h"
#include <QString>
#include <QDir>
#include <QFile>
#include <QProcess>


void printVersion ()
{
	cout << "COMPASS Pre-processor Step 3 version 1.0" << endl << endl;
}
void printUsageHelp ()
{
//Usage======================================================================
QString usage ("compass_pp3  -name=Name -input=InputDir  -rule=RuleName\n"); 
usage.append  ("		       -dam=DamParams-output=OutputDir [-help]\n");
    cout << usage.toUtf8().data() << endl;
}
void printVerboseHelp ()
{
//Required===================================================================
QString verbose ("=Required================\n");
verbose.append ("    Name        Name for this scenario. Used in output file names.\n");
verbose.append ("    		     no spaces allowed in name\n");
verbose.append ("                example -name=04Base10\n\n");

verbose.append ("    InputDir   Input directory (must include .dat and .trans for years \n");
verbose.append ("               1929-1998 inclusive and a rulename.lwgflowavg.txt file).\n");
verbose.append ("                no spaces allowed in name\n");
verbose.append ("               example: -input=Final/MOD/debug1\n\n");

verbose.append ("    RuleName	Name of the transport rule used to produce the input files.\n"); 
verbose.append ("                no spaces allowed in name\n");
verbose.append ("               example: -rule=base2004-S1\n\n");

verbose.append ("    DamParams	Name of the dam parameter file without extension.\n");
verbose.append ("                no spaces allowed in name\n");
verbose.append ("                example: -dam=dam_params_2004_Base\n\n");

verbose.append ("    OutputDir   Output directory if different from input directory.\n");
verbose.append ("     no spaces allowed in name\n");
verbose.append ("    example: -output=Final/MOD/debug1\n\n");

//Optional===================================================================
verbose.append ("=Optional=================\n");
verbose.append ("    help         Print this information (-help).\n\n");
verbose.append ("==========================\n");

    std::cout << verbose.toUtf8().data() << std::endl;
}
void help ()
{
	printVersion ();
	printUsageHelp ();
	printVerboseHelp ();
	exit(1);
}
void help (QString string)
{
    cout << "ERROR: " << string.toUtf8().data() << endl;
	printUsageHelp ();
	exit (1);
}

void error (int err, QString string)
{
    QString msg (QString("ERROR %1: %2").arg(QString::number(err),string));
    cout << msg.toUtf8().data() << endl;
}

void info (QString string)
{
    cout << "INFO:  " << string.toUtf8().data() << endl;
}

SETTINGS args;

int main (int argc, char *argv[])
{
	cmd_args = &args;
	int check;
	QString appPath;

	/* Get options from command line arguments */
	check = getArgs (argc, argv);
	if (check == 0 || 
		cmd_args->help)
		help();
	if (cmd_args->inputDir.isEmpty())
        help (QString("No input file directory provided."));
	if (cmd_args->ruleName.isEmpty())
        help (QString("No transport rule name provided."));
	if (cmd_args->damScenario.isEmpty())
        help (QString("No dam parameter file name provided."));

    // create the work (temporary) directory
	appPath = QDir::current().path();
    cmd_args->workDir = cmd_args->outputDir + QString ("/");
	cmd_args->workDir.append (cmd_args->name);
    cmd_args->workDir.append ("/temp/step3/Work/");
	cmd_args->workDir.append (cmd_args->damScenario.section ('.', 0, 0));

	// add damScenario to output directory, on second thought, don't
	//cmd_args->outputDir.append ("/");
	//cmd_args->outputDir.append (cmd_args->damScenario.section ('.', 0, 0));

	// check for existence of input directory
	checkDir (cmd_args->inputDir, false);

	// check for existence of work and output directories, create if not
	checkDir (cmd_args->workDir, true);
	checkDir (cmd_args->outputDir, true);

	// 1. create new release files
	createReleaseFiles();

	// 2. create scenario files
	createScenarioFiles();

}
int getArgs (int argc, char *argv[])
{
	int i;
	QString arg;
	int ret = 0;

	arg = QString ("");
	cmd_args->help = false;

	for (i = 1; i < argc; i++)
	{
		arg = QString (argv[i]);
		if      (arg.startsWith ("-name", Qt::CaseInsensitive))
		{
			ret = 1;
			cmd_args->name = QString (arg.section ("=", 1));
		}
		else if (arg.startsWith ("-input", Qt::CaseInsensitive))
		{
			ret = 1;
			cmd_args->inputDir = QString (arg.section ("=", 1));
		}
		else if (arg.startsWith ("-rule", Qt::CaseInsensitive))
		{
			ret = 2;
			cmd_args->ruleName = QString (arg.section ("=", 1));
		}
		else if (arg.startsWith ("-dam", Qt::CaseInsensitive))
		{
			ret = 3;
			cmd_args->damScenario = QString (arg.section ("=", 1));
		}
		else if (arg.startsWith ("-output", Qt::CaseInsensitive))
		{
			ret = 4;
			cmd_args->outputDir = QString (arg.section ("=", 1));
		}
		else if (arg.startsWith ("-help", Qt::CaseInsensitive))
		{
			ret = 5;
			cmd_args->help = true;
		}
		else
		{
			ret = 5;
			cmd_args->help = true;
		}
	}
	if (!cmd_args->help)
	{
		if (cmd_args->inputDir.contains ('\\'))
			cmd_args->inputDir.replace ('\\', '/');
		if (cmd_args->outputDir.isEmpty())
			cmd_args->outputDir = cmd_args->inputDir;
		else if (cmd_args->outputDir.contains ('\\'))
			cmd_args->outputDir.replace ('\\', '/');

		if (cmd_args->name.isEmpty())
			cmd_args->name = cmd_args->damScenario;
	}
	return ret;
}


void checkDir (QString dir, bool create)
{
	QString msg;
	QDir check(dir);
	if (!check.exists())
	{
		if (create)
		{
			QDir::current().mkpath (dir);
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

/* Create temporary release files for each year (1929 - 1998) */
void createReleaseFiles ()
{
    double flowavg[N_YRS];
    int ch1start[N_YRS], sthdstart[N_YRS];
	QString lwgflavgFilename (cmd_args->inputDir);
	lwgflavgFilename.append ("/");
	lwgflavgFilename.append (cmd_args->ruleName);
	lwgflavgFilename.append (".lwgflowavg.txt");

    /* Read the 70 (N_YRS) year flow averages for LWG */
	parseLwgAvgFile (lwgflavgFilename, flowavg);

	/* Determine start date */
	findStartDates (flowavg, ch1start, sthdstart);

	/* Create all release files */
	createSpeciesReleaseFiles ("ch1",  ch1start);
	createSpeciesReleaseFiles ("sthd", sthdstart);
}

/* parse average flow from rulename.lwgflowavg.txt */
void parseLwgAvgFile(QString fname, double flow[N_YRS])
{
	int year, realYear;
	QString lwgAvgFilename;
	QFile lwgAvgFile;
	QString line;
	QStringList tokens;

	lwgAvgFilename = fname;
	lwgAvgFile.setFileName (lwgAvgFilename);
	lwgAvgFile.open (QIODevice::ReadOnly);

    for (year = 0; year < N_YRS; year++)
	{
		realYear = year + YEAR_INDEX;
		//line = QString (lwgAvgFile.readLine (132));
		tokens = readLine (&lwgAvgFile, '\t');//line.split (" ", QString::SkipEmptyParts);
		if (tokens[0].contains (QString::number (realYear)))
		{
			flow[year] = tokens[2].toDouble();
		}
	}
}

/* Regression eqns:  startdate = B0 + B1*avgflow
   Regression parameters:
   Chinook
   B0 = 134.8840679 - 36 = 98.88407
   B1 = -0.1423965

   Steelhead
   B0 = 139.8129056 - 44 = 95.8129
   B1 = - 0.1194203
*/
void findStartDates (double flow[N_YRS], int ch1[N_YRS], int sthd[N_YRS])
{
	int i;
	double ch1_B0, ch1_B1, sthd_B0, sthd_B1;

	ch1_B0 = 134.8840679 - 36.0;
	ch1_B1 =  -0.1423965;
	sthd_B0 = 139.8129056 - 44.0;
	sthd_B1 =  -0.1194203;

    for (i = 0; i < N_YRS; i++)
	{
		ch1[i]  = (int) (ch1_B0  + (ch1_B1  * flow[i]) + 0.5);
		sthd[i] = (int) (sthd_B0 + (sthd_B1 * flow[i]) + 0.5);
	}
}

void createSpeciesReleaseFiles (QString speciesName, int startDate[N_YRS])
{
    info ("Creating release files (.rls)");
	int yr, realYear;
	QString templateFilename;
	QFile templateFile;
	QString rlsFilename;
	QFile rlsFile;
	QString line;
	QString dirname;
	QDir thisdir;

	dirname = cmd_args->workDir;
	dirname.append ("/");
	dirname.append (speciesName);
	
	thisdir = QDir (dirname);
	if (!thisdir.exists())
		QDir::current().mkdir (dirname);

    templateFilename = QDir::current().path() + QString ("/pre-proc/step3/");
	templateFilename.append (speciesName);
	templateFilename.append (".wild.lgr.rls.template");
	templateFile.setFileName (templateFilename);

    for (yr = 0; yr < N_YRS; yr++)
	{
		templateFile.open (QIODevice::ReadOnly);
		realYear = yr + YEAR_INDEX;
		rlsFilename = thisdir.path();
		rlsFilename.append ("/");
		rlsFilename.append (speciesName);
		rlsFilename.append (".w.lgrf.");
		rlsFilename.append (QString::number (realYear));
		rlsFilename.append (".rls");
		rlsFile.setFileName (rlsFilename);
//		QDir::mkpath ("c:/temp/compass_pp/step3/Work");
        rlsFile.open (QIODevice::WriteOnly);
        rlsFile.seek(0);

		while (!templateFile.atEnd())
		{
			/* Read a line from the release template */
			line = QString (templateFile.readLine (127));
			/* Change the startdate */
			if (line.contains ("startdate"))
			{
				line.replace ("startdate", QString::number ((int)startDate[yr]));
			}
			/* Write the line to the release file */
            rlsFile.write (line.toUtf8());
		}
		templateFile.close();
		rlsFile.close();
	}
}

/* Create a scenario file for each year (1929 - 1998) */
void createScenarioFiles ()
{
    info ("Creating scenario files (.scn)");
	int i, year, realYear;
	QString yearStr;
	QStringList files;
	QString transScenario (cmd_args->inputDir.section ('/', -1));
	QString transRule (cmd_args->inputDir.section ('/', 1));
	QString ctrlFilename ("temp.ctrl");
	QString damParamFilename (cmd_args->workDir.section ('/', 0, 2));//"step3/");
	QString datFilename (cmd_args->inputDir);
    QString clbFilename ("/clb/calib_feb08.clb");
    QString pbnFilename ("/pbn/SAR_vs_Date.pbn");
	QString transFilename (cmd_args->inputDir);
	QString rlsFilename;
	QString ch1Filename;
	QString sthdFilename;
	QString scnFilename;
	QString finalDirName;
	QDir finalDir, scnDir, ch1Dir, sthdDir;
    QDir thisDir (QDir::current());

	finalDirName = cmd_args->outputDir;
	finalDirName.append ("/");
	finalDirName.append (cmd_args->name);
	finalDir = QDir (finalDirName);
	scnDir = QDir (finalDirName + QString ("/scn"));
	ch1Dir = QDir (finalDirName + QString ("/ch1"));
	sthdDir = QDir (finalDirName + QString ("/sthd"));
	checkDir (scnDir.path(), true);
	checkDir (ch1Dir.path(), true);
	checkDir (sthdDir.path(), true);

    clbFilename = QString(QString("%1/clb/calib_feb08.clb").arg(thisDir.absolutePath()));
    pbnFilename = QString(QString("%1/pbn/SAR_vs_Date.pbn").arg(thisDir.absolutePath()));
    damParamFilename = QString(QString("%1/pre-proc/step3/").arg(thisDir.absolutePath()));
//	damParamFilename.append ("/step3/");
	damParamFilename.append (cmd_args->damScenario);
	if (!(damParamFilename.endsWith (".dam")))
		damParamFilename.append (".dam");

    for (year = 0; year < N_YRS; year++)
	{
		realYear = year + YEAR_INDEX;
		yearStr = QString::number (realYear);
		/* Write a .ctrl file to combine all the data */
		// get all the file names
		datFilename = cmd_args->inputDir;
		datFilename.append ("/");
		datFilename.append (cmd_args->name);
		datFilename.append (".");
		datFilename.append (yearStr);
		datFilename.append (".dat");

		transFilename = cmd_args->inputDir;
		transFilename.append ("/");
		transFilename.append (cmd_args->name);
		transFilename.append (".");
		transFilename.append (yearStr);
		transFilename.append (".trans");

		rlsFilename = cmd_args->workDir;
		rlsFilename.append ("/ch1/ch1.w.lgrf.");
		rlsFilename.append (yearStr);
		rlsFilename.append (".rls");
		// create the release file names
		ch1Filename = ch1Dir.path();
		ch1Filename.append ("/");
		ch1Filename.append (cmd_args->name);
		ch1Filename.append ("_ch1.w.lgrf.");
		ch1Filename.append (yearStr);
		ch1Filename.append (".rls");

		sthdFilename = sthdDir.path();
		sthdFilename.append ("/");
		sthdFilename.append (cmd_args->name);
		sthdFilename.append ("_sthd.w.lgrf.");
		sthdFilename.append (yearStr);
		sthdFilename.append (".rls");

		// Create the output file name(s)
		scnFilename = scnDir.path();
		scnFilename.append ("/");
		scnFilename.append (cmd_args->name);
		scnFilename.append (".");
		scnFilename.append (yearStr);
		scnFilename.append (".scn");

		// create a list of the file names
		files.clear();
		files.append (datFilename);
		files.append (transFilename);
		files.append (clbFilename);
		files.append (pbnFilename);
		files.append (damParamFilename);
		files.append (rlsFilename);

		writeCompassCtrl (files, ctrlFilename);

		/* Run COMPASS to read in the .ctrl file and output a .scn file */
		// Call COMPASS and save the scenario file
		runCOMPASS (ctrlFilename, scnFilename);

		// Call COMPASS again to save the species release file
		runCOMPASS (ctrlFilename, ch1Filename);

		// Substitute the other species release file 
		i = files.indexOf (rlsFilename);
		//files.removeAt (i);
		files[i].replace ("ch1", "sthd");
		//rlsFilename.replace ("ch1", "sthd");
		//files.append (rlsFilename);
		writeCompassCtrl (files, ctrlFilename);

		// call COMPASS again
		runCOMPASS (ctrlFilename, sthdFilename);
        info (QString("Year %1 of %2, Real year %3 complete.").arg(
                  QString::number(year + 1), QString::number(N_YRS), yearStr));

	}
}

/* Run COMPASS with input and output files */
int runCOMPASS (QString inputfile, QString outputfile)
{
    int retval;
	QString cmd;
    QStringList args;
    QProcess compss;
    compss.setStandardOutputFile(QString("msgs_preproc-final.txt"));

	// Create the command line
/*	cmd = QString ("compassb -lw -bs -f \"");
	cmd.append (inputfile);
	cmd.append ("\" -o \"");
	cmd.append (outputfile);
	cmd.append ("\" >msgs_final.txt\n");
#ifdef WIN32
	cmd.replace ("/", "\\");
#endif*/
    cmd = QString(QString ("compassb -bs -f %1 -o %2\n").arg(
                      inputfile, outputfile));
/*    args.append(QString("-lw"));
    args.append(QString("-bs"));
    args.append(QString("-f"));
    args.append(inputfile);
    args.append(QString("-o"));
    args.append(outputfile);*/
    // do the system call
    return compss.execute(cmd);//, args);
/*    if (!compss.waitForFinished())
    {
        retval = 2;
        error (retval, "Process timed out! erronious rezults!");
    }
    return retval;*/
//    return system (cmd.toUtf8().data());
}

void createScenarioFile (QString scnFilename)
{
}

/* Create a COMPASS ctrl file */
int writeCompassCtrl (QStringList filenames, QString ctrlFilename)
{
	int retval, i;
	QString copiedFile;
	copiedFile = cmd_args->outputDir;
	copiedFile.append ("/");
	copiedFile.append (cmd_args->name);
	copiedFile.append ("/");
	copiedFile.append (cmd_args->name);
	copiedFile.append ("_first.ctrl");
	QDir copyDir (QFileInfo (copiedFile).path());
	if (!copyDir.exists())
		copyDir.mkdir (copyDir.path());

	//QString ctrlName (ctrlFilename);
	QFile ctrlFile (ctrlFilename);
	// save the file
	if (!ctrlFile.copy (copiedFile))
		copiedFile.replace ("first", "second");//.append;
	if (!ctrlFile.copy (copiedFile))
	{
		QDir::current().remove (copiedFile);
		ctrlFile.copy (copiedFile);
	}

	QDir thisdir (QFileInfo (ctrlFile).path());

	if (!thisdir.exists())
	{
		thisdir.mkdir (thisdir.path());
	}

	if (ctrlFile.exists())
	{
		thisdir.remove (ctrlFilename);
	}

	if (ctrlFile.open (QIODevice::WriteOnly))
	{
		writeCompassFileHeader ("ctrl", &ctrlFile);
		for (i = 0; i < filenames.count(); i++)
		{
#ifdef WIN32
			filenames[i].replace ('/', '\\');
#endif
			ctrlFile.write ("include \"");
            ctrlFile.write (filenames.at(i).toUtf8());
			ctrlFile.write ("\"\n");
		}
		ctrlFile.close();
		retval = 0;
	}
	else
		retval = -1;

	return retval;
}





