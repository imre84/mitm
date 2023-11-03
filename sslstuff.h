#ifndef INCLUDED_SSLSTUFF_H
#define INCLUDED_SSLSTUFF_H

class QString;

bool generateX509(                                  const QString &certFileName, const QString &keyFileName,              long daysValid=10*365, unsigned int length_in_bits=2048);
bool genkey(const QString &fqdn, const QString &ca, const QString &certFileName, const QString &keyFileName, long serial, long daysValid=10*365, unsigned int length_in_bits=2048);

#endif // INCLUDED_SSLSTUFF_H
