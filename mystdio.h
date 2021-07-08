#ifndef INCLUDED_MYSTDIO_H
#define INCLUDED_MYSTDIO_H

#include <QTextStream>

QTextStream &operator<<(QTextStream &a,const QVariant &b);

extern QTextStream cin;
extern QTextStream cerr;
extern QTextStream cout;
#define endl Qt::endl

#endif // INCLUDED_MYSTDIO_H
