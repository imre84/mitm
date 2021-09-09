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

//EXTERNAL CODE https://www.kdab.com/qt-range-based-for-loops-and-structured-bindings/
template <typename T>
class asKeyValueRange
{
public:
    asKeyValueRange(T &data):m_data{data}{}
    auto begin() { return m_data.keyValueBegin(); }
    auto end() { return m_data.keyValueEnd(); }
private:
    T &m_data;
};

#endif // INCLUDED_QTHELPER_H
