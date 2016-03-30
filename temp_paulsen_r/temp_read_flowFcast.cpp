//#include <stdio.h>
//#include <string.h>
//#include <stdlib.h>
//#include <ctype.h>

#include <iostream>

using namespace std;

#include <QFile>
#include <QString>
#include <QList>
#include <QStringList>

bool isfloat (QString word);
int readRange (QString token, int *n1, int *n2);


#define MAXLINE      500

/* the USEMEANFLOW flag indicates that no flow prediction is available
 * for this station and so the historic mean flows should be substituted...
 * This parameter needs to be defined in  temp_r.c  and  temp_paulsen_r.c  also.                  */
#define USEMEANFLOW  1001

/*
 * if you add an abbreviation that is not 3 characters long,
 * then modify the strncmp() in search_name_assoc() accordingly.
 * Added 8-digit station names... so changed 3 to 8.  DS4/9/02
 */
QStringList dam_abbvs;
QStringList dam_names;
void setupDamNames()
{
	dam_abbvs.append (QString("CHJ")); dam_names.append (QString("Chief Joseph Dam"));
	dam_abbvs.append ("IHR"); dam_names.append ("Ice Harbor Dam");
	dam_abbvs.append ("LMN"); dam_names.append ("Lower Monumental Dam");
	dam_abbvs.append ("LGS"); dam_names.append ("Little Goose Dam");
	dam_abbvs.append ("BON"); dam_names.append ("Bonneville Dam");
	dam_abbvs.append ("JDA"); dam_names.append ("John Day Dam");
	dam_abbvs.append ("MCN"); dam_names.append ("McNary Dam");
	dam_abbvs.append ("WAN"); dam_names.append ("Wanapum Dam");
	dam_abbvs.append ("RRH"); dam_names.append ("Rocky Reach Dam");
	dam_abbvs.append ("WEL"); dam_names.append ("Wells Dam");
	dam_abbvs.append ("RIS"); dam_names.append ("Rock Island Dam");
	dam_abbvs.append ("PRD"); dam_names.append ("Priest Rapids Dam");
	dam_abbvs.append ("TDA"); dam_names.append ("The Dalles Dam");
	dam_abbvs.append ("LWG"); dam_names.append ("Lower Granite Dam");
	dam_abbvs.append ("DWR"); dam_names.append ("Dworshak Dam");
	dam_abbvs.append ("ANA"); dam_names.append ("Anatone USGS");
	dam_abbvs.append ("PEC"); dam_names.append ("Peck USGS");
	dam_abbvs.append ("13342500"); dam_names.append ("NotInFlowpred");
	dam_abbvs.append ("13341050"); dam_names.append ("NotInFlowpred");
	dam_abbvs.append ("13334300"); dam_names.append ("NotInFlowpred");
	dam_abbvs.append ("13340600"); dam_names.append ("NotInFlowpred");
}

QString *findDamName (QString damAbbrev)
{	
	QString *name = NULL;
	if (dam_abbvs.isEmpty())
		setupDamNames();

	int	index = dam_abbvs.indexOf (damAbbrev);
	if (index > -1)
		name = (QString *) &(dam_names[index]);
	return name;
}

int readFlowPredFile (QFile * flowpredfile, QString damAbbv, float *flow_predict)
{
	bool okay = true;
	int retval = 0, flag = 0, count = 0, n = 0, m = 0;
//	char line[MAXLINE];
	QString newline;
	QStringList tokens;
	float flow;
	QString damName = *(findDamName (damAbbv));
	if (damName.isEmpty())
	{
        cerr << damAbbv.toUtf8().data() << " is not a valid dam abbreviation." << endl;
	}
	/* If station (dam) is not listed in flowpred file, this return indicates
	   that the mean flow is to be substituted  DS 4/02*/
	else if (damName.contains("NotInFlowpred"))
	{
	    retval = USEMEANFLOW;
	}
	/* search for the start of data for this dam */
	else
	{
		flag = 0;
        if (!flowpredfile->isOpen())
            flowpredfile->open(QIODevice::ReadOnly);
		while (!flowpredfile->atEnd())
		{
			newline = QString (flowpredfile->readLine (MAXLINE));
			if (newline.contains (damName))
			{
				flag = 1;
				break;
			}
		}
		if (flag)
		{
			count = 0;
			while (retval >= 0)
			{
				tokens = newline.split (' ', QString::SkipEmptyParts);
				for (int i = 0; i < tokens.count(); i++)
				{
					flow = tokens[i].toFloat (&okay);
					if (okay)
					{
						flow_predict[count++] = flow;
					}
					else if (tokens[i].contains ('['))
					{
						retval = readRange (tokens[i], &n, &m);
						if (retval < 0 || n < count)
						{
                            cerr << "Error in input file " << flowpredfile->fileName().toUtf8().data() << endl;
							retval = -1;
						}
						flow = tokens[++i].toFloat (&okay);
						if (okay)
						{
							for (int j = n; j <= m; j++)
								flow_predict[j] = flow;
							count = m + 1;
						}
						else
						{
                            cerr << "Error in input file " << flowpredfile->fileName().toUtf8().data() << endl;
							retval = -1;
						}
					}
				}
				if (flowpredfile->atEnd())
					break;
				newline = QString (flowpredfile->readLine (MAXLINE));
				if ((newline.startsWith ("Flow")))
					break;
			}
			retval = count - 1;
		}
		else
		{
            cerr << "Error in input file " << flowpredfile->fileName().toUtf8().data();
            cerr << " -- No data for dam " << damName.toUtf8().data() << endl;
			retval = -1;
		}
	}
	return retval;
}

int readRange (QString token, int *n1, int *n2)
{
	bool okay;
	int retval = 0, n, m;
	token.replace ('[', ' ');
	token.replace (']', ' ');
	token.replace (':', ' ');
	QStringList tokens (token.split (' ', QString::SkipEmptyParts));
	n = tokens[0].toInt(&okay);
	if (!okay)
	{
		retval = -1;
	}
	else
	{
		n1 = &n;
		m = tokens[1].toInt(&okay);
		if (!okay)
			retval = -1;
		else 
			n2 = &m;
	}
	return retval;
}

/*
 * If 'word' is of the form "[N:M]", set n1 = N and n2 = M and return 1;
 * If 'word' is just a number, return 2;
 * Return -1 upon error;
 */
//int read_word (QString word, int *n1, int *n2)

/*
 * This function OK's a string representing a floating point number 
 * if it starts and ends with a digit, and has 0 or 1 periods within it.
 */
bool isfloat (QString word)
{
	bool okay;
	word.toFloat(&okay);
	return okay;
}

