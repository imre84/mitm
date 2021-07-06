#ifndef INCLUDED_PROXYSERVERSOCKET_H
#define INCLUDED_PROXYSERVERSOCKET_H

#include <QTcpServer>

//EXTERNAL CODE Qt creator can make a skeleton class, fyi
class tProxyServerSocket : public QTcpServer
{
    Q_OBJECT
public:
    explicit tProxyServerSocket(QObject *parent = nullptr);

private slots:
    void onNewConnection();

protected:
    virtual void incomingConnection(qintptr handle) override;
};

#endif // INCLUDED_PROXYSERVERSOCKET_H
