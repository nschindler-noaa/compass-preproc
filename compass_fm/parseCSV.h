#ifndef FM_PARSECSV_H
#define FM_PARSECSV_H

#include <QFile>
#include <QDir>
#include <QString>
#include <QStringList>

#define N_YRS  82    /* 1929 = 0 */
#define N_DYS 365    /* julian days in a year */

/* JAN = 0, 2 sections for APR and AUG */
enum months {
	JAN,
	FEB,
	MAR,
	AP1,
	AP2,
	MAY,
	JUN,
	JUL,
	AG1,
	AG2,
	SEP,
	OCT,
	NOV,
	DEC,
	N_MNS
};
enum days {
	JAN_FIRST = 0,
	JAN_LAST = 30,
	FEB_FIRST = 31,
	FEB_LAST = 58,
	MAR_FIRST = 59,
	MAR_LAST = 89,
	AP1_FIRST = 90,
	AP1_LAST = 104,
	AP2_FIRST = 105,
	AP2_LAST = 119,
	MAY_FIRST = 120,
	MAY_LAST = 150,
	JUN_FIRST = 151,
	JUN_LAST = 180,
	JUL_FIRST = 181,
	JUL_LAST = 211,
	AG1_FIRST = 212,
	AG1_LAST = 226,
	AG2_FIRST = 227,
	AG2_LAST = 242,
	SEP_FIRST = 243,
	SEP_LAST = 272,
	OCT_FIRST = 273,
	OCT_LAST = 303,
	NOV_FIRST = 304,
	NOV_LAST = 333,
	DEC_FIRST = 334,
	DEC_LAST = 364,
};

/* Spill proportion during month for all days of typical year. */
struct month_spill_prop_data
{
	int   julian_day[N_DYS];
	double bon[N_DYS];
	double tda[N_DYS];
	double jda[N_DYS];
	double mcn[N_DYS];
	double prd[N_DYS];
	double ihr[N_DYS];
	double lmn[N_DYS];
	double lgs[N_DYS];
	double lwg[N_DYS];
	double hell[N_DYS];
	double dwr[N_DYS];
};
typedef struct month_spill_prop_data MONTH_SPILL_PROP;

struct prj_70_water_years
{
	double flow[70][N_MNS];
};
typedef struct prj_70_water_years PROJECT_ARCHIVE;

/* Actual daily flow for 1995 to 2006 in Kcfs (indexed to 1929) */
struct prj_actual_dy_flow
{
	double bon[79][N_DYS];
	double ihr[79][N_DYS];
	double jda[79][N_DYS];
	double lgs[79][N_DYS];
	double lmn[79][N_DYS];
	double lwg[79][N_DYS];
	double mcn[79][N_DYS];
	double peck[79][N_DYS];
	double prd[79][N_DYS];
	double tda[79][N_DYS];
};
typedef struct prj_actual_dy_flow ACTUAL_FLOW;

struct year_match
{
	int real_yr[70];
};
typedef struct year_match YEAR_MATCH;

struct HS_project_data
{
	QString  name;        /* project/dam name */
	int      number;      /* project/dam number */
	double   q_out[N_YRS][N_MNS];       /* flow */
	double   q_min[N_YRS][N_MNS];       /* minimum flow */
	double   q_force[N_YRS][N_MNS];     /* forced flow */
	double   q_bypass[N_YRS][N_MNS];    /* bypass flow */
	double   q_other[N_YRS][N_MNS];     /* other */
	double   q_overgen[N_YRS][N_MNS];   /* overgen */
	double   elev_feet[N_YRS][N_MNS];   /* water elevation at dam */
	double   q_tot_spill[N_YRS][N_MNS]; /* total spill (forced + bypass + overgen) */
	double   q_turbine[N_YRS][N_MNS];   /* turbine flow (flow - total spill) */
	int      force_on[N_YRS][N_MNS];    /* 1 "on" or 0 "off" */
}; 
typedef struct HS_project_data HS_PROJECT;

struct modulated_flow_data
{
	QString  name;
	double   daily_flow_kcfs[N_YRS][N_DYS];
	double   day_spill_prop[N_YRS][N_DYS];
	double   night_spill_prop[N_YRS][N_DYS];
	double   monthly_elev_feet[N_YRS][N_DYS];
	double   turb_q_daily[N_YRS][N_DYS];
	double   spill_prob[N_YRS][N_DYS];

};
typedef struct modulated_flow_data FINAL_PROJECT;

enum hs_projects { HS_BON, HS_TDA, HS_JDA, HS_MCN, HS_IHR, HS_LMN, HS_LGS, HS_LWG, HS_DWR, HS_HELL, HS_PRD, HS_PRJ };

enum out_projects { ANA, BON, IHR, JDA, LGS, LMN, LWG, MCN, PEC, PRD, TDA, OUT_PRJ };

MONTH_SPILL_PROP *parse_monthly_spill (QString filename);
PROJECT_ARCHIVE * parse_70_yr_arch (QString filename);
YEAR_MATCH *      parse_70_yr_match (QString filename);
ACTUAL_FLOW *     parse_daily_flow (QString filename);
HS_PROJECT *      parse_HydSim (QString filename);
FINAL_PROJECT *   parse_HydSim_daily (QString filename);

void  get_hs_data (int yr, int mo, QString str, HS_PROJECT *prj);
char *get_token (char *str, char delim);
QString substring (QString str, int first, int last);

int calc_mcn_daily_flow_prop (ACTUAL_FLOW *actual, double flow_prop[N_YRS][N_DYS]);
double calc_mcn_ave_daily_flow (int year, int fday, int lday, ACTUAL_FLOW *act);
FINAL_PROJECT *modulate_flow_to_daily (
		HS_PROJECT *hyd, 
		MONTH_SPILL_PROP *prp, 
		YEAR_MATCH *real, 
		double flow_prop[N_YRS][N_DYS]);
int calc_flows(int yr, int mo, int dy,  
		FINAL_PROJECT *project,
		HS_PROJECT *hydro,
	   	double *spill_prop,
		double *flow_prop);
int calc_spill_prop(int yr, int dy, int prj, FINAL_PROJECT *fin);

int output_final (FINAL_PROJECT *fin, QString out = QString(""));
int write_flow_archive (FINAL_PROJECT *fin);

#endif // FM_PARSECSV_H
