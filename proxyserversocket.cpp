#include "proxyserversocket.h"
#include "serversidetcpsocket.h"

#include <QSslSocket>

tProxyServerSocket::tProxyServerSocket(const QString &workdir, const QString &cadir, long serial, QObject *parent) : QTcpServer(parent),m_workdir(workdir),m_cadir(cadir),m_serial(serial)
{
    connect(this,&QTcpServer::newConnection,this,&tProxyServerSocket::onNewConnection);
}

long tProxyServerSocket::getSerial()
{
    return ++m_serial;
}

void tProxyServerSocket::onNewConnection()
{
    while (hasPendingConnections())
    {
        tServersideTcpSocket *sock=qobject_cast<tServersideTcpSocket *>(nextPendingConnection());
        Q_ASSERT(sock);
        Q_UNUSED(sock);
    }
}

void tProxyServerSocket::incomingConnection(qintptr handle)
{
    QSslSocket *sock=new tServersideTcpSocket(m_workdir, m_cadir, this);
    sock->setSocketDescriptor(handle);
    addPendingConnection(sock);
}
