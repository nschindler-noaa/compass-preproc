//#include <stdio.h>
//#include <string.h>
//#include <stdlib.h>
//#include <ctype.h>

#include <iostream>
#include <QFile>
#include <QString>
#include <QStringList>

char *get_word(char **);
int read_word(char *, int *, int *);
QString search_name_assoc(QString);
bool isfloat(QString);

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
QStringList dam_abbvs();
QStringList dam_names();
dam_abbvs.append ("CHJ"); dam_names.append ("Chief Joseph Dam");
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
dam_abbvs.append ("13342500"); dam_names.append ("NotInFlowpred");
dam_abbvs.append ("13341050"); dam_names.append ("NotInFlowpred");
dam_abbvs.append ("13334300"); dam_names.append ("NotInFlowpred");
dam_abbvs.append ("13340600"); dam_names.append ("NotInFlowpred");


QString findDamName (QString damAbbrev)
{
	QString name();
	int	index = dam_abbvs.indexOf (dam_abbrev);
	if (index > -1)
		name = dam_names[index];
	return name;
}

int readFlowPredFile (QFile * flowpredfile, QString damAbbv, float *flow_predict)
{
	bool okay;
	int retval, flag;
	char line[MAXLINE];
	QString newline();
	QStringList tokens();
	float flow;
	QString damName = findDamName (damAbbv);
	if (damName.isEmpty())
	{
		cerr << damAbbrev.toAscii().data() << " is not a valid dam abbreviation." << endl;
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
			while (!flowpredfile->atEnd())
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
						retval = readRange (tokens[i], n, m);
						if (retval < 0 || n < count)
						{
							cerr << "Error in input file " << flowpredfile->fileName().toAscii().data() << endl;
							retval = -1;
						}
						flow = tokens[++i].toFloat (&okay);
						if (okay)
							for (int j = n, j <= m; j++, count++)
								flow_predict[j] = flow;
						else
						{
							cerr << "Error in input file " << flowpredfile->fileName().toAscii().data() << endl;
							retval = -1;
						}
					}
				}
				newline = QString (flowpredfile->readLine (MAXLINE));
				if (retval < 0 || newline.startsWith ("Flow"))
					break;
			} 
		}
		else
		{
			cerr << "Error in input file " << flowpredfile->fileName().toAscii().data();
			cerr << ". No data for dam " << damName.toAscii().data() << endl;
			retval = -1;
		}
	}
	return retval;
}
/*
int read_flowpred (FILE *flowpred, char *dam, float *flow_pred)
{
	char buf[MAXLINE],
		 *dam_name,
		 *buf_ptr,
		 *word;
	int	 flag,
		 tmp,
		 count,
		 n,
		 m;

	dam_name = search_name_assoc(dam);
	if (dam_name == NULL)
	{
		fprintf (stderr, 
					"I can not recognize %s as a valid dam abbreviation\n", 
					dam
		);
		exit (1);
	}

	/* If station (dam) is not listed in flowpred file, this return indicates
	   that the mean flow is to be substituted  DS 4/02*/
/*	if ( strcmp(dam_name,"NotInFlowpred") == 0 ) {
	    return USEMEANFLOW;
	}

	/* search for the start of data for this dam */
/*	flag = 0;
	while(fgets(buf, MAXLINE, flowpred))
	{
		if(strstr(buf, dam_name) != NULL)
		{
			flag = 1;
			break;
		}
	}
	if(flag == 0)
	{
		fprintf(stderr, "Could not locate the data for %s in flowpred file.\n",	dam_name);
		exit(1);
	}

	/* read in the data */
/*	count = 0;
	buf_ptr = strpbrk ((const char *) buf, "[]0123456789");
	for (;;)
	{
		word = strchr(buf, '\n');
		if (word != NULL)
			word = NULL;
		while ((word = get_word (&buf_ptr)) != NULL)
		{
			switch (read_word( word, &n, &m))
			{
				case 1: /* [N:M] */
/*					word = get_word (&buf_ptr); 
					if (read_word (word, &tmp, &tmp) != 2)
					{
						fprintf (stderr, 
							"Bad format, no number following [%i:%i]: '%s'\n", 
							n, 
							m, 
							buf
						);
						exit (1);
					}
					while (n++ <= m)
					{
						flow_pred[count++] = atof(word);
					}
					break;
				case 2:
					flow_pred[count++] = atof(word);
					break;
				default:
					fprintf (stderr, "Error reading input: '%s'\n", buf);
					exit (1);
			}
		}
		if (! fgets(buf, MAXLINE, flowpred))
		{
			break;
		}
		if(strncmp(buf, "Flow", 4) == 0)
		{
			break;
		}
		buf_ptr = buf;
	}

	return count - 1;
}*/

int readRange (QString token, int *n1, int *n2)
{
	int retval = 0;
	token.replace ('[', ' ');
	token.replace (']', ' ');
	token.replace (':', ' ');
	QStringList tokens (token.split (' ', QString::SkipEmptyParts));
	n1 = tokens[0].toInt(&okay);
	if (!okay)
	{
		retval = -1;
	}
	else
	{
		n2 = tokens[1].toInt(&okay);
		if (!okay)
			retval = -1;
	}
}

/*
 * If 'word' is of the form "[N:M]", set n1 = N and n2 = M and return 1;
 * If 'word' is just a number, return 2;
 * Return -1 upon error;
 */
/*int read_word (QString word, int *n1, int *n2)
{
	int retval = 1;
	bool okay;
	if (word.isEmpty())
		retval = -1;

	else if (word.contains ('['))
	{
		word.replace ('[', ' ');
		word.replace (']', ' ');
		word.replace (':', ' ');
		QStringList tokens (word.split (' ', QString::SkipEmptyParts));
		n1 = tokens[0].toInt(&okay);
		if (!okay)
		{
			retval = -1;
		}
		else
		{
			n2 = tokens[1].toInt(&okay);
			if (!okay)
				retval = -1;
		}
	}
	else
	{
		if (isfloat (word) < 0)
			retval = -1;
		else
			retval = 2;
	}
	return retval;
}

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

/*
 * return a pointer to the first word (sequence of non-space/tab 
 * characters), NULL terminate that word, make the passed pointer
 * to a pointer 'string' point to the character directly after the
 * found NULL terminated word.   return NULL if no word is found.
 */
char *get_word (char **string)
{
	char	*ptr,
			*retval;

	/* suck up leading whitespace */
	for (ptr = *string ; *ptr == ' ' || *ptr == '\t' || *ptr == '\n' ; ptr++)
		;

	if (*ptr == NULL)
		return NULL;

	retval = ptr;

	/* search for end of this word */
	for ( ; *ptr != ' ' && *ptr != '\t' && *ptr != '\n' && *ptr != NULL ; ptr++)
		;

	if (*ptr == NULL)
		*string = NULL;
	else
	{
		*ptr = NULL;
		*string = ptr + 1;
	}

	return retval;
}
