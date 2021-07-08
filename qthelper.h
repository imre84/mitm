#ifndef INCLUDED_QTHELPER_H
#define INCLUDED_QTHELPER_H

#include <QMetaEnum>

//EXTERNAL CODE https://stackoverflow.com/questions/34281682/how-to-convert-enum-to-qstring
template<class myEnum>
const char *enumToString(const myEnum x)
{
    QMetaEnum z=QMetaEnum::fromType<myEnum>();
    return z.valueToKey(x);
}

#endif // INCLUDED_QTHELPER_H
