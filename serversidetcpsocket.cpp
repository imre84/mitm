#include "serversidetcpsocket.h"
#include "sslstuff.h"
#include "mystdio.h"
#include "qthelper.h"
#include "proxyserversocket.h"

#include <QFileInfo>
#include <QDir>

tServersideTcpSocket::tServersideTcpSocket(const QString &workdir, const QString &cadir, QObject *parent) : QSslSocket(parent), m_clientSide(0),m_port(0), m_workdir(workdir), m_cadir(cadir)
{
    connect(this,&QIODevice::readyRead,this,&tServersideTcpSocket::cleartextReadyRead);
}

tServersideTcpSocket::~tServersideTcpSocket()
{
    if(m_port)
    {
        qCritical()<<"Socket to "<<m_host<<':'<<m_port<<" gets closed";
    }
}

#define CODE(x) do{ x } while(0)
#define ERRORIF(l) CODE(if(l){deleteLater();return;})

void tServersideTcpSocket::cleartextReadyRead()
{
    //https://datatracker.ietf.org/doc/html/rfc7231#section-4.3.6
    QByteArray buf=readAll();
    ERRORIF(buf.left(8)!="CONNECT ");
    ERRORIF(buf[buf.size()-1]!='\n');
    if(buf[buf.size()-2]!='\n')
    {
        ERRORIF(buf[buf.size()-2]!='\r');
        ERRORIF(buf[buf.size()-3]!='\n');
        ERRORIF(buf[buf.size()-4]!='\r');
    }
    else
        ERRORIF(buf[buf.size()-2]!='\n');
    buf=buf.mid(8);
    while(buf[0]==' ') buf=buf.mid(1);
    buf=buf.mid(0,buf.indexOf(' '));
    int pos=buf.indexOf(':');
    ERRORIF(pos<0);
    m_host=buf.mid(0,pos);
    bool l;
    uint xport=buf.mid(pos+1).toUInt(&l);
    ERRORIF(!l);
    ERRORIF(!xport);
    ERRORIF(xport>=65536);
    m_port=xport;
    qCritical()<<"Initializing CONNECTion to "<<m_host<<':'<<m_port;
    disconnect(this        , &QIODevice::readyRead                                           , this, &tServersideTcpSocket::cleartextReadyRead );
    m_clientSide=new QSslSocket(this);
       connect(m_clientSide, &QAbstractSocket::connected                                     , this, &tServersideTcpSocket::clientConnected    );
    m_clientSide->connectToHost(m_host,m_port);

       connect(this        , &QSslSocket::peerVerifyError                                    , this, &QObject::deleteLater                     );
    //1 line of EXTERNAL CODE from https://doc.qt.io/qt-5/qsslsocket.html#sslErrors-1 follows:
       connect(this        ,  QOverload<const QList<QSslError> &>::of(&QSslSocket::sslErrors), this, &QObject::deleteLater                     );
       connect(m_clientSide, &QSslSocket::peerVerifyError                                    , this, &QObject::deleteLater                     );
    //1 line of EXTERNAL CODE from https://doc.qt.io/qt-5/qsslsocket.html#sslErrors-1 follows:
       connect(m_clientSide,  QOverload<const QList<QSslError> &>::of(&QSslSocket::sslErrors), this, &QObject::deleteLater                     );
       connect(this        , &QIODevice::aboutToClose                                        , this, &QObject::deleteLater                     );
       connect(m_clientSide, &QIODevice::aboutToClose                                        , this, &QObject::deleteLater                     );
       connect(this        , &QAbstractSocket::errorOccurred                                 , this, &tServersideTcpSocket::onError            );
       connect(m_clientSide, &QAbstractSocket::errorOccurred                                 , this, &tServersideTcpSocket::onError            );
       connect(this        , &QAbstractSocket::disconnected                                  , this, &QObject::deleteLater                     );
       connect(m_clientSide, &QAbstractSocket::disconnected                                  , this, &QObject::deleteLater                     );

}

void tServersideTcpSocket::clientConnected()
{
    qCritical()<<"Downstream TCP connection for "<<m_host<<':'<<m_port<<" is ready, initiating downstream SSL handshake";
    disconnect(m_clientSide, &QAbstractSocket::connected                                     , this, &tServersideTcpSocket::clientConnected    );
       connect(m_clientSide, &QSslSocket::encrypted                                          , this, &tServersideTcpSocket::clientsideEncrypted);
    m_clientSide->startClientEncryption();
}

QByteArray contents(const QString &fn)
{
    QFile f(fn);
    if(!f.open(QIODevice::OpenModeFlag::ReadOnly))
        return QByteArray();
    return f.readAll();
}

void tServersideTcpSocket::clientsideEncrypted()
{
    qCritical()<<"Downstream TCP connection for "<<m_host<<':'<<m_port<<" is now ssl encrypted, initiating upstream SSL handshake";
    disconnect(m_clientSide, &QSslSocket::encrypted                                          , this, &tServersideTcpSocket::clientsideEncrypted);
       connect(this        , &QSslSocket::encrypted                                          , this, &tServersideTcpSocket::serversideEncrypted);
    QStringList w=m_host.split('.');
    std::reverse(w.begin(),w.end());
    QString curdir=m_workdir;
    for(const QString &s:w)
    {
        curdir+="/";
        curdir+=s;
        if(!QFileInfo::exists(curdir))
            QDir().mkdir(curdir);
    }

    if(!QFileInfo::exists(curdir+"/cert"))
    {
        long serial=static_cast<tProxyServerSocket *>(parent())->getSerial();
        genkey(m_host,m_cadir,curdir+"/cert",curdir+"/key",serial);
    }

    this->setPrivateKey(curdir+"/key");
    QList<QSslCertificate> chain;
    chain.push_back(QSslCertificate(contents(curdir+"/cert")));
    chain.push_back(QSslCertificate(contents(m_cadir+"/cert")));
    this->setLocalCertificateChain(chain);
    this->write("HTTP/1.1 200 OK sent from mitm server\n\n");
    this->flush();
    this->startServerEncryption();
}

void tServersideTcpSocket::serversideEncrypted()
{
    qCritical()<<"Upstream TCP connection for "<<m_host<<':'<<m_port<<" is now encrypted";
    disconnect(this        , &QSslSocket::encrypted                                          , this, &tServersideTcpSocket::serversideEncrypted);
       connect(this        , &QIODevice::readyRead                                           , this, &tServersideTcpSocket::sslDataReceived    );
       connect(m_clientSide, &QIODevice::readyRead                                           , this, &tServersideTcpSocket::sslDataReceived    );
//    if(this        ->waitForReadyRead(1))
//                                          sslDataReceived2(*this        , *m_clientSide);

//    if(m_clientSide->waitForReadyRead(1))
//                                          sslDataReceived2(*m_clientSide, *this        );
}

void tServersideTcpSocket::sslDataReceived()
{
    sslDataReceived1(sender());
}

void tServersideTcpSocket::onError(QAbstractSocket::SocketError socketError)
{
    if(socketError==QAbstractSocket::SocketTimeoutError)
        return;

    if(m_port>0)
        qCritical()<<"Socket error for "<<m_host<<':'<<m_port<<" "<<enumToString(socketError)<<errorString();
    else
        qCritical()<<"Socket error "                              <<enumToString(socketError)<<errorString();

    deleteLater();
}

void tServersideTcpSocket::sslDataReceived1(QObject *sndr)
{
    QSslSocket *from=static_cast<QSslSocket *>(sndr);
    QSslSocket *to;
    if(from==static_cast<QSslSocket *>(this))
        to=this->m_clientSide;
    else
        to=this;

    sslDataReceived2(*from,*to);
}

void tServersideTcpSocket::sslDataReceived2(QSslSocket &from, QSslSocket &to)
{
    QByteArray buf=from.readAll();
    qCritical()<<buf;

    to.write(buf);
}
