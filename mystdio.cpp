#include "mystdio.h"
#include <QVariant>
#include <QTextStream>

QTextStream &operator<<(QTextStream &a,const QVariant &b)
{
    a<<b.toString();
    return a;
}

QTextStream cin(stdin);
QTextStream cerr(stderr);
QTextStream cout(stdout);
