#ifndef PP_RULES_H
#define PP_RULES_H

#ifdef WIN32
#pragma once
#endif

#include <QString>
#include <QStringList>
#include <QHash>
#include <QFile>
#include "pp_fileIoUtils.h"

#define rules_filename  "Rules.pm"

#define N_DYS    366

enum alternative 
{
	BASE,
	BASE2000,
	BASE2004,
	BASE2004_1,
	BASE2004_2,
	BASE2004_3,
	BASE2004_4,
	BASE2004_5,
	BASE2004_20070313,
	BASE2004_COE,
	BASE2004_UPA,
	BIOP_2007_D1,
	BIOP_2008_FINAL,
	COALITIONXX,
	COMPOSITE,
	OPS_2008,
	ORCRITFC,
	PA2007,
	PA2007_A1,
	PA2007_A2,
	PA2007_B1,
	PA2007_C1,
	PA2007_D1,
	PA2007_M15_A1,
	PA2007_M15_A2,
	PA2007_M15_B1,
	PA2007_M15_C1,
	PA2007NEW,
	N_ALT
};

enum location {
	ANA,
	BON,
	CHJ,
	DWR,
	IHR,
	JDA,
	LGS,
	LMN,
	LWG,
	MCN,
	PEC,
	PRD,
	RIS,
	RRH,
	TDA,
	WAN,
	WEL,
	N_DAM
};

struct rules  {
	int   trans_start[3];
	int   trans_start_max[3];
	int   trans_end_max[3];
	float trans_range_lo;
	float trans_range_hi;
	float trans_range_lo_maf;
	float trans_range_hi_maf;
	int   trans_flow_dam;
	float planned_spill_day[N_DYS];
	float planned_spill_night[N_DYS];
	float elevation[N_DYS];
	float ph_avail[N_DYS];
	float ph_cap[N_DYS];
	float flowmin[N_DYS];
	float flowmax[N_DYS];
	float flowmaxspill[N_DYS];
	float maxspill[N_DYS];
	float spill_cap;
	float rsw_spill_cap;
};

typedef struct rules RULES;

int getRules (QString dir, QString name = QString(""));
void setRules ();
void readRule (QString line, QFile *file);
float readFloat (QString token, float &value);
int readThreeInt (QString token, int iarray[]);
int readDailyFloat (QFile * infile, QStringList tokens, float value[]);
int readDam (QString token, int &id);
void readList (QString line, QFile *file);
void readPush (QString line);
void readHash (QHash<QString, QString> *hash, QFile *file);
void readStrings (QStringList *list, QString line, QFile *file);
void setRivOrder ();

int prj_to_int (QString prj);
QString int_to_prj (int id);

void set_alt ();
void set_loc ();


#endif // PP_RULES_H
