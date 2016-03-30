#include <iostream>
#include <string>

#include <QString>

#include "parseCSV.h"

using namespace std;

bool daily;

#ifdef WIN32
#define PATHSEP QString("\\")
#else
#define PATHSEP QString("/")
#endif


const char *version = "COMPASS flow modulation version 1.2\n";
const char *usage = "compass_fm -f hydrosimfile [-d] [-p monthlyproportion] [-o output] [-h] \n"
              "    -f  hydrosimfile is the output from HydSim, there is no \n"
              "        default file name, it is required.               \n"
              "    -d  flag specifies that the flow is in daily values  \n\n"
			  "    -p  monthlyproportion is the file that has the monthly\n"
			  "        proportion of spill by day. If not specified, it \n"
              "        will use 'FlowModData/HS_in_month_spill.csv'.    \n\n"
			  "    -o  output is the filename that is to be created. if \n"
              "        not specified, output to file 'HydSim/output.csv'\n\n"
              "    -h  print out this help information                  \n";

#define prop_file      "FlowModData/HS_in_month_spill.csv"
#define anatone_file   "FlowModData/anatone_70_year_archive.csv"
#define orofino_file   "FlowModData/orofino_70_year_archive.csv"
#define brownlee_file  "FlowModData/brownlee_70_year_archive.csv"
#define history_file   "FlowModData/actual_daily_q.csv"
#define yr_match_file  "FlowModData/year_match_70_yr_2007_12.csv"

bool printhelp()
{
	cout << version << endl << usage << endl;
	return true;
}

int main (int argc, char *argv[])
{
    FINAL_PROJECT *output = NULL;
	double mcn_daily_prop[N_YRS][N_DYS];
	int err = 1;
	// set default file names
    QString datadir;
    QString proportionFile(prop_file);
	QString anatoneFile  (anatone_file);
	QString orofinoFile  (orofino_file);
	QString brownleeFile (brownlee_file);
	QString historyFile  (history_file);
	QString yearMatchFile(yr_match_file);

	QString hydsimFile, outputFile;
	// check command arguments
    daily = false;
	bool help = false;
	for (int i = 1; i < argc && !help; i++)
	{
		if (!strcmp (argv[i], "-h"))
			help = printhelp();
		else if (!strcmp (argv[i], "-f"))
			hydsimFile = QString(argv[++i]);
		else if (!strcmp (argv[i], "-p"))
			proportionFile = QString(argv[++i]);
        else if (!strcmp (argv[i], "-o"))
            outputFile = QString(argv[++i]);
        else if (!strcmp (argv[i], "-d"))
            daily = true;
    }
	if (!help && !hydsimFile.isEmpty())
	{
        datadir = QString(hydsimFile.section(PATHSEP, 0, -3));
        if (!datadir.isEmpty())
        {
            if (!proportionFile.startsWith(datadir))
                proportionFile.prepend(datadir + PATHSEP);
            anatoneFile.prepend (datadir + PATHSEP);
            orofinoFile.prepend (datadir + PATHSEP);
            brownleeFile.prepend(datadir + PATHSEP);
            historyFile.prepend (datadir + PATHSEP);
            yearMatchFile.prepend(datadir + PATHSEP);
        }
		if (outputFile.isEmpty())
		{
#ifdef WIN32
//			outputFile.append ("C:");
#endif
            outputFile = QString(QString("%1%2%3%4%5").arg(datadir, PATHSEP, QString("HydSim"), PATHSEP, QString("output.csv")));
		}
		// read in Anatone and convert flows to KCFS
		PROJECT_ARCHIVE *ana = parse_70_yr_arch (anatoneFile);
		// read in Orofino and convert flows to KCFS
		PROJECT_ARCHIVE *oro = parse_70_yr_arch (orofinoFile);
		// read in Brownlee and convert flows to KCFS
		PROJECT_ARCHIVE *brn = parse_70_yr_arch (brownleeFile);
		// read in HydSim and convert flows to KCFS
        cout << "preproc flow: Reading HydSim file ..." << endl;
        if (daily)
            output = parse_HydSim_daily (hydsimFile);
        else
        {
            HS_PROJECT *hydsim = parse_HydSim (hydsimFile);

            // Add Orofino (Clearwater) to Dworshak and
            // Add Anatone minus Brownlee to Hells Canyon - per RD memo Feb 1, 2007
            for (int i = 0; i < N_YRS; i++)
                for (int j = 0; j < N_MNS; j++)
                {
                    hydsim[HS_DWR].q_out[i][j] += oro->flow[i][j];
                    hydsim[HS_HELL].q_out[i][j] += (ana->flow[i][j] - brn->flow[i][j]);
                }

            // read in historical flows 1995 - 2006
            ACTUAL_FLOW *actual = parse_daily_flow (historyFile);
            // Create daily proportion for McNary for 1995-2006 from historical flows
            err = calc_mcn_daily_flow_prop (actual, mcn_daily_prop);

            // read in year matching to match calendar years with real years
            YEAR_MATCH *match = parse_70_yr_match (yearMatchFile);

            // read in "within month spill.csv"
            MONTH_SPILL_PROP *prop = parse_monthly_spill (proportionFile);

            // Apply McNary proportion to all flows
            // Create final data set
            cout << "preproc flow: Modulating monthly to daily flows ..." << endl;
            output = modulate_flow_to_daily (hydsim, prop, match, mcn_daily_prop);
        }

		// Check for negative turbine flows
		//err = flow_check (output);

        cout << "preproc flow: Writing output file ..." << endl;
		output_final (output, outputFile);

		//write_flow_archive (output);
	}

	return err;
}

