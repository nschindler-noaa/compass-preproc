#ifndef COMPASS_PP_H
#define COMPASS_PP_H

#include <QString>
#include <QFile>
//#include "rules.h"
#include "../compass_pp/pp_fileIoUtils.h"

#define YEAR_INDEX 1929
#define N_YRS  70    /* 1929 = 0 */
#define N_DYS 366    /* julian days in a year, 1 - 365 (0 is Dec31 prev year) */
#define SPACE    QString (" ");

#define UToSpc(str)    str.replace('_',' ')
#define SpcToU(str)    str.replace(' ','_')

enum options { ALL = 101, SPILL, NOSPILL, FLOW, TEMP, MAF, ERROR };

struct settings {
	QString name;        // name for this scenario
	QString inputDir;    // directory of input files
	QString ruleName;    // name of transport rule (for lwgflowavg filename)
	QString damScenario; // scenario name
	QString workDir;     // directory for temporary files
	QString outputDir;   // output directory if different from input
	bool help;
};
typedef struct settings SETTINGS;
SETTINGS *cmd_args;

void printVersion ();
void printUsageHelp ();
void printVerboseHelp ();
void help();
void help (QString string);
int getArgs (int argc, char *argv[]);
void setDirs();
void checkDir (QString dir, bool create);
int writeCompassCtrl (QStringList filenames, QString ctrlFilename);
void setDamNames();
void setOutputDams ();
void createReleaseFiles ();
void createSpeciesReleaseFiles (QString speciesName, int startDate[70]);
void parseLwgAvgFile(QString filename, double flowavg[70]);
void findStartDates (double flow[70], int ch1[70], int sthd[70]);
void createScenarioFiles ();
void createScenarioFile(QString scnFilename);
int runCOMPASS (QString inputfile, QString outputfile);


#endif
