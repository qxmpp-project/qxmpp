#ifndef REMOTEINTERFACE_H
#define REMOTEINTERFACE_H

#include "QXmppRpcManager.h"

class RemoteInterface : public QXmppInvokable
{
    Q_OBJECT
public:
    RemoteInterface(QObject *parent = nullptr);

    bool isAuthorized( const QString &jid ) const override;

// RPC Interface
public slots:
    QString echoString( const QString &message );

};

#endif // REMOTEINTERFACE_H
