#include "parseCSV.h"

#include <iostream>
#include <qlist.h>
#include <QIODevice>
#include <qstring.h>
#include <QStringList>


#define MIN(x,y) (x) < (y) ? (x) : (y)

QStringList getLine_csv(QFile *infile)
{
	QStringList slist;
	char line[1024];

	if (infile->atEnd())
		slist.append(QString("EOF"));

	int i = infile->readLine (line, sizeof(line));
	if (i > 0)
	{
		QString temp (line);
		slist = temp.split (",");
	}
	else
	{
		slist.append(QString("ERROR"));
	}
	return slist;
}

MONTH_SPILL_PROP *parse_monthly_spill(QString filename)
{
	MONTH_SPILL_PROP *mon_spl_prop = 
		(MONTH_SPILL_PROP *) calloc (1, sizeof(MONTH_SPILL_PROP));
	QFile input(filename);
	int day;

	bool okay = input.open (QIODevice::ReadOnly);
//	input.reset();
	QStringList line = getLine_csv(&input);
	while (!(line = getLine_csv(&input)).at(0).contains("EOF"))
	{
		day = (int)line.at(1).toFloat() - 1;
		mon_spl_prop->bon[day] = line.at(3).toFloat();
		mon_spl_prop->tda[day] = line.at(4).toFloat();
		mon_spl_prop->jda[day] = line.at(5).toFloat();
		mon_spl_prop->mcn[day] = line.at(6).toFloat();
		mon_spl_prop->prd[day] = line.at(7).toFloat();
		mon_spl_prop->ihr[day] = line.at(8).toFloat();
		mon_spl_prop->lmn[day] = line.at(9).toFloat();
		mon_spl_prop->lgs[day] = line.at(10).toFloat();
		mon_spl_prop->lwg[day] = line.at(11).toFloat();
		mon_spl_prop->hell[day] = line.at(12).toFloat();
		mon_spl_prop->dwr[day] = line.at(13).toFloat();
	}
	return mon_spl_prop;
}

PROJECT_ARCHIVE *parse_70_yr_arch(QString filename)
{
	PROJECT_ARCHIVE *prj_arch = 
		(PROJECT_ARCHIVE *) calloc (1, sizeof(PROJECT_ARCHIVE));
	QFile input(filename);
	int year_index;

	bool okay = input.open (QIODevice::ReadOnly);
	QStringList line = getLine_csv(&input);
	while (!(line = getLine_csv(&input)).at(0).contains("EOF"))
	{
		year_index = line.at(0).toInt() - 1929;
		for (int i = 0; i < N_MNS; i++)
			prj_arch->flow[year_index][i] = line.at(i + 1).toFloat(); // / 1000.0;
	}
	return prj_arch;
}

YEAR_MATCH *parse_70_yr_match(QString filename)
{
	YEAR_MATCH *year_match = 
		(YEAR_MATCH *) calloc (70, sizeof(YEAR_MATCH));
	QFile input(filename);
	int year_index;

	bool okay = input.open (QIODevice::ReadOnly);
	input.reset();
	QStringList line = getLine_csv(&input);
	while (!(line = getLine_csv(&input)).at(0).contains("EOF"))
	{
		year_index = line.at(0).toInt() - 1929;
		year_match->real_yr[year_index] = line.at(1).toInt() - 1929;
	}
	return year_match;
}

ACTUAL_FLOW *parse_daily_flow(QString filename)
{
	ACTUAL_FLOW *act_flow = 
		(ACTUAL_FLOW *) calloc (1, sizeof(ACTUAL_FLOW));
	QFile input(filename);
	int day, year;

	bool okay = input.open (QIODevice::ReadOnly);
	input.reset();
	QStringList line = getLine_csv(&input);
	while (!(line = getLine_csv(&input)).at(0).contains("EOF"))
	{
		year = line.at(0).toInt() - 1929;
		day = line.at(1).toInt() - 1;
		act_flow->bon[year][day] = line.at(2).toFloat() * 1000.0;
		act_flow->ihr[year][day] = line.at(3).toFloat() * 1000.0;
		act_flow->jda[year][day] = line.at(4).toFloat() * 1000.0;
		act_flow->lgs[year][day] = line.at(5).toFloat() * 1000.0;
		act_flow->lmn[year][day] = line.at(6).toFloat() * 1000.0;
		act_flow->lwg[year][day] = line.at(7).toFloat() * 1000.0;
		act_flow->mcn[year][day] = line.at(8).toFloat() * 1000.0;
		act_flow->peck[year][day] = line.at(9).toFloat() * 1000.0;
		act_flow->prd[year][day] = line.at(10).toFloat() * 1000.0;
		act_flow->tda[year][day] = line.at(11).toFloat() * 1000.0;
	}
	return act_flow;
}

HS_PROJECT *parse_HydSim(QString filename)
{
	HS_PROJECT *projects = 
		(HS_PROJECT *) calloc (HS_PRJ, sizeof(HS_PROJECT));
	//projects = (HS_PROJECT *) calloc (HS_PRJ, sizeof(HS_PROJECT));
	QFile input(filename);
	char line[132];
	QString linestr;
	QStringList linelst;
	int month, year;
	int yr_1928 = (1928 - 1929);
	int yr_1998 = (1998 - 1929);

	bool okay = input.open (QIODevice::ReadOnly);
	input.reset();
	int check = input.readLine (line, sizeof(line));

	while (check > -1)
	{
        linestr = QString::fromUtf8 (line);
		if (linestr.contains ("BPA REGULATOR OUTPUT"))
		{
			year = substring (linestr, 83, 4).toInt() - 1929;
			QString mo_str = substring (linestr, 46, 10);
			if (mo_str.startsWith ("JANUARY"))
				month = JAN;
			else if (mo_str.startsWith ("FEBRUARY"))
				month = FEB;
			else if (mo_str.startsWith ("MARCH"))
				month = MAR;
			else if (mo_str.startsWith ("APRIL 1-15"))
				month = AP1;
			else if (mo_str.startsWith ("APRIL16-30"))
				month = AP2;
			else if (mo_str.startsWith ("MAY"))
				month = MAY;
			else if (mo_str.startsWith ("JUNE"))
				month = JUN;
			else if (mo_str.startsWith ("JULY"))
				month = JUL;
			else if (mo_str.startsWith ("AUG 1-15"))
				month = AG1;
			else if (mo_str.startsWith ("AUG 16-31"))
				month = AG2;
			else if (mo_str.startsWith ("SEPTEMBER"))
				month = SEP;
			else if (mo_str.startsWith ("OCTOBER"))
				month = OCT;
			else if (mo_str.startsWith ("NOVEMBER"))
				month = NOV;
			else if (mo_str.startsWith ("DECEMBER"))
				month = DEC;
			// Convert water years to calendar years
			// Water years run from July of previous year to June of calendar year.
			if (month >= 7) year = year - 1;    
			// Using the current HydSim data, this results in 3 months (Oct - Dec) of
			// data existing in 1928 and not in 1998. Since there is no fish migration
			// during that time, we can move the data to 1998 so we don't have to deal
			// with zero data. We could copy data, but that requires additional steps. 
			if (year == yr_1928) year = yr_1998; 
		}
		else if (linestr.startsWith (" BONN"))
			get_hs_data (year, month, linestr, &(projects[HS_BON]));
		else if (linestr.startsWith (" DALLES"))
			get_hs_data (year, month, linestr, &(projects[HS_TDA]));
		else if (linestr.startsWith (" J DAY"))
			get_hs_data (year, month, linestr, &(projects[HS_JDA]));
		else if (linestr.startsWith (" MCNARY"))
			get_hs_data (year, month, linestr, &(projects[HS_MCN]));
		else if (linestr.startsWith (" ICE H"))
			get_hs_data (year, month, linestr, &(projects[HS_IHR]));
		else if (linestr.startsWith (" LR MON"))
			get_hs_data (year, month, linestr, &(projects[HS_LMN]));
		else if (linestr.startsWith (" L GOOS"))
			get_hs_data (year, month, linestr, &(projects[HS_LGS]));
		else if (linestr.startsWith (" LR.GRN"))
			get_hs_data (year, month, linestr, &(projects[HS_LWG]));
		else if (linestr.startsWith (" DWRSHK"))
			get_hs_data (year, month, linestr, &(projects[HS_DWR]));
		else if (linestr.startsWith (" HELL C"))
			get_hs_data (year, month, linestr, &(projects[HS_HELL]));
		else if (linestr.startsWith (" PRIEST"))
			get_hs_data (year, month, linestr, &(projects[HS_PRD]));

		check = input.readLine (line, sizeof(line));
	}
/*	projects[HS_BON].name.append ("BON");
	projects[HS_TDA].name.append ("TDA");
	projects[HS_JDA].name.append ("JDA");
	projects[HS_JDA].name.append ("MCN");
	projects[HS_JDA].name.append ("IHR");
	projects[HS_JDA].name.append ("LMN");
	projects[HS_JDA].name.append ("LGS");
	projects[HS_JDA].name.append ("LWG");
	projects[HS_JDA].name.append ("DWR");
	projects[HS_JDA].name.append ("HLC");
	projects[HS_JDA].name.append ("PRD");
*/
	return projects;
}

FINAL_PROJECT *parse_HydSim_daily (QString filename)
{
    FINAL_PROJECT *final =
            (FINAL_PROJECT *) calloc (OUT_PRJ, sizeof(FINAL_PROJECT));
    HS_PROJECT *projects =
        (HS_PROJECT *) calloc (HS_PRJ, sizeof(HS_PROJECT));
    //projects = (HS_PROJECT *) calloc (HS_PRJ, sizeof(HS_PROJECT));
    QFile input(filename);
    char line[132];
    QString linestr;
    QStringList linelst;
    int month, year;
    int yr_1928 = (1928 - 1929);
    int yr_1998 = (1998 - 1929);

    bool okay = input.open (QIODevice::ReadOnly);
    input.reset();
    int check = input.readLine (line, sizeof(line));

    while (check > -1)
    {
        linestr = QString::fromUtf8 (line);
        if (linestr.contains ("BPA REGULATOR OUTPUT"))
        {
            year = substring (linestr, 83, 4).toInt() - 1929;
            QString mo_str = substring (linestr, 46, 10);
            if (mo_str.startsWith ("JANUARY"))
                month = JAN;
            else if (mo_str.startsWith ("FEBRUARY"))
                month = FEB;
            else if (mo_str.startsWith ("MARCH"))
                month = MAR;
            else if (mo_str.startsWith ("APRIL 1-15"))
                month = AP1;
            else if (mo_str.startsWith ("APRIL16-30"))
                month = AP2;
            else if (mo_str.startsWith ("MAY"))
                month = MAY;
            else if (mo_str.startsWith ("JUNE"))
                month = JUN;
            else if (mo_str.startsWith ("JULY"))
                month = JUL;
            else if (mo_str.startsWith ("AUG 1-15"))
                month = AG1;
            else if (mo_str.startsWith ("AUG 16-31"))
                month = AG2;
            else if (mo_str.startsWith ("SEPTEMBER"))
                month = SEP;
            else if (mo_str.startsWith ("OCTOBER"))
                month = OCT;
            else if (mo_str.startsWith ("NOVEMBER"))
                month = NOV;
            else if (mo_str.startsWith ("DECEMBER"))
                month = DEC;
            // Convert water years to calendar years
            // Water years run from July of previous year to June of calendar year.
            if (month >= 7) year = year - 1;
            // Using the current HydSim data, this results in 3 months (Oct - Dec) of
            // data existing in 1928 and not in 1998. Since there is no fish migration
            // during that time, we can move the data to 1998 so we don't have to deal
            // with zero data. We could copy data, but that requires additional steps.
            if (year == yr_1928) year = yr_1998;
        }
        else if (linestr.startsWith (" BONN"))
            get_hs_data (year, month, linestr, &(projects[HS_BON]));
        else if (linestr.startsWith (" DALLES"))
            get_hs_data (year, month, linestr, &(projects[HS_TDA]));
        else if (linestr.startsWith (" J DAY"))
            get_hs_data (year, month, linestr, &(projects[HS_JDA]));
        else if (linestr.startsWith (" MCNARY"))
            get_hs_data (year, month, linestr, &(projects[HS_MCN]));
        else if (linestr.startsWith (" ICE H"))
            get_hs_data (year, month, linestr, &(projects[HS_IHR]));
        else if (linestr.startsWith (" LR MON"))
            get_hs_data (year, month, linestr, &(projects[HS_LMN]));
        else if (linestr.startsWith (" L GOOS"))
            get_hs_data (year, month, linestr, &(projects[HS_LGS]));
        else if (linestr.startsWith (" LR.GRN"))
            get_hs_data (year, month, linestr, &(projects[HS_LWG]));
        else if (linestr.startsWith (" DWRSHK"))
            get_hs_data (year, month, linestr, &(projects[HS_DWR]));
        else if (linestr.startsWith (" HELL C"))
            get_hs_data (year, month, linestr, &(projects[HS_HELL]));
        else if (linestr.startsWith (" PRIEST"))
            get_hs_data (year, month, linestr, &(projects[HS_PRD]));

        check = input.readLine (line, sizeof(line));
    }
/*	projects[HS_BON].name.append ("BON");
    projects[HS_TDA].name.append ("TDA");
    projects[HS_JDA].name.append ("JDA");
    projects[HS_JDA].name.append ("MCN");
    projects[HS_JDA].name.append ("IHR");
    projects[HS_JDA].name.append ("LMN");
    projects[HS_JDA].name.append ("LGS");
    projects[HS_JDA].name.append ("LWG");
    projects[HS_JDA].name.append ("DWR");
    projects[HS_JDA].name.append ("HLC");
    projects[HS_JDA].name.append ("PRD");
*/
    return final;
}

char *get_token (char *str, char delim)
{
	int i = 0, n = 0;
	char *new_str;
	while (str[i] && str[i] != delim && str[i] != '\n')
		n++;
	
	new_str = (char *) malloc(++n);

	if (new_str != NULL)
		for (i = 0; i < n; i++)
			new_str[i] = *(str++);

    return new_str;
}

QString substring (QString instr, int first, int number)
{
	QString outstr = instr;
	outstr.remove (0, first);
	outstr.truncate (number);
	return outstr;
}

void get_hs_data (int yr, int mo, QString str, HS_PROJECT *prj)
{
//	prj->name = QString (substring (str, 1, 6));
	prj->number = substring (str, 9, 4).toInt();
	// read in values
	prj->q_out[yr][mo]     = substring (str, 21, 6).toFloat();// / 1000.0;
	prj->q_min[yr][mo]     = substring (str, 28, 6).toFloat();// / 1000.0;
	prj->q_force[yr][mo]   = substring (str, 35, 6).toFloat();// / 1000.0;
	prj->q_bypass[yr][mo]  = substring (str, 42, 6).toFloat();// / 1000.0;
	prj->q_other[yr][mo]   = substring (str, 49, 6).toFloat();// / 1000.0;
	prj->q_overgen[yr][mo] = substring (str, 56, 6).toFloat();// / 1000.0;
	prj->elev_feet[yr][mo] = substring (str, 91, 6).toFloat();
	// do some calculations
	prj->q_tot_spill[yr][mo] = prj->q_force[yr][mo] + prj->q_bypass[yr][mo] + prj->q_overgen[yr][mo];
	prj->q_turbine[yr][mo] = prj->q_out[yr][mo] - prj->q_tot_spill[yr][mo];
	if (prj->q_force[yr][mo] > 0 || prj->q_overgen[yr][mo] > 0)
		prj->force_on[yr][mo] = 1;
	else 
		prj->force_on[yr][mo] = 0;
}

int calc_mcn_daily_flow_prop (ACTUAL_FLOW *actual, double flow_prop[N_YRS][N_DYS])
{
	double average;
	int year, day, year_start, year_stop;
	year_start = 1995 - 1929;
	year_stop = 2006 - 1929;
	for (year = 0; year <= year_stop; year++)
		for (day = JAN_FIRST; day <= DEC_LAST; day++)
			flow_prop[year][day] = 0.0;

	for (year = year_start; year <= year_stop; year++)
	{
		// January
		average = calc_mcn_ave_daily_flow (year, JAN_FIRST, JAN_LAST, actual);
		if (average <= 0.0) return -1;
		for (day = JAN_FIRST; day <= JAN_LAST; day++)
		{
			flow_prop[year][day] = actual->mcn[year][day] / average;
			if (flow_prop[year][day] < 0) 
				flow_prop[year][day] = 1.0;
		}
		// February
		average = calc_mcn_ave_daily_flow (year, FEB_FIRST, FEB_LAST, actual);
		if (average <= 0.0) return -1;
		for (day = FEB_FIRST; day <= FEB_LAST; day++)
		{
			flow_prop[year][day] = actual->mcn[year][day] / average;
			if (flow_prop[year][day] < 0) 
				flow_prop[year][day] = 1.0;
		}
		// March
		average = calc_mcn_ave_daily_flow (year, MAR_FIRST, MAR_LAST, actual);
		if (average <= 0.0) return -1;
		for (day = MAR_FIRST; day <= MAR_LAST; day++)
		{
			flow_prop[year][day] = actual->mcn[year][day] / average;
			if (flow_prop[year][day] < 0) 
				flow_prop[year][day] = 1.0;
		}
		// April 1-15
		average = calc_mcn_ave_daily_flow (year, AP1_FIRST, AP1_LAST, actual);
		if (average <= 0.0) return -1;
		for (day = AP1_FIRST; day <= AP1_LAST; day++)
		{
			flow_prop[year][day] = actual->mcn[year][day] / average;
			if (flow_prop[year][day] < 0) 
				flow_prop[year][day] = 1.0;
		}
		// April 16-31
		average = calc_mcn_ave_daily_flow (year, AP2_FIRST, AP2_LAST, actual);
		if (average <= 0.0) return -1;
		for (day = AP2_FIRST; day <= AP2_LAST; day++)
		{
			flow_prop[year][day] = actual->mcn[year][day] / average;
			if (flow_prop[year][day] < 0) 
				flow_prop[year][day] = 1.0;
		}
		// May
		average = calc_mcn_ave_daily_flow (year, MAY_FIRST, MAY_LAST, actual);
		if (average <= 0.0) return -1;
		for (day = MAY_FIRST; day <= MAY_LAST; day++)
		{
			flow_prop[year][day] = actual->mcn[year][day] / average;
			if (flow_prop[year][day] < 0) 
				flow_prop[year][day] = 1.0;
		}
		// June
		average = calc_mcn_ave_daily_flow (year, JUN_FIRST, JUN_LAST, actual);
		if (average <= 0.0) return -1;
		for (day = JUN_FIRST; day <= JUN_LAST; day++)
		{
			flow_prop[year][day] = actual->mcn[year][day] / average;
			if (flow_prop[year][day] < 0) 
				flow_prop[year][day] = 1.0;
		}
		// July
		average = calc_mcn_ave_daily_flow (year, JUL_FIRST, JUL_LAST, actual);
		if (average <= 0.0) return -1;
		for (day = JUL_FIRST; day <= JUL_LAST; day++)
		{
			flow_prop[year][day] = actual->mcn[year][day] / average;
			if (flow_prop[year][day] < 0) 
				flow_prop[year][day] = 1.0;
		}
		// August 1-15
		average = calc_mcn_ave_daily_flow (year, AG1_FIRST, AG1_LAST, actual);
		if (average <= 0.0) return -1;
		for (day = AG1_FIRST; day <= AG1_LAST; day++)
		{
			flow_prop[year][day] = actual->mcn[year][day] / average;
			if (flow_prop[year][day] < 0) 
				flow_prop[year][day] = 1.0;
		}
		// August 16-31
		average = calc_mcn_ave_daily_flow (year, AG2_FIRST, AG2_LAST, actual);
		if (average <= 0.0) return -1;
		for (day = AG2_FIRST; day <= AG2_LAST; day++)
		{
			flow_prop[year][day] = actual->mcn[year][day] / average;
			if (flow_prop[year][day] < 0) 
				flow_prop[year][day] = 1.0;
		}
		// September
		average = calc_mcn_ave_daily_flow (year, SEP_FIRST, SEP_LAST, actual);
		if (average <= 0.0) return -1;
		for (day = SEP_FIRST; day <= SEP_LAST; day++)
		{
			flow_prop[year][day] = actual->mcn[year][day] / average;
			if (flow_prop[year][day] < 0) 
				flow_prop[year][day] = 1.0;
		}
		// October
		average = calc_mcn_ave_daily_flow (year, OCT_FIRST, OCT_LAST, actual);
		if (average <= 0.0) return -1;
		for (day = OCT_FIRST; day <= OCT_LAST; day++)
		{
			flow_prop[year][day] = actual->mcn[year][day] / average;
			if (flow_prop[year][day] < 0) 
				flow_prop[year][day] = 1.0;
		}
		// November
		average = calc_mcn_ave_daily_flow (year, NOV_FIRST, NOV_LAST, actual);
		if (average <= 0.0) return -1;
		for (day = NOV_FIRST; day <= NOV_LAST; day++)
		{
			flow_prop[year][day] = actual->mcn[year][day] / average;
			if (flow_prop[year][day] < 0) 
				flow_prop[year][day] = 1.0;
		}
		// December
		average = calc_mcn_ave_daily_flow (year, DEC_FIRST, DEC_LAST, actual);
		if (average <= 0.0) return -1;
		for (day = DEC_FIRST; day <= DEC_LAST; day++)
		{
			flow_prop[year][day] = actual->mcn[year][day] / average;
			if (flow_prop[year][day] < 0) 
				flow_prop[year][day] = 1.0;
		}
	}
	return 1;
}
double calc_mcn_ave_daily_flow (int year, int fday, int lday, ACTUAL_FLOW *act)
{
	int day;
	double sum, ave;
	sum = ave = 0.0;
	for (day = fday; day <= lday; day++)
	{
		sum += act->mcn[year][day];
	}
	ave = sum / (lday - fday + 1);
	return ave;
}

FINAL_PROJECT *modulate_flow_to_daily (
		HS_PROJECT *hyd, 
		MONTH_SPILL_PROP *prp, 
		YEAR_MATCH *real, 
		double flow_prop[N_YRS][N_DYS])
{
	int year_start = 1929 - 1929;
	int year_stop = 1998 - 1929;
	int year, year_match;
	int prj = ANA; 
	int month, day;

	FINAL_PROJECT *final = 
		(FINAL_PROJECT *) calloc (OUT_PRJ, sizeof(FINAL_PROJECT));

	for (year = year_start; year <= year_stop; year++)
	{
		year_match = real->real_yr[year];
		// January
		for (day = JAN_FIRST; day <= DEC_LAST; day++)
		{
			if (day <= JAN_LAST)
				month = JAN;
			else if (day <= FEB_LAST)
				month = FEB;
			else if (day <= MAR_LAST)
				month = MAR;
			else if (day <= AP1_LAST)
				month = AP1;
			else if (day <= AP2_LAST)
				month = AP2;
			else if (day <= MAY_LAST)
				month = MAY;
			else if (day <= JUN_LAST)
				month = JUN;
			else if (day <= JUL_LAST)
				month = JUL;
			else if (day <= AG1_LAST)
				month = AG1;
			else if (day <= AG2_LAST)
				month = AG2;
			else if (day <= SEP_LAST)
				month = SEP;
			else if (day <= OCT_LAST)
				month = OCT;
			else if (day <= NOV_LAST)
				month = NOV;
			else if (day <= DEC_LAST)
				month = DEC;
			else 
				month = 0;

			calc_flows (year, month, day, &final[ANA], &hyd[HS_HELL], 
				prp->hell, flow_prop[year_match]);
			calc_spill_prop (year, day, ANA, final);

			calc_flows (year, month, day, &final[BON], &hyd[HS_BON], 
				prp->bon, flow_prop[year_match]);
			calc_spill_prop (year, day, BON, final);

			calc_flows (year, month, day, &final[IHR], &hyd[HS_IHR], 
				prp->ihr, flow_prop[year_match]);
			calc_spill_prop (year, day, IHR, final);

			calc_flows (year, month, day, &final[JDA], &hyd[HS_JDA], 
				prp->jda, flow_prop[year_match]);
			calc_spill_prop (year, day, JDA, final);

			calc_flows (year, month, day, &final[LGS], &hyd[HS_LGS], 
				prp->lgs, flow_prop[year_match]);
			calc_spill_prop (year, day, LGS, final);

			calc_flows (year, month, day, &final[LMN], &hyd[HS_LMN], 
				prp->lmn, flow_prop[year_match]);
			calc_spill_prop (year, day, LMN, final);

			calc_flows (year, month, day, &final[LWG], &hyd[HS_LWG], 
				prp->lwg, flow_prop[year_match]);
			calc_spill_prop (year, day, LWG, final);

			calc_flows (year, month, day, &final[MCN], &hyd[HS_MCN], 
				prp->mcn, flow_prop[year_match]);
			calc_spill_prop (year, day, MCN, final);

			calc_flows (year, month, day, &final[PEC], &hyd[HS_DWR], 
				prp->dwr, flow_prop[year_match]);
			calc_spill_prop (year, day, PEC, final);

			calc_flows (year, month, day, &final[PRD], &hyd[HS_PRD], 
				prp->prd, flow_prop[year_match]);
			calc_spill_prop (year, day, PRD, final);

			calc_flows (year, month, day, &final[TDA], &hyd[HS_TDA], 
				prp->tda, flow_prop[year_match]);
			calc_spill_prop (year, day, TDA, final);

		}
	}

	return final;
}

int calc_flows(int yr, int mo, int dy,  
		FINAL_PROJECT *project,
		HS_PROJECT *hydro,
	   	double *spill_prop,
		double *flow_prop)
{
	// calculate daily flow using McNary variation
	project->daily_flow_kcfs[yr][dy] = 
		(double)(hydro->q_out[yr][mo]) / 1000.0 * flow_prop[dy];     
	// calculate day and night spill 
	project->day_spill_prop[yr][dy] =     
		(double)(hydro->q_tot_spill[yr][mo] - hydro->q_bypass[yr][mo] +
		(double)spill_prop[dy] * hydro->q_bypass[yr][mo]) / 1000.0 * flow_prop[dy];   
	project->night_spill_prop[yr][dy] =     
		(double)(hydro->q_tot_spill[yr][mo] - hydro->q_bypass[yr][mo] +
		(double)spill_prop[dy] * hydro->q_bypass[yr][mo]) / 1000.0 * flow_prop[dy];   
	// copy elevation
	project->monthly_elev_feet[yr][dy] =   
		hydro->elev_feet[yr][mo];            
	// calculate spill probability
	project->spill_prob[yr][dy] = 0;
	/*project->spill_prob[yr][dy] =          
		project->day_spill_prop[yr][dy] /   
		project->daily_flow_kcfs[yr][dy];*/
	// calculate turbine flow
	project->turb_q_daily[yr][dy] =        
		project->daily_flow_kcfs[yr][dy] - project->day_spill_prop[yr][dy];

	return 1;
}

int calc_spill_prop(int yr, int dy, int prj, FINAL_PROJECT *final)  
{
	// Find turbine flow minimum
	double limit;
	switch (prj)
	{
		// Anatone and Peck are not dams (no elev)
	case ANA:
	case PEC:
		limit = 0.0;
		final[prj].day_spill_prop[yr][dy] = 0.0;
		final[prj].night_spill_prop[yr][dy] = 0.0;
		final[prj].monthly_elev_feet[yr][dy] = 0.0;
		final[prj].turb_q_daily[yr][dy] = 
			final[prj].daily_flow_kcfs[yr][dy];
		break;

		// Numbers for min turbine flow, except PRD, from KAF 6-1
	case BON:
		limit = 30.0;
		break;

	case TDA:
	case JDA:
	case MCN:
		limit = 50.0;
		break;

	case IHR:
		limit = 9.5;
		break;

	case LWG:
	case LMN:
	case LGS:
		limit = 11.5;
		break;
		
	case PRD:    // *** BOGUS NUMBER *** 6-1
	default:
		limit = 50.0;
		break;
	}
	// Check for negative turbine flows
	if (final[prj].turb_q_daily[yr][dy] < limit)
	{
		// Recalculate spill
		final[prj].turb_q_daily[yr][dy] = MIN (limit, final[prj].daily_flow_kcfs[yr][dy]);

		final[prj].day_spill_prop[yr][dy] = 
			(final[prj].daily_flow_kcfs[yr][dy] - final[prj].turb_q_daily[yr][dy]);
		final[prj].night_spill_prop[yr][dy] = 
			(final[prj].daily_flow_kcfs[yr][dy] - final[prj].turb_q_daily[yr][dy]);
	}
	// Calculate proportion
	final[prj].day_spill_prop[yr][dy] /= final[prj].daily_flow_kcfs[yr][dy];
	final[prj].night_spill_prop[yr][dy] /= final[prj].daily_flow_kcfs[yr][dy];
	return 1;
}

int output_final (FINAL_PROJECT *fin, QString filename)
{
    bool standardout = false;
	QStringList projects;
	int prj, year, day, real_year;
    QFile outFile;
    QDir newdir; // (filename.section ('/', 0, -2));

    if (filename.isEmpty())
        standardout = true;
    else
    {
        outFile.setFileName(filename);
        newdir.setPath(outFile.fileName().section ('/', 0, -2));
        if (!newdir.exists())
            newdir.mkpath (newdir.path());
    }

    projects.append(QString("ANA"));
    projects.append(QString("BON"));
    projects.append(QString("IHR"));
    projects.append(QString("JDA"));
    projects.append(QString("LGS"));
    projects.append(QString("LMN"));
    projects.append(QString("LWG"));
    projects.append(QString("MCN"));
    projects.append(QString("PEC"));
    projects.append(QString("PRD"));
    projects.append(QString("TDA"));


//    if (!outFile.open (QIODevice::ReadWrite))// | QIODevice::Text | QIODevice::Truncate))
//        standardout = true;
//    else
//        outFile.seek(0);
    outFile.open (QIODevice::WriteOnly);
//	outFile.reset();
	QString line ("project,cal_yr,julian_day,daily_flow_kcfs,day_spill_proportion,night_spill_proportion,monthly_elevation_feet,turb_q_daily,spill_prob \x0D\x0A");
    outFile.write (line.toUtf8());

	for (prj = 0; prj < OUT_PRJ; prj++)
	{
		for (year = 0; year < 70; year++)
		{
			for (day = 0; day < N_DYS; day++)
			{
				real_year = year + 1929;
				line.clear();
				line.append (projects[prj]);                 
				line.append (",");
				line.append (QString::number(real_year));     
				line.append (",");
				line.append (QString::number(day + 1));       
				line.append (",");
				line.append (QString::number(fin[prj].daily_flow_kcfs[year][day], 'f', 2)); 
				line.append (",");
				line.append (QString::number(fin[prj].day_spill_prop[year][day], 'f', 4)); 
				line.append (",");
				line.append (QString::number(fin[prj].night_spill_prop[year][day], 'f', 4)); 
				line.append (",");
				line.append (QString::number(fin[prj].monthly_elev_feet[year][day], 'f', 1)); 
				line.append (",");
				line.append (QString::number(fin[prj].turb_q_daily[year][day], 'f', 8)); 
				line.append (",");
				line.append (QString::number(fin[prj].spill_prob[year][day], 'f', 1)); 
                line.append (" \n");
                if (standardout)
                    std::cout << line.toUtf8().data();
                else
                    outFile.write (line.toUtf8());
			}
		}
	}
	outFile.close();

	return 1;
}
/* 1. process modulated flow, spill, elevation data from HYDSIM/Paulsen 
 *    SAS process.
 *    a. parse HYDSIM modulated SAS file
 *    b. produce "flow.archive" structured file for COMPASS  */
int write_flow_archive (FINAL_PROJECT *fin)
{
	//write header

	// format each line

	// write line
	return 0;
}
