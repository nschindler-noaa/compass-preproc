#include <iostream>
#include "rules.h"
#include <QFile>
#include <QMessageBox>

using namespace std;

QHash<QString, QString> locationList;
QHash<QString, QString> flowMap;
QHash<QString, QString> poolMap;
QHash<QString, QString> months;
QHash<QString, QString> monthPeriods;
//QStringList flowMap;
//QStringList poolMap;

QStringList ops_params;
QStringList rules_params;
QStringList loc_list;
QStringList dams;
QStringList river_list;
QStringList hist_list;
QStringList spill_list;
QStringList trans_spring_list;
QStringList trans_list;
QStringList usgs_list;
QStringList control_dam_list;
/*int loc_list[N_DAM];
int river_list[N_DAM];
int hist_list[N_DAM];
int spill_list[N_DAM];
int trans_list[N_DAM];
int trans_spring_list[N_DAM];
int usgs_list[N_DAM];

int control_dam_list[N_DAM];*/
QStringList control_month_list;
QStringList control_spill_month_list;

QStringList locations;
QStringList alternatives;

QStringList riv_order;

RULES water_rules[N_DAM][N_ALT];

QString delimiter("'");

/* Get periods and values from "rules" hash for "SCENARIO" and type.
 * Populate hash with type, dam, julian and value*/
int getRules (QString dir, QString name)
{
    int result = 1;
	QString filename;
	QFile rulesFile;
	QString line;
	QString tokens;

	set_alt ();
	set_loc ();

	//QString tempname ("c:/working/pre-processing/compass/modules/rules.pm");

    filename = dir + PATHSEP;
    if (name.isEmpty())
        filename.append (rules_filename);
    else
        filename.append (name);

	rulesFile.setFileName (filename);
	if (rulesFile.open (QIODevice::ReadOnly))
	{
		line = QString (rulesFile.readLine (132));
		while (!rulesFile.atEnd())
		{
			if (line.startsWith ("$rules"))
				readRule (line, &rulesFile);
			else if (line.startsWith ("our"))
				readList (line, &rulesFile);
			else if (line.startsWith ("push"))
				readPush (line);
			line = QString (rulesFile.readLine (132));
        }
        rulesFile.close();
    }
	else
	{
        cout << "ERROR!  Could not find Rules.pm file!" << endl;
        result = -1;
	}
	setRivOrder ();
    return result;
}

/* Setup the rules data */
void setRules ()
{
	QString title ("2008biopfinal");
	//trans_rules = (TRANS_RULES *) calloc (3, sizeof (TRANS_RULES));
	int loc = locations.indexOf ("LWG");
	int alt = alternatives.indexOf (title);

}

/* Read a rule from the rule file and stop at the semi-colon 
 * (may need to read more than one line). */
void readRule (QString line, QFile *file)
{
	int loc;
	int alt;
	QStringList tokens;
	tokens = line.split (delimiter, QString::SkipEmptyParts);

	loc = locations.indexOf (tokens[1]);
	alt = alternatives.indexOf (tokens[3]);

	if      (tokens[5].contains ("transport:start_date"))
		readThreeInt (tokens[6], water_rules[loc][alt].trans_start);
	else if (tokens[5].contains ("transport:max_start"))
		readThreeInt (tokens[6], water_rules[loc][alt].trans_start_max);
	else if (tokens[5].contains ("transport:max_end"))
		readThreeInt (tokens[6], water_rules[loc][alt].trans_end_max);
	else if (tokens[5].contains ("transport:range_lo_maf"))
		readFloat (tokens[6], water_rules[loc][alt].trans_range_lo_maf);
	else if (tokens[5].contains ("transport:range_hi_maf"))
		readFloat (tokens[6], water_rules[loc][alt].trans_range_hi_maf);
	else if (tokens[5].contains ("transport:range_lo"))
		readFloat (tokens[6], water_rules[loc][alt].trans_range_lo);
	else if (tokens[5].contains ("transport:range_hi"))
		readFloat (tokens[6], water_rules[loc][alt].trans_range_hi);
	else if (tokens[5].contains ("transport:flow_dam"))
		readDam (tokens[6], water_rules[loc][alt].trans_flow_dam);
	else if (tokens[5].contains ("planned_spill_day"))
		readDailyFloat (file, tokens, water_rules[loc][alt].planned_spill_day);
	else if (tokens[5].contains ("planned_spill_night"))
		readDailyFloat (file, tokens, water_rules[loc][alt].planned_spill_night);
	else if (tokens[5].contains ("elev"))
		readDailyFloat (file, tokens, water_rules[loc][alt].elevation);
	else if (tokens[5].contains ("ph_avail"))
		readDailyFloat (file, tokens, water_rules[loc][alt].ph_avail);
	else if (tokens[5].contains ("ph_cap"))
		readDailyFloat (file, tokens, water_rules[loc][alt].ph_cap);
	else if (tokens[5].contains ("minflow"))
		readDailyFloat (file, tokens, water_rules[loc][alt].flowmin);
	else if (tokens[5].contains ("flowmax"))
		readDailyFloat (file, tokens, water_rules[loc][alt].flowmax);
	else if (tokens[5].contains ("flowmaxspill"))
		readDailyFloat (file, tokens, water_rules[loc][alt].flowmaxspill);
	else if (tokens[5].contains ("maxspill"))
		readDailyFloat (file, tokens, water_rules[loc][alt].maxspill);
	else if (tokens[5].contains ("spill_cap"))
		readFloat (tokens[6], water_rules[loc][alt].spill_cap);
	else if (tokens[5].contains ("rsw_spill_cap"))
		readFloat (tokens[6], water_rules[loc][alt].rsw_spill_cap);

	// otherwise, ignore the line
}

/* Read a float value from an input line */
float readFloat (QString token, float &value)
{
	bool okay;
	float val = -1.0;

	QStringList parts = token.split ('"');
	val = parts[1].toFloat (&okay);
	if (okay)
		value = val;

	return val;
}
/* Read three integers in a collection, i.e. [90:100:120] */
int readThreeInt (QString token, int iarray[])
{
	int retval = 0;
	bool okay;
	int low, med, hi;

	if (token.contains ("}"))
	{
		QStringList parts = token.split ('"');
		QStringList nums = parts[1].split (':');
		for (int i = 0; i < nums.count(); i++)
		{
			iarray[i] = nums[i].toInt (&okay);
			if (!okay)
				iarray[i] = 0.0;
		}
		if (iarray[1] < iarray[0])
			iarray[1] = iarray[0];
		if (iarray[2] < iarray[1])
			iarray[2] = iarray[1];
/*		low = nums[0].toInt (&okay);
		if (!okay)
			low = 0;
		med = nums[1].toInt (&okay);
		if (!okay)
			med = 100;
		hi = nums[2].toInt (&okay);
		if (!okay)
			hi = 365;
		iarray [0] = low;
		iarray [1] = med;
		iarray [2] = hi;*/
	}
	else
		retval = -1;
	return retval;
}
/* Read an array of float values indexed by julian day (1-365) */
int readDailyFloat (QFile * infile, QStringList tokens, float value[])
{
	int retval = 0, i;
	QString line;
	QStringList parts;
	int start, stop;
	float val;

	while (!infile->atEnd())
	{
		for (i = 0; i < tokens.count(); i++)
		{
			if (tokens[i].contains (':'))
			{
				parts = tokens[i].split (':');
				start = parts[0].toInt ();
				stop = parts[1].toInt ();
				val = parts[2].toFloat ();

				for (int j = start; j <= stop; j++)
					value[j] = val;

				retval = retval > stop? retval: stop;
			}
		}

		if (tokens[i-1].contains (';'))
			break;
		line = QString (infile->readLine (132));
		tokens = line.split ('\'');
	}
	value[0] = value[1];

	return retval;
}

int readDam (QString token, int &id)
{
	QStringList parts = token.split ('"');
	id = prj_to_int (parts[1]);
	return 0;
}
/* Read a list - a hash or a string array - from the rule file
 * and stop at the semi-colon (may need to read more than one line). */
void readList (QString line, QFile *file)
{
	int i;
	QStringList tokens = line.split (' ', QString::SkipEmptyParts);
	if (tokens[1].contains ("%loc_list"))
		readHash (&locationList, file);
	else if (tokens[1].contains ("%dflowMap"))
		readHash (&flowMap, file);
	else if (tokens[1].contains ("%poolMap"))
		readHash (&poolMap, file);
	else if (tokens[1].contains ("%monthsPeriods"))
		readHash (&monthPeriods, file);
	else if (tokens[1].contains ("%months"))
		readHash (&months, file);
	else if (tokens[1].contains ("opsParams"))
		readStrings (&ops_params, line, file);
	else if (tokens[1].contains ("ruleParams"))
		readStrings (&rules_params, line, file);
	else if (tokens[1].contains ("loc_list"))
		readStrings (&loc_list, line, file);
	else if (tokens[1].contains ("dams"))
		readStrings (&dams, line, file);
	else if (tokens[1].contains ("river_list"))
		readStrings (&river_list, line, file);
	else if (tokens[1].contains ("hist_list"))
		readStrings (&hist_list, line, file);
	else if (tokens[1].contains ("spill_list"))
		readStrings (&spill_list, line, file);
	else if (tokens[1].contains ("trans_spring_list"))
		readStrings (&trans_spring_list, line, file);
	else if (tokens[1].contains ("trans_list"))
		readStrings (&trans_list, line, file);
	else if (tokens[1].contains ("usgs_list"))
		readStrings (&usgs_list, line, file);
	else if (tokens[1].contains ("controlDams"))
		readStrings (&control_dam_list, line, file);
	else if (tokens[1].contains ("controlPeriods"))
		readStrings (&control_month_list, line, file);
	else if (tokens[1].contains ("controlSpillPeriods"))
		readStrings (&control_spill_month_list, line, file);
}

/* read the push command and apply it to the appropriate list */
void readPush (QString line)
{
	int i, j;
	QStringList tokens = line.split ('(');
	tokens = tokens[1].split (')');
	tokens = tokens[0].split ('@', QString::SkipEmptyParts);
	if (tokens[0].contains ("river_list", Qt::CaseInsensitive))
	{
		if (tokens[1].contains ("dams", Qt::CaseInsensitive))
		{
			for (j = 0; j < dams.count(); j++)
			{
				river_list.append (dams[j]);
			}
		}
	}
	else if (tokens[0].contains ("hist_list", Qt::CaseInsensitive))
	{
		if (tokens[1].contains ("dams", Qt::CaseInsensitive))
		{
			for (j = 0; j < dams.count(); j++)
			{
				hist_list.append (dams[j]);
			}
		}
	}
}

void readHash (QHash<QString, QString> *hash, QFile *file)
{
	int i, j;
	QString line = file->readLine (132);
	while (line.size() > 0)
	{
		QStringList tokens = line.split (delimiter);
//		if (tokens.size() > 2)
		{
			hash->insert (tokens[1], tokens[3]);
		}
		line = file->readLine (132);
		if (line.contains (";"))
			line.clear();
	}
}

void readStrings (QStringList *list, QString line, QFile *file)
{
	int i, j;
	QStringList tokens;
	while (line.size() > 0)
	{
		tokens = line.split (delimiter);
		if (tokens.size() == 1)
		{
			tokens = line.split (" ");
			for (i = 3, j = 0; i < tokens.size(); i++)
			{
				if (tokens[i].contains ("qw("))
					tokens[i] = tokens[i].section ("qw(", 1, 1);
				if (tokens[i].contains (")"))
				{
					tokens.insert (i + 1, tokens[i].section (")", 1, 1));
					tokens[i] = tokens[i].section (")", 0, 0);
				}
				if (!tokens[i].contains (";"))
					list->insert (j++, tokens[i]);
				else 
					break;
			}
			if (!tokens[i].contains (";"))
				line = file->readLine (132);
			else 
				line.clear();
		}
		else
		{
			for (i = 1, j = 0; i < tokens.size(); i++)
			{
				if (!tokens[i].contains (","))
					list->insert (j++, tokens[i]);
			}
			if (!tokens[i-1].contains (";"))
				line = file->readLine (132);
			else
				line.clear();
		}
	}
}

int prj_to_int (QString prj)
{
	int value = -1;
	if      (prj.contains ("ANA"))
		value = ANA;
	else if (prj.contains ("BON"))
		value = BON;
	else if (prj.contains ("CHJ"))
		value = CHJ;
	else if (prj.contains ("DWR"))
		value = DWR;
	else if (prj.contains ("IHR"))
		value = IHR;
	else if (prj.contains ("JDA"))
		value = JDA;
	else if (prj.contains ("LGS"))
		value = LGS;
	else if (prj.contains ("LMN"))
		value = LMN;
	else if (prj.contains ("LWG"))
		value = LWG;
	else if (prj.contains ("MCN"))
		value = MCN;
	else if (prj.contains ("PEC"))
		value = PEC;
	else if (prj.contains ("PRD"))
		value = PRD;
	else if (prj.contains ("RIS"))
		value = RIS;
	else if (prj.contains ("RRH"))
		value = RRH;
	else if (prj.contains ("TDA"))
		value = TDA;
	else if (prj.contains ("WAN"))
		value = WAN;
	else if (prj.contains ("WEL"))
		value = WEL;

	return value;
}
QString int_to_prj (int id)
{
	QString prj;
	switch (id)
	{
	case ANA:
		prj = QString ("ANA");
		break;
	case BON:
		prj = QString ("BON");
		break;
	case CHJ:
		prj = QString ("CHJ");
		break;
	case DWR:
		prj = QString ("DWR");
		break;
	case IHR:
		prj = QString ("IHR");
		break;
	case JDA:
		prj = QString ("JDA");
		break;
	case LGS:
		prj = QString ("LGS");
		break;
	case LMN:
		prj = QString ("LMN");
		break;
	case LWG:
		prj = QString ("LWG");
		break;
	case MCN:
		prj = QString ("MCN");
		break;
	case PEC:
		prj = QString ("PEC");
		break;
	case PRD:
		prj = QString ("PRD");
		break;
	case RIS:
		prj = QString ("RIS");
		break;
	case RRH:
		prj = QString ("RRH");
		break;
	case TDA:
		prj = QString ("TDA");
		break;
	case WAN:
		prj = QString ("WAN");
		break;
	case WEL:
		prj = QString ("WEL");
		break;
	default:
		prj = QString ("");
	}
	return prj;
}

void set_alt ()
{
	alternatives.insert (BASE, QString("BASE"));
	alternatives.insert (BASE2000, QString("base2000"));
	alternatives.insert (BASE2004, QString("base2004"));
	alternatives.insert (BASE2004_1, QString("base2004-S1"));
	alternatives.insert (BASE2004_2, QString("base2004-S2"));
	alternatives.insert (BASE2004_3, QString("base2004-S3"));
	alternatives.insert (BASE2004_4, QString("base2004-S4"));
	alternatives.insert (BASE2004_5, QString("base2004-S5"));
	alternatives.insert (BASE2004_20070313, QString("base2004-20070313"));
	alternatives.insert (BASE2004_COE, QString("base2004-COE"));
	alternatives.insert (BASE2004_UPA, QString("base2004-UPA"));
	alternatives.insert (BIOP_2007_D1, QString("biop2007-D1"));
	alternatives.insert (BIOP_2008_FINAL, QString("2008biopfinal"));
	alternatives.insert (COALITIONXX, QString("coalitionXX"));
	alternatives.insert (COMPOSITE, QString("composite"));
	alternatives.insert (OPS_2008, QString("2008ops"));
	alternatives.insert (ORCRITFC, QString("ORCRITFC"));
	alternatives.insert (PA2007, QString("pa2007"));
	alternatives.insert (PA2007_A1, QString("pa2007-A1"));
	alternatives.insert (PA2007_A2, QString("pa2007-A2"));
	alternatives.insert (PA2007_B1, QString("pa2007-B1"));
	alternatives.insert (PA2007_C1, QString("pa2007-C1"));
	alternatives.insert (PA2007_D1, QString("pa2007-D1"));
	alternatives.insert (PA2007_M15_A1, QString("pa2007m15-A1"));
	alternatives.insert (PA2007_M15_A2, QString("pa2007m15-A2"));
	alternatives.insert (PA2007_M15_B1, QString("pa2007m15-B1"));
	alternatives.insert (PA2007_M15_C1, QString("pa2007m15-C1"));
	alternatives.insert (PA2007NEW, QString("pa2007new"));
}

void set_loc ()
{
	locations.insert (ANA, QString("ANA"));
	locations.insert (BON, QString("BON"));
	locations.insert (CHJ, QString("CHJ"));
	locations.insert (DWR, QString("DWR"));
	locations.insert (IHR, QString("IHR"));
	locations.insert (JDA, QString("JDA"));
	locations.insert (LGS, QString("LGS"));
	locations.insert (LMN, QString("LMN"));
	locations.insert (LWG, QString("LWG"));
	locations.insert (MCN, QString("MCN"));
	locations.insert (PEC, QString("PEC"));
	locations.insert (PRD, QString("PRD"));
	locations.insert (RIS, QString("RIS"));
	locations.insert (RRH, QString("RRH"));
	locations.insert (TDA, QString("TDA"));
	locations.insert (WAN, QString("WAN"));
	locations.insert (WEL, QString("WEL"));
}

void setRivOrder ()
{
	riv_order.append ("Wanapum_Dam");
	riv_order.append ("Lower_Granite_Dam");
	riv_order.append ("Clearwater_Headwater");
	riv_order.append ("Bonneville_Dam");
	riv_order.append ("Columbia_Headwater");
	riv_order.append ("Little_Goose_Dam");
	riv_order.append ("North_Fork_Clearwater_Headwater");
	riv_order.append ("Rock_Island_Dam");
	riv_order.append ("Priest_Rapids_Dam");
	riv_order.append ("Snake_Headwater");
	riv_order.append ("Wenatchee_Headwater");
	riv_order.append ("Chief_Joseph_Dam");
	riv_order.append ("Salmon_Headwater");
	riv_order.append ("John_Day_Dam");
	riv_order.append ("McNary_Dam");
	riv_order.append ("Lower_Monumental_Dam");
	riv_order.append ("Ice_Harbor_Dam");
	riv_order.append ("Wells_Dam");
	riv_order.append ("Deschutes_Headwater");
	riv_order.append ("The_Dalles_Dam");
	riv_order.append ("Middle_Fork_Clearwater_Headwater");
	riv_order.append ("Methow_Headwater");
	riv_order.append ("Rocky_Reach_Dam");
}
