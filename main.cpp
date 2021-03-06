#include <QCoreApplication>
#include <QLocale>
#include <QTranslator>
#include <QSettings>
#include <QHostAddress>
#include <QTcpServer>
#include <QDir>
#include <QFileInfo>
#include <QTextStream>
#include <QDirIterator>

#include "proxyserversocket.h"
#include "sslstuff.h"
#include "mystdio.h"
#include "qthelper.h"

void helpme(char *appname0)
{
    const QString appName=QFileInfo(appname0).baseName();
    cerr<<QObject::tr("Usage: %1 settings [list]                      #lists current settings").arg(appName)<<endl;
    cerr<<QObject::tr("Usage: %1 settings clear|reset [configkey|all] #resets given or all config variables to default").arg(appName)<<endl;
    cerr<<QObject::tr("Usage: %1 settings set configkey value         #sets given config variable to given value").arg(appName)<<endl;
    cerr<<QObject::tr("Usage: %1 settings get configkey               #gets value for given config variable").arg(appName)<<endl;
    cerr<<QObject::tr("Usage: %1                                      #starts the actual mitm app").arg(appName)<<endl;
    cerr<<endl;
}

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName(QStringLiteral("imrepentek"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("org.imrepentek"));
    QCoreApplication::setApplicationName(QStringLiteral("mitm"));
    QCoreApplication::setApplicationVersion(QStringLiteral("1.0"));

    //EXTERNAL CODE the app and translator related code are autogenerated by qt creator
    QCoreApplication app(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "mitm_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            app.installTranslator(&translator);
            break;
        }
    }

    QSettings settings;
    const QMap<QString,QString> defaultSettings={{"listenAddress","127.0.0.1"},
                                                 {"listenPort","8080"},
                                                 {"workDir",QDir::toNativeSeparators(SRCDIR "/certs")},
                                                };

    for(auto [key,value]:asKeyValueRange(defaultSettings))
    {
        if(!settings.contains(key))
            settings.setValue(key,value);
    }

    if(argc>1)
    {
        if(argv[1]==QStringLiteral("settings"))
        {
            if((argc==2)||((argc==3)&&(argv[2]==QStringLiteral("list"))))
            {
                for(const QString &key:settings.allKeys())
                {
                    cout<<key<<": "<<settings.value(key)<<endl;
                }
                return 0;
            }
            else
            {
                if((argv[2]==QStringLiteral("clear"))||(argv[2]==QStringLiteral("reset")))
                {
                    if((argc==3)||((argc==4)&&(argv[3]==QStringLiteral("all"))))
                    {
                        settings.clear();
                        return 0;
                    }
                    else if(argc==4)
                    {
                        const QString key=QString::fromLocal8Bit(argv[3]);
                        if(!settings.contains(key))
                        {
                            cerr<<QObject::tr("Unable to reset config for %1: not a valid config key").arg(key)<<endl;
                            return 1;
                        }
                        settings.remove(key);
                        return 0;
                    }
                }
                else if((argc==5)&&(argv[2]==QStringLiteral("set")))
                {
                    const QString key=QString::fromLocal8Bit(argv[3]);
                    const QString value=QString::fromLocal8Bit(argv[4]);
                    if(!settings.contains(key))
                    {
                        cerr<<QObject::tr("Unable to set config for %1: not a valid config key").arg(key)<<endl;
                        return 1;
                    }
                    settings.setValue(key,value);
                    return 0;
                }
                else if((argc==3)&&(argv[2]==QStringLiteral("get")))
                {
                    const QString key=QString::fromLocal8Bit(argv[3]);
                    if(!settings.contains(key))
                    {
                        cout<<QObject::tr("%s is not set").arg(key)<<endl;
                        return 0;
                    }

                    cout<<key<<": "<<settings.value(key)<<endl;
                    return 0;
                }
            }
        }

        helpme(argv[0]);
        return 1;
    }

    quint16 listenPort;
    {
        bool ok;
        uint listenPort0=settings.value(QStringLiteral("listenPort")).toUInt(&ok);
        if((!ok)||(!listenPort0)||(listenPort0>=65536))
        {
            cerr<<QObject::tr("Your listenPort setting should be a valid port number ([1..65535]).")<<endl;
            return 1;
        }
        listenPort=listenPort0;
    }
    const QHostAddress listenAddress=settings.value(QStringLiteral("listenAddress")).toString().isEmpty()?QHostAddress::Any:QHostAddress(settings.value(QStringLiteral("listenAddress")).toString());
    const QString workdir=settings.value(QStringLiteral("workDir")).toString();
    const QString cadir=workdir+"/00cacert";

    {
        QFileInfo z(workdir);
        if(z.exists()&&(!z.isDir()))
        {
            QString msg=QObject::tr("Fatal error: %1 is not a directory").arg(listenAddress.toString()).arg(listenPort);
            qCritical()<<msg.toUtf8().constData();
            return 1;
        }
        if(!z.exists())
            QDir().mkdir(workdir);

        if(!QFileInfo::exists(cadir))
        {
            qInfo()<<"Generating ca cert...";
            QDir().mkdir(cadir);
            generateX509(cadir+"/cert",cadir+"/key");
            qInfo()<<"Ok, ca cert generated.";
        }
    }

    long serial=0;
    //EXTERNAL CODE from https://doc.qt.io/qt-5/qdiriterator.html
    QDirIterator it(workdir,{"cert"}, QDir::NoFilter, QDirIterator::Subdirectories);
    while(it.hasNext())
    {
        it.next();
        ++serial;
    }

    tProxyServerSocket sock(workdir,cadir,serial);
    const bool listenSucceeded=sock.listen(listenAddress,listenPort);
    if(!listenSucceeded)
    {
        QString msg=QObject::tr("Fatal error: Failed to listen at %1:%2").arg(listenAddress.toString()).arg(listenPort);
        qCritical()<<msg.toUtf8().constData();
        return 1;
    }

    return app.exec();
}
