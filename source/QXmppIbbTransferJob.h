#ifndef QXMPPIBBTRANSFERJOB_H
#define QXMPPIBBTRANSFERJOB_H

#include <QObject>
class QIODevice;
class QXmppIbbOpenIq;
class QXmppIbbCloseIq;
class QXmppIbbErrorIq;
class QXmppIbbDataIq;
class QXmppClient;

class QXmppIbbTransferJob : public QObject
{
    Q_OBJECT
public:

    QXmppIbbTransferJob(QXmppClient *parent = 0);
    ~QXmppIbbTransferJob();
    QString getSid() const;
    void setSid( const QString &sid );
    QString getRemoteJid( ) const;
    void setRemoteJid( const QString &jid );
    QString getId() const;
    void setId( const QString &id );

// Used by the client
public slots:
    void requestTransfer( );
    void acceptTransfer( );
    void cancelTransfer( );

signals:
    void transferRequested( const QString &sid,  const QString &remoteJid );
    void transferStarted( const QString &sid );
    void transferFinished( const QString &sid, const QString &reason);
    void transferCanceled( const QString &sid, const QString &reason );
    void readyForTeardown( const QString &sid );

public:
    void setIoDevice( QIODevice *io );
    void setBlockSize( long size);

    // Used by the stream.
    void gotAck();
    void gotOpen( const QXmppIbbOpenIq &open );
    void gotClose( const QXmppIbbCloseIq &close );
    void gotError( const QXmppIbbErrorIq &err );
    void gotData( const  QXmppIbbDataIq &data );

private:
    enum TransferState { Idle, Requesting, Pending, TransferringIn, TransferringOut };
    void sendNextBlock();

    QXmppClient *m_client;
    QIODevice *m_io;
    long m_blockSize;
    qint64 m_streamBlockSize;
    quint16 m_sequence;
    QString m_sid;
    QString m_id;
    QString m_localJid;
    QString m_remoteJid;
    TransferState m_state;
};

#endif // QXMPPIBBTRANSFERJOB_H
