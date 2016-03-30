#include "pp_fileIoUtils.h"

#include <QFileInfo>
#include <QTime>
#ifdef WIN32
#include <Winsock2.h>
#else
#include <pwd.h>
#include <unistd.h>
#endif

QStringList readLine (QFile *infile, QChar delim)
{
	QString strline;
	QStringList line;
	if (infile->isOpen() || infile->open (QIODevice::ReadOnly))
	{
		strline.append (infile->readLine (MAXLINE));
        line = strline.split (QChar(delim), QString::SkipEmptyParts);
	}
	return line;
}


int writeLine (QFile *outfile, QChar delim, QStringList line)
{
	qint64 i;
	QString strline;
	if (outfile->open (QIODevice::WriteOnly))
	{
		strline.append (line[0]);
		for (i = 1; i < line.count(); i++)
		{
			strline.append (delim);//(" ");
			strline.append (line[i]);
		}
		strline.append ("\n");
        i = outfile->write (strline.toUtf8().data(), MAXLINE);
	}
	else 
	{
		i = -1;
	}

	return i;
}


QString checkFile (QString filename)
{
	QString error ("");
	QFileInfo fileinfo (filename);
	if (!fileinfo.exists ())
		error = QString ("File '") + filename + QString ("' does not exist\n");
	else if (!fileinfo.isReadable())
		error = QString ("File '") + filename + QString ("' is not readable\n");
	else if (!fileinfo.isWritable())
		error = QString ("File '") + filename + QString ("' is not writable\n");

	return error;
}

QString checkFile (QString filename, QChar type)
{
	QString error ("");
	QFileInfo fileinfo (filename);
	if (type == 'x' || type == 'r')
	{
		if (!fileinfo.exists())
			error = QString ("File '") + filename + QString ("' does not exist");
	}
	if (type == 'w')
	{
		if (fileinfo.exists() && !fileinfo.isWritable())
			error = QString ("File '") + filename + QString ("' is not writable");
	}
	if (type == 'r' && error.isEmpty())
	{
		if (!fileinfo.isReadable())
			error = QString ("File '") + filename + QString ("' is not readable");
	}

	return error;
}


int writeCompassFileHeader (QString type, QFile *outfile)
{
	int retval = 0;
	QString line;
	if (outfile->isOpen() || outfile->open (QIODevice::WriteOnly))
	{
/*        QString username, fullname, hostname;
#ifdef WIN32
		char *idValue;
		size_t len;
		char hostname_buffer[256];

        _dupenv_s(&idValue, &len, "USER");
		if (idValue == NULL)
			_dupenv_s (&idValue, &len, "USERNAME");
		username = QString (idValue);
		free (idValue);

//		_dupenv_s(&hostname_buffer, &len, "COMPUTERNAME");
#else
		char hostname_buffer[256];
//		extern int gethostname ();
//		extern uid_t getuid ();
		struct passwd *pwd;
		pwd = getpwuid (getuid ());
		username = QString (pwd->pw_name);
//		fullname = QString (pwd->gecos);
#endif
		gethostname (hostname_buffer, sizeof(hostname_buffer));
		hostname = QString (hostname_buffer);
*/
		line.clear();
		line.append ("#============================================================================#\n");
		if (type.contains ("ops", Qt::CaseInsensitive))
		{
			line.append ("# COMPASS Operations File                    COMPASS Preprocessor v1.0.0     #\n");
			line.append ("# Written from Ingres observed Flow and Spill.                               #\n");
		}
		else if (type.contains ("ctrl", Qt::CaseInsensitive))
		{
			line.append ("# COMPASS Control File                       COMPASS Preprocessor v1.0.0     #\n");
		}
		else if (type.contains ("riv", Qt::CaseInsensitive))
		{
			line.append ("# COMPASS River Environment File             COMPASS Preprocessor v1.0.0     #\n");
			line.append ("# Written from HYDROSIM flow and rules, Paulsen Temperature Model,           #\n");
			line.append ("# and Daily historical mean flow and temperature from DART.                  #\n");
		}
		else if (type.contains ("trans", Qt::CaseInsensitive))
		{
			line.append ("# COMPASS Transport File                     COMPASS Preprocessor v1.0.0     #\n");
		}
		else if (type.contains ("dam", Qt::CaseInsensitive))
		{
			line.append ("# COMPASS Dam Parameters File                COMPASS Preprocessor v1.0.0     #\n");
		}
		else if (type.contains ("rls", Qt::CaseInsensitive))
		{
			line.append ("# COMPASS Release File                       COMPASS Preprocessor v1.0.0     #\n");
		}
		else if (type.contains ("clb", Qt::CaseInsensitive))
		{
			line.append ("# COMPASS Calibration File                   COMPASS Preprocessor v1.0.0     #\n");
		}
        outfile->write (line.toUtf8());
		line.clear();
		line.append ("#                                                                            #\n");
        outfile->write (line.toUtf8());
		line.clear();
		line.append ("# Written on ");
		line.append (QDate::currentDate().toString());
		line.append (" ");
		line.append (QTime::currentTime().toString());
		while (line.count() < 77)
			line.append (" ");
		line.append ("#\n");
        outfile->write (line.toUtf8());
		line.clear();
		line.append ("#	from unknown host");
		while (line.count() < 77)
			line.append (" ");
		line.append ("#\n");
		line.append ("#============================================================================#\n");
		line.append ("\nversion 9\n\n");
        outfile->write (line.toUtf8());
		line.clear();
		retval = 0;
	}
	else
		retval = -1;
	return retval;
}

