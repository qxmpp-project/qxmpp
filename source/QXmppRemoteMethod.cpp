#include "QXmppRemoteMethod.h"
#include "QXmppClient.h"
#include "QXmppUtils.h"
#include "QXmppConfiguration.h"

#include <QEventLoop>
#include <QTimer>
#include <qdebug.h>

QXmppRemoteMethod::QXmppRemoteMethod(const QString &jid, const QString &method, const QVariantList &args, QXmppClient *client) :
        QObject(client), m_client(client)
{
    m_payload.setTo( jid );
    m_payload.setFrom( client->getConfiguration().jid() );
    m_payload.setInterface( method.section('.', 0, 0 ) );
    m_payload.setMethod( method.section('.', 1) );
    m_payload.setArguments( args );
}

QXmppRemoteMethodResult QXmppRemoteMethod::call( )
{
    QEventLoop loop(this);
    connect( this, SIGNAL(callDone()), &loop, SLOT(quit()));
    QTimer::singleShot(30000,&loop, SLOT(quit())); // Timeout incase the other end hangs...

    m_client->sendPacket( m_payload );

    loop.exec( QEventLoop::ExcludeUserInputEvents | QEventLoop::WaitForMoreEvents );
    return m_result;
}

void QXmppRemoteMethod::gotError( const QXmppRpcErrorIq &iq )
{
    if ( iq.id() == m_payload.id() )
    {
        m_result.hasError = true;
        m_result.errorMessage = iq.error().text();
        m_result.code = iq.error().type();
        emit callDone();
    }
}

void QXmppRemoteMethod::gotResult( const QXmppRpcResponseIq &iq )
{
    if ( iq.id() == m_payload.id() )
    {
        m_result.hasError = false;
        m_result.result = iq.values();
        emit callDone();
    }
}
