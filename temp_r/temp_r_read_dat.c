#include <stdio.h>
#include <string.h>
#include <stdlib.h>
//#include <ctype.h>

#include <iostream>
#include <QString>
#include <QStringList>

char *get_word(char **);
int read_word(char *, int *, int *);
char *search_name_assoc(char *);
int isfloat(char *);

#define MAXLINE 500

/* the USEMEANFLOW flag indicates that no flow prediction is available
 * for this station and so the historic mean flows should be substituted...
 * This parameter needs to be defined in   temp_r.c   also.                  */
#define USEMEANFLOW 1001

/*
 * if you add an abbreviation that is not 3 characters long,
 * then modify the strncmp() in search_name_assoc() accordingly.
 * Added 8-digit station names... so changed 3 to 8.  DS4/9/02
 */
QStringList dam_abbvs;
QStringList dam_names;
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

char	*name_assoc[] = 
{
	"CHJ",  "Chief Joseph Dam",
	"IHR",  "Ice Harbor Dam", 
	"LMN",  "Lower Monumental Dam", 
	"LGS",  "Little Goose Dam", 
	"BON",  "Bonneville Dam", 
	"JDA",  "John Day Dam", 
	"MCN",  "McNary Dam", 
	"WAN",  "Wanapum Dam", 
	"RRH",  "Rocky Reach Dam", 
	"WEL",  "Wells Dam",
	"RIS",  "Rock Island Dam",
	"PRD",  "Priest Rapids Dam",
	"TDA",  "The Dalles Dam",
	"LWG",  "Lower Granite Dam",
	"DWR",  "Dworshak Dam",       /* ??? */
	"13342500", "NotInFlowpred",   /*signal that station is not in flowpred file */ 
	"13341050", "NotInFlowpred",   /*and that mean flows are to be used instead  */ 
	"13334300", "NotInFlowpred",
	"13340600", "NotInFlowpred", 		
	NULL
};

QString findDamName (QString damAbbrev)
{
	int	index;

	index = dam_abbvs.indexOf (dam_abbrev);
	return dam_names[index];
}
char * search_name_assoc (char * dam_abbrev)
{
	int	index;

	index = 0;

	while (name_assoc[index])
	{
		if (strncmp (dam_abbrev, name_assoc[index], 8) == 0)  /* 8  --was 3 */
		{
			return name_assoc[index + 1];
		}
		index++;
	}

	return NULL;
}

int readFlowPredFile (QFile * flowpredfile, QString damAbbv, float *flow_predict)
{
	QString damName = findDamName (damAbbv);
	if (damName == QString::null())

}

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
	if ( strcmp(dam_name,"NotInFlowpred") == 0 ) {
	    return USEMEANFLOW;
	}

	/* search for the start of data for this dam */
	flag = 0;
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
	count = 0;
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
					word = get_word (&buf_ptr); 
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
}


/*
 * If 'word' is of the form "[N:M]", set n1 = N and n2 = M and return 1;
 * If 'word' is just a number, return 2;
 * Return -1 upon error;
 */
int read_word (char *word, int *n1, int *n2)
{
	if (word == NULL)
		return -1;

	if (*word == '[')
	{
		if (sscanf (word, "[%i:%i]", n1, n2) != 2 || *n1 > *n2)
		{
			return -1;
		}
		return 1;
	}
	else
	{
		if (isfloat (word) < 0)
			return -1;
		else
			return 2;
	}
}

/*
 * This function OK's a string representing a floating point number 
 * if it starts and ends with a digit, and has 0 or 1 periods within it.
 */
int isfloat (QString word)
{
	int	index,
		period_cnt;

	for (index = period_cnt = 0 ; *(word + index) != NULL ; index++)
	{
		if (! isdigit((int)*(word + index)))
		{
			if (*(word + index) == '.' && period_cnt == 0)
				period_cnt = 1;
			else
				return -1;
		}
	}
	if (index == 0 || *(word + index - 1) == '.' || *word == '.')
		return -1;

	return 1;	
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
