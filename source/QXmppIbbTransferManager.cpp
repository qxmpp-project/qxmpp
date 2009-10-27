#include "QXmppIbbTransferManager.h"
#include "QXmppClient.h"

QXmppIbbTransferManager::QXmppIbbTransferManager(QXmppClient* client):
        m_client(client)
{
}

void QXmppIbbTransferManager::teardownIbbTransferManager( const QString &sid )
{
    QString id = getIbbTransferJobBySid(sid)->getId();
    delete m_activeTransfers[id];
    m_activeTransfers.remove(id);
}

void QXmppIbbTransferManager::addIbbTransferManager( QXmppIbbTransferJob *mgr )
{
    m_activeTransfers[mgr->getId()] = mgr;
    connect( mgr, SIGNAL(transferCanceled(QString,QString)),
             this, SIGNAL(byteStreamCanceled(QString,QString)));
    connect( mgr, SIGNAL(transferFinished(QString,QString)),
             this, SIGNAL(byteStreamClosed(QString,QString)));
    connect( mgr, SIGNAL(transferRequested(QString,QString)),
             this, SIGNAL(byteStreamRequestReceived(QString,QString)));
    connect( mgr, SIGNAL(transferStarted(QString)),
             this, SIGNAL(byteStreamOpened(QString)));
    connect( mgr, SIGNAL(readyForTeardown(QString)),
             this, SLOT(teardownIbbTransferManager(QString)));
}

QXmppIbbTransferJob *QXmppIbbTransferManager::getIbbTransferJob( const QString &id )
{
    QXmppIbbTransferJob *mgr = m_activeTransfers[id];
    if( mgr == 0 )
    {
        mgr = new QXmppIbbTransferJob(m_client);
        mgr->setId(id);
        addIbbTransferManager(mgr);
    }
    return mgr;
}

bool QXmppIbbTransferManager::isIbbTransferJobId( const QString &id )
{
    return m_activeTransfers.keys().contains(id);
}

QXmppIbbTransferJob *QXmppIbbTransferManager::getIbbTransferJobBySid( const QString &sid )
{
    foreach( QXmppIbbTransferJob *mgr, m_activeTransfers )
    {
        if ( mgr->getSid() == sid )
            return mgr;
    }

    return 0;
}

void QXmppIbbTransferManager::sendByteStreamRequest( const QString &sid, const QString &bareRemoteJid,  QIODevice *io)
{

    QXmppIbbTransferJob *mgr = new QXmppIbbTransferJob(m_client);
    mgr->setSid( sid );
    mgr->setRemoteJid( bareRemoteJid );  //FIXME: make like send message
    mgr->setIoDevice( io );
    mgr->requestTransfer();
    addIbbTransferManager( mgr );

}

void QXmppIbbTransferManager::acceptByteStreamRequest( const QString &sid, QIODevice *io )
{
    QXmppIbbTransferJob *mgr = getIbbTransferJobBySid(sid);
    if( mgr == 0 )
        return;

    mgr->setIoDevice(io);
    mgr->acceptTransfer();
}

void QXmppIbbTransferManager::rejectByteStreamRequest( const QString &sid )
{
    QXmppIbbTransferJob *mgr = getIbbTransferJobBySid(sid);
    if( mgr == 0 )
        return;
    mgr->cancelTransfer();
}

void QXmppIbbTransferManager::cancelByteStreamRequest( const QString &sid )
{
    QXmppIbbTransferJob *mgr = getIbbTransferJobBySid(sid);
    if( mgr == 0 )
        return;
    mgr->cancelTransfer();
}
