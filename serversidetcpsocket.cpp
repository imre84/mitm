#include "serversidetcpsocket.h"

tServersideTcpSocket::tServersideTcpSocket(QObject *parent) : QSslSocket(parent), clientSide(0)
{
    connect(this,&QIODevice::readyRead,this,&tServersideTcpSocket::cleartextReadyRead);
}

#define CODE(x) do{ x } while(0)
#define ERRORIF(l) CODE(if(l){deleteLater();return;})

void tServersideTcpSocket::cleartextReadyRead()
{
    //https://datatracker.ietf.org/doc/html/rfc7231#section-4.3.6
    QByteArray buf=readAll();
    ERRORIF(buf.left(8)!="CONNECT ");
    buf=buf.mid(8);
    while(buf[0]==' ') buf=buf.mid(1);
    buf=buf.mid(0,buf.indexOf(' '));
    int pos=buf.indexOf(':');
    ERRORIF(pos<0);
    QString shost(buf.mid(0,pos));
    bool l;
    uint xport=buf.mid(pos+1).toUInt(&l);
    ERRORIF(!l);
    ERRORIF(!xport);
    ERRORIF(xport>=65536);
    quint16 port=xport;
    qDebug()<<"Host:"<<shost<<"Port:"<<port;
    disconnect(this      , &QIODevice::readyRead                                           , this, &tServersideTcpSocket::cleartextReadyRead );
    clientSide=new QSslSocket(this);
       connect(clientSide, &QAbstractSocket::connected                                     , this, &tServersideTcpSocket::clientConnected    );
    clientSide->connectToHost(shost,port);

       connect(this      , &QSslSocket::peerVerifyError                                    , this, &QObject::deleteLater                     );
    //1 line of EXTERNAL CODE from https://doc.qt.io/qt-5/qsslsocket.html#sslErrors-1 follows:
       connect(this      ,  QOverload<const QList<QSslError> &>::of(&QSslSocket::sslErrors), this, &QObject::deleteLater                     );
       connect(clientSide, &QSslSocket::peerVerifyError                                    , this, &QObject::deleteLater                     );
    //1 line of EXTERNAL CODE from https://doc.qt.io/qt-5/qsslsocket.html#sslErrors-1 follows:
       connect(clientSide,  QOverload<const QList<QSslError> &>::of(&QSslSocket::sslErrors), this, &QObject::deleteLater                     );
       connect(this      , &QIODevice::aboutToClose                                        , this, &QObject::deleteLater                     );
       connect(clientSide, &QIODevice::aboutToClose                                        , this, &QObject::deleteLater                     );
}

void tServersideTcpSocket::clientConnected()
{
    disconnect(clientSide, &QAbstractSocket::connected                                     , this, &tServersideTcpSocket::clientConnected    );
       connect(clientSide, &QSslSocket::modeChanged                                        , this, &tServersideTcpSocket::clientsideEncrypted);
    clientSide->startClientEncryption();
}

void tServersideTcpSocket::clientsideEncrypted()
{
    disconnect(clientSide, &QSslSocket::modeChanged                                        , this, &tServersideTcpSocket::clientsideEncrypted);
       connect(this      , &QSslSocket::modeChanged                                        , this, &tServersideTcpSocket::serversideEncrypted);
    this->startServerEncryption();
}

void tServersideTcpSocket::serversideEncrypted()
{
    disconnect(this      , &QSslSocket::modeChanged                                        , this, &tServersideTcpSocket::serversideEncrypted);
}
