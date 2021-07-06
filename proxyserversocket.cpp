#include "proxyserversocket.h"
#include "serversidetcpsocket.h"

#include <QSslSocket>

tProxyServerSocket::tProxyServerSocket(QObject *parent) : QTcpServer(parent)
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
    QSslSocket *sock=new tServersideTcpSocket(this);
    sock->setSocketDescriptor(handle);
    addPendingConnection(sock);
}
