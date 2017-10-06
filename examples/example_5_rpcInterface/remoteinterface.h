#ifndef REMOTEINTERFACE_H
#define REMOTEINTERFACE_H

#include "QXmppRpcManager.h"

class RemoteInterface : public QXmppInvokable
{
    Q_OBJECT
public:
    RemoteInterface(QObject *parent = 0);

    bool isAuthorized( const QString &jid ) const;

// RPC Interface
public Q_SLOTS:
    QString echoString( const QString &message );

};

#endif // REMOTEINTERFACE_H
