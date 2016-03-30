#ifndef COMPASS_PP_H
#define COMPASS_PP_H

#include <QString>
#include <QFile>
#include "rules.h"

#define YEAR_INDEX 1929
#define N_YRS  82    /* 1929 = 0 */
#define N_DYS 366    /* julian days in a year, 1 - 365 (0 is Dec31 prev year) */
#define SPACE    QString (" ");

#define UToSpc(str)    str.replace('_',' ')
#define SpcToU(str)    str.replace(' ','_')

enum options { NONE = 100, ALL, SPILL, NOSPILL, FLOW, TEMP, MAF, ERROR };

struct settings {
    QString rules;      // Rule file name
    QString altern;     // Alternate name (rule name)
	QString hydsim;     // HydSim output file name
	QString hydro;      // HydSim modulated flow file name
	QString dayspill;   // In month spill fractions (average to 1.0)
	QString scenario;   // Scenario name
	QString altName;    // 
	QString damParam;   // dam parameter file identifier
    QString dataDir;
	QString outputDir;  // directory for final .scn and .rls files
    bool daily;         // daily values in print file
	int mod;            //
	int trans;          //
	bool help;          //
	int debug;          //
};
typedef struct settings SETTINGS;
SETTINGS *cmd_args;


struct directory {
	QString app;        //
    QString data;
	QString user;       //
	QString bin;        //
	QString mean;       //
	QString sub;        //
	QString final;      //
	QString flfcast;    //
	QString tpfcast;    //
	QString dat;        //
	QString flarchive;  //
	QString hydsim;   //
	QString hydro;      //
	QString monte;      //
	QString ppData;
	QString fmData;
};
typedef struct directory DIRS;
DIRS dirs;

struct data_struct
{
	QString  name;
	double   flow_kcfs[N_YRS][N_DYS];
	double   spill[N_YRS][N_DYS];
	double   plan_spill_day[N_YRS][N_DYS];
	double   plan_spill_night[N_YRS][N_DYS];
	double   elev_feet[N_YRS][N_DYS];
	double   temp[N_YRS][N_DYS];
	double   flowFC[N_YRS][N_DYS];
	double   flowmax[N_YRS][N_DYS];
	double   flowmaxspill[N_YRS][N_DYS];
	double   flowmin[N_YRS][N_DYS];
	double   spillmax[N_YRS][N_DYS];
	double   ph_cap[N_YRS][N_DYS];
	double   ph_avail[N_YRS][N_DYS];
	double   altspillp[N_YRS][N_DYS];
};
typedef struct data_struct RH_DATA;

//enum mod_projects { ANA, BON, CHJ, IHR, JDA, LGS, LMN, LWG, MCN, PEC, PRD, RIS, RRH, TDA, WAN, WEL, MOD_PRJ };
RH_DATA rh_data[N_DAM];
double rh_hydro[N_DAM][N_YRS][N_DYS];
double rh_hydsim[N_DAM][N_YRS][N_DYS];

double flowmean[N_DAM][N_DYS];
double tempmean[N_DAM][N_DYS];

int critday[70];

double springFlow[N_DAM][70];
/*
struct transport_rules
{
	QString project;
	QString reftitle;
	int startDate[3];
	int endDate[3];
	int maxStartDate[3];
	int maxEndDate[3];
	double range_low;
	double range_high;
	double range_low_maf;
	double range_high_maf;
};
typedef struct transport_rules TRANS_RULES;
TRANS_RULES trans_rules[N_DAM];*/

#define flowcastFileName "springflow.txt"
#define mafFileName "maf_apr.txt"
#define monteRun "datmaker"

void help();
int get_args (int argc, char *argv[]);//, SETTINGS *args);
int get_option (QString opt);
void setDirs();
void checkDir (QString &dir, bool create);
void findFile (QString &filename, QString dir1, QString dir2);
bool modulated;
void processModulatedFile();
void get_mean (QString type, double value[N_DAM][N_DYS]);
void writeFlowFcastFile ();
void outputFlow (int prj, int yr, QFile *ofile);
void runTempModel ();
void getTemps ();
int writeFlowArchiveFile(QString altName, QStringList dams, int year);
void generateTemps();
int generateTransports(QFile *tflow, QFile *cflow);
void writeSpringAvgForecast (QString);
void writeSpringCrit (int prj, QString type, float flow, int day1, int day2, QFile *cflow);
void modifySpillTransFlow (QFile *tflow);
void modifySpillTransTemp ();
void zero_spill (int dam, int yr, int start, int end);
void springAverage (int prj, QString type, int start, int end, QFile *altfile);
void springAveForecast ();
void convertSpill (QStringList damlist, QFile * modfile);
int writeCompassRiv ();
int writeCompassCtrl (QString altName);
int writeCompassAlt (QString altName);
int writeCompassTrans (int prj, int start, int year, int end);
int writeDefaultCompassTrans ();
void createCompassDat();
void setDamNames();
void setOutputDams ();
void runCompass();
void write_period_avg (QString type);
void write_period_avg_spill (QString type);
void parse_summaryalt_spill();
void create_final_files ();
void copyFile (QString filename, QFile *file);

#endif
