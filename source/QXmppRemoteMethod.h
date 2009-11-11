#ifndef QXMPPREMOTEMETHOD_H
#define QXMPPREMOTEMETHOD_H

#include <QObject>
#include <QVariant>

#include "QXmppRpcIq.h"

class QXmppClient;
class QXmppStream;

struct QXmppRemoteMethodResult {
    QXmppRemoteMethodResult() : hasError(false), code(0) { }
    bool hasError;
    int code;
    QString errorMessage;
    QVariant result;
};

class QXmppRemoteMethod : public QObject
{
    Q_OBJECT
public:
    QXmppRemoteMethod(const QString &jid, const QString &method, const QVariantList &args, QXmppClient *client);
    QXmppRemoteMethodResult call( );

private slots:
    void gotError( const QXmppRpcErrorIq &iq );
    void gotResult( const QXmppRpcResponseIq &iq );

signals:
    void callDone();

private:
    QXmppRpcInvokeIq m_payload;
    QXmppClient *m_client;
    QXmppRemoteMethodResult m_result;

};

#endif // QXMPPREMOTEMETHOD_H
