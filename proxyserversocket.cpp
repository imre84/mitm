#include "proxyserversocket.h"
#include "serversidetcpsocket.h"

#include <QSslSocket>

tProxyServerSocket::tProxyServerSocket(const QString &workdir, const QString &cadir, QObject *parent) : QTcpServer(parent),m_workdir(workdir),m_cadir(cadir)
{
    connect(this,&QTcpServer::newConnection,this,&tProxyServerSocket::onNewConnection);
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
