#ifndef INCLUDED_PROXYSERVERSOCKET_H
#define INCLUDED_PROXYSERVERSOCKET_H

#include <QTcpServer>

//EXTERNAL CODE Qt creator can make class skeletons, fyi
class tProxyServerSocket : public QTcpServer
{
    Q_OBJECT
public:
    explicit tProxyServerSocket(const QString &workdir, const QString &cadir, QObject *parent = nullptr);

private slots:
    void onNewConnection();

protected:
    virtual void incomingConnection(qintptr handle) override;

private:
    const QString m_workdir, m_cadir;
};

#endif // INCLUDED_PROXYSERVERSOCKET_H
