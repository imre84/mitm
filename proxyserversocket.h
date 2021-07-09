#ifndef INCLUDED_PROXYSERVERSOCKET_H
#define INCLUDED_PROXYSERVERSOCKET_H

#include <QTcpServer>

//EXTERNAL CODE Qt creator can make class skeletons, fyi
class tProxyServerSocket : public QTcpServer
{
    Q_OBJECT
public:
    explicit tProxyServerSocket(const QString &workdir, const QString &cadir, long serial, QObject *parent = nullptr);
    long getSerial();

private slots:
    void onNewConnection();

protected:
    virtual void incomingConnection(qintptr handle) override;

private:
    const QString m_workdir, m_cadir;
    long m_serial;
};

#endif // INCLUDED_PROXYSERVERSOCKET_H
