#ifndef PP_FILE_IO_UTILS_H
#define PP_FILE_IO_UTILS_H

#include <QFile>
#include <QString>
#include <QStringList>
#include <QDir>
#include <QList>

#define MAXLINE  512
#ifdef WINVER
#define PATHSEP  QString("\\")
#else
#define PATHSEP QString ("/")
#endif

QStringList readLine (QFile *infile, QChar delim);

int writeLine (QFile *outfile, QChar delim, QStringList line);

QString checkFile (QString);
QString checkFile (QString filename, QChar type);

int writeCompassFileHeader (QString type, QFile *outfile);

#endif // PP_FILE_IO_UTILS_H
