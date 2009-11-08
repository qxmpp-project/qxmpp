#ifndef QXMPPIBBTRANSFERMANAGER_H
#define QXMPPIBBTRANSFERMANAGER_H

#include <QObject>
#include <QHash>
#include "QXmppIbbTransferJob.h"

class QXmppClient;

class QXmppIbbTransferManager : public QObject
{
    Q_OBJECT
public:
    QXmppIbbTransferManager(QXmppClient* client);

signals:
    /// Notifies that a XMPP bytestream request has been received.  The
    /// program must either reply with an acceptByteStreamRequest(...) or
    /// a rejectByteStreamRequest(...) depending on the desired response.
    void byteStreamRequestReceived( const QString &sid, const QString &remoteJid );
    /// Notifies that a XMPP bytestream has been closed.
    void byteStreamClosed( const QString &sid , const QString &reason );
    /// Notifies that a XMPP bytestream was canceled by the remote peer.
    /// The reason is given for the cancelation.
    void byteStreamCanceled( const QString &sid , const QString &reason );
    /// Notifes that the XMPP bytestream has been opened and the transfer
    /// has started.
    void byteStreamOpened( const QString &sid );

public:
    QXmppIbbTransferJob *getIbbTransferJob( const QString &id );
    bool isIbbTransferJobId( const QString &id );

public slots:
    /// Send a request to open a bytestream to a specific jid.  Once the
    /// stream is opened then the data is read from the QIODevice.  The
    /// QIODevice MUST be be open and ready for reading otherwise the
    /// transfer will fail.  When there are no more bytes available to
    /// send from the QIODevice then the bytestream is closed.
    void sendByteStreamRequest( const QString &sid, const QString &bareJid,  QIODevice *io);
    /// Accept a bytestream with a specific sid.  Data from the remote
    /// peer is then written to the QIODevice.  Therefor the QIODevice must
    /// be open and ready for writing before this method is called.
    void acceptByteStreamRequest( const QString &sid, QIODevice *io );
    /// Rejects a bytestream from a specific sid.
    void rejectByteStreamRequest( const QString &sid );
    /// Cacels a currenly connected bytestream with a specific sid.
    void cancelByteStreamRequest( const QString &sid );

private slots:
    void addIbbTransferManager( QXmppIbbTransferJob *mgr );
    void teardownIbbTransferManager( const QString &sid );

private:
    QXmppIbbTransferJob *getIbbTransferJobBySid( const QString &sid );

    QXmppClient* m_client;
    QHash<QString, QXmppIbbTransferJob*> m_activeTransfers;
};

#endif // QXMPPIBBTRANSFERMANAGER_H
