#ifndef INCLUDED_SERVERSIDETCPSOCKET_H
#define INCLUDED_SERVERSIDETCPSOCKET_H

#include <QSslSocket>

//EXTERNAL CODE Qt creator can make a skeleton class, fyi
class tServersideTcpSocket : public QSslSocket
{
    Q_OBJECT

public:
    explicit tServersideTcpSocket(const QString &workdir,const QString &cadir, QObject *parent = nullptr);
    ~tServersideTcpSocket();
private slots:
    void cleartextReadyRead();
    void clientConnected();
    void clientsideEncrypted();
    void serversideEncrypted();
    void sslDataReceived();
    void onError(SocketError socketError);

private:
    void sslDataReceived1(QObject *);
    void sslDataReceived2(QSslSocket &from,QSslSocket &to);

private:
    QSslSocket *m_clientSide;
    QString m_host;
    quint16 m_port;
    const QString m_workdir,m_cadir;
};

#endif // INCLUDED_SERVERSIDETCPSOCKET_H
