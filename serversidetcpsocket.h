#ifndef INCLUDED_SERVERSIDETCPSOCKET_H
#define INCLUDED_SERVERSIDETCPSOCKET_H

#include <QSslSocket>

//EXTERNAL CODE Qt creator can make a skeleton class, fyi
class tServersideTcpSocket : public QSslSocket
{
    Q_OBJECT

public:
    explicit tServersideTcpSocket(QObject *parent = nullptr);
private slots:
    void cleartextReadyRead();
    void clientConnected();
    void clientsideEncrypted();
    void serversideEncrypted();
private:
    QSslSocket *clientSide;
};

#endif // INCLUDED_SERVERSIDETCPSOCKET_H
