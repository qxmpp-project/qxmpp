#include "QXmppIbbTransferJob.h"

#include <QIODevice>
#include <QUuid>
#include "QXmppIbbIqs.h"
#include "QXmppDataIq.h"
#include "QXmppClient.h"
#include "QXmppUtils.h"

QXmppIbbTransferJob::QXmppIbbTransferJob( QXmppClient *parent )
    : QObject (parent ), m_client(parent), m_io(0), m_blockSize(4096),
    m_streamBlockSize(0), m_sequence(0), m_state(Idle)
{
    m_localJid = parent->getConfiguration().getJid();
    m_id = generateStanzaHash();
    m_sid = generateStanzaHash();
}

QXmppIbbTransferJob::~QXmppIbbTransferJob()
{
}

QString QXmppIbbTransferJob::getSid() const
{
    return m_sid;
}

void QXmppIbbTransferJob::setSid( const QString &sid )
{
    m_sid = sid;
}

void QXmppIbbTransferJob::setRemoteJid( const QString &jid )
{
    m_remoteJid = jid;
}

QString QXmppIbbTransferJob::getRemoteJid( ) const
{
    return m_remoteJid;
}

QString QXmppIbbTransferJob::getId() const
{
   return m_id;
}

void QXmppIbbTransferJob::setId( const QString &id )
{
    m_id  = id;
}


void QXmppIbbTransferJob::requestTransfer( )
{
    m_state = Requesting;
    QXmppIbbOpenIq request;
    request.setBlockSize(m_blockSize);
    request.setTo( m_remoteJid );
    request.setFrom( m_localJid );
    request.setSid( m_sid );
    request.generateAndSetNextId();
    m_id = request.getId();

    m_client->sendPacket( request );

}

void QXmppIbbTransferJob::acceptTransfer( )
{
    if( m_state != Pending )
    {
        return;
    }

    m_state = TransferringIn;
    QXmppIbbAckIq ack;
    ack.setTo( m_remoteJid );
    ack.setFrom( m_localJid );
    ack.setId( m_id );
    m_client->sendPacket( ack );
    emit transferStarted(m_sid);
}

void QXmppIbbTransferJob::cancelTransfer( )
{
    QXmppIbbErrorIq cancel;
    cancel.setId(m_id);
    cancel.setErrorType( QXmppIbbErrorIq::Cancel );
    m_client->sendPacket( cancel );
}

void QXmppIbbTransferJob::setIoDevice( QIODevice *io )
{
    m_io = io;
}

void QXmppIbbTransferJob::setBlockSize( long size)
{
    m_blockSize = size;
}

void QXmppIbbTransferJob::gotAck()
{
    if( m_state == Requesting )
    {
        // start transfer
        m_state = TransferringOut;
        sendNextBlock();
    }
    else if ( m_state == TransferringOut )
    {
        // send next packet
        sendNextBlock();
    }
    else if ( m_state == Idle )
    {
         emit readyForTeardown(m_sid);
    }
    else
    {

    }
}

void QXmppIbbTransferJob::gotOpen( const QXmppIbbOpenIq &open )
{
    m_sid = open.getSid();
    m_id = open.getId();
    m_remoteJid = open.getFrom();
    if( open.getBlockSize() > m_blockSize )
    {
        // cancel
        m_state = Idle;
        QXmppIbbErrorIq modifyError;
        modifyError.setId(m_id);
        modifyError.setErrorType( QXmppIbbErrorIq::Modify );
        m_client->sendPacket( modifyError );
        emit readyForTeardown(m_sid);
    }
    else
    {
        m_streamBlockSize = open.getBlockSize();
        m_state = Pending;
        emit transferRequested( m_sid , m_remoteJid );
    }
}

void QXmppIbbTransferJob::gotClose( const QXmppIbbCloseIq &close )
{
    m_state = Idle;
    QXmppIbbAckIq ack;
    ack.setTo( m_remoteJid );
    ack.setFrom( m_localJid );
    ack.setId( m_id );
    m_client->sendPacket( ack );
    emit transferFinished(m_sid, "Closed");
    emit readyForTeardown(m_sid);
}

void QXmppIbbTransferJob::gotError( const QXmppIbbErrorIq &err )
{
    m_state = Idle;
    emit transferCanceled(m_sid,err.getError().getConditionStr());
    emit readyForTeardown(m_sid);
}
void QXmppIbbTransferJob::gotData( const  QXmppDataIq &data )
{
    if( m_io &&
        (data.getSequence() == 0 || data.getSequence() > m_sequence) )
    {
        m_io->write( data.getPayload() );
        m_sequence = data.getSequence();
        QXmppIbbAckIq ack;
        ack.setId(m_id);
        ack.setTo(m_remoteJid);
        ack.setFrom(m_localJid);
        m_client->sendPacket( ack );
    }
    else
    {
        QXmppIbbErrorIq error;
        error.setId(m_id);
        error.setTo(m_remoteJid);
        error.setFrom(m_localJid);
        error.setErrorType(QXmppIbbErrorIq::Cancel);
        m_client->sendPacket( error );
    }
}

void QXmppIbbTransferJob::sendNextBlock()
{

    if( m_io == 0 || !m_io->isReadable() )
    {
        QXmppIbbErrorIq error;
        error.setId(m_id);
        error.setTo(m_remoteJid);
        error.setFrom(m_localJid);
        error.setErrorType(QXmppIbbErrorIq::Cancel);
        m_client->sendPacket( error );
    }
    else if( m_io->atEnd() || !m_io->isOpen() )
    {
        QXmppIbbCloseIq close;
        close.setId(m_id);
        close.setTo(m_remoteJid);
        close.setFrom(m_localJid);
        close.setSid(m_sid);
        m_client->sendPacket( close );
        m_state = Idle;
        emit transferFinished(m_sid, "Send finished");
    }
    else
    {
        //FIXME: work better with sockets and other stream devices.
        QByteArray buffer = m_io->read( m_blockSize );

        m_sequence++;
        QXmppDataIq sendData;
        sendData.setId(m_id);
        sendData.setTo(m_remoteJid);
        sendData.setFrom(m_localJid);
        sendData.setSid(m_sid);
        sendData.setSequence(m_sequence);
        sendData.setPayload( buffer );

        m_client->sendPacket( sendData );
    }
}
