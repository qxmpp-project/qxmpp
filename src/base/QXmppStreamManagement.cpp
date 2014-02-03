#include "QXmppStreamManagement.h"

#include "QXmppConfiguration.h"
#include <QDomElement>
#include <qxmlstream.h>
#include "QXmppConstants.h"


class QXmppStreamManagementPrivate
{
public:
    QXmppStreamManagementPrivate();
    QXmppConfiguration::StreamManagementMode streamManagementMode;
    bool streamManagementEnabled;
    int  outboundCount;
    int  inboundCount;
    QMap <int, QXmppStanza*> outboundBuffer;

};

QXmppStreamManagementPrivate::QXmppStreamManagementPrivate()
    : streamManagementMode(QXmppConfiguration::SMDisabled)
    , streamManagementEnabled(false)
    , outboundCount(0)
    , inboundCount(0)
{

}

QXmppStreamManagement::QXmppStreamManagement(QObject *parent)
    : QXmppLoggable(parent)
    , d(new QXmppStreamManagementPrivate)
{
}

void QXmppStreamManagement::enableStreamManagement()
{
    d->streamManagementEnabled = true;
    d->outboundCount = 0;
    d->inboundCount = 0;
}

void QXmppStreamManagement::stanzaSent(const QXmppStanza &stanza)
{
    d->outboundCount++;
    debug(QString("SM STANZA SENT outbound counter:%1").arg(QString::number(d->outboundCount)));
    switch(stanza.getStanzaType())
    {
    case QXmppStanza::Message:
    {
        QXmppMessage *message = new QXmppMessage(static_cast<QXmppMessage const &>(stanza));
        d->outboundBuffer.insert(d->outboundCount, message);
        break;
    }
    case QXmppStanza::Iq:
    {
        QXmppIq *iq = new QXmppIq(static_cast<QXmppIq const &>(stanza));
        d->outboundBuffer.insert(d->outboundCount, iq);
        break;
    }
    case QXmppStanza::Presence:
    {
        QXmppPresence *presence = new QXmppPresence(static_cast<QXmppPresence const &>(stanza));
        d->outboundBuffer.insert(d->outboundCount, presence);
        break;
    }
    default:
        break;
    }
}

void QXmppStreamManagement::ackReceived(const int handled)
{
    QMapIterator <int, QXmppStanza*> i(d->outboundBuffer);
    const QXmppStanza *stanza = NULL;
    while (i.hasNext()) {
        i.next();
        if(i.key()<= handled)
        {
            stanza = i.value();
            switch (stanza->getStanzaType())
            {
                case QXmppStanza::Message:
                {
                    QXmppMessage message(static_cast<QXmppMessage const &> (*stanza));
                    emit messageAcknowledged(message, true);
                    break;
                }
                case QXmppStanza::Iq:
                {
                    QXmppIq iq(static_cast<QXmppIq const &> (*stanza));
                    emit iqAcknowledged(iq, true);
                    break;
                }
                case QXmppStanza::Presence:
                {
                    QXmppPresence presence(static_cast<QXmppPresence const &>(*stanza));
                    emit presenceAcknowledged(presence, true);
                    break;
                }
                default:
                    break;
           }

           d->outboundBuffer.remove(i.key());
           delete stanza;
           debug(QString("SM h:%1 removed from the buffer").arg(i.key()));
       }
    }
}

void QXmppStreamManagement::stanzaHandled()
{
    d->inboundCount++;
}

void QXmppStreamManagement::enableToXml(QXmlStreamWriter *xmlStream, const bool resume)
{
    xmlStream->writeStartElement("enable");
    xmlStream->writeAttribute("xmlns",ns_stream_management);
    if(resume)
    {
        xmlStream->writeAttribute("resume","true");
    }
    xmlStream->writeEndElement();
}

void QXmppStreamManagement::ackToXml(QXmlStreamWriter *xmlStream)
{
    xmlStream->writeStartElement("a");
    xmlStream->writeAttribute("xmlns",ns_stream_management);
    QString counter = QString::number(d->inboundCount);
    xmlStream->writeAttribute("h",counter);
    xmlStream->writeEndElement();
}

void QXmppStreamManagement::requestToXml(QXmlStreamWriter *xmlStream)
{
    xmlStream->writeStartElement("r");
    xmlStream->writeAttribute("xmlns",ns_stream_management);
    xmlStream->writeEndElement();
}

bool QXmppStreamManagement::isStreamManagementEnabled() const
{
    return d->streamManagementEnabled;
}

int QXmppStreamManagement::outboundCounter() const
{
    return d->outboundCount;
}

void QXmppStreamManagement::socketDisconnected()
{

    QMapIterator <int, QXmppStanza*> i(d->outboundBuffer);
    const QXmppStanza *stanza = NULL;
    while (i.hasNext()) {
        i.next();
        stanza = i.value();
        switch (stanza->getStanzaType())
        {
            case QXmppStanza::Message:
            {
                QXmppMessage message(static_cast<QXmppMessage const &> (*stanza));
                emit messageAcknowledged(message, false);
                break;
            }
            case QXmppStanza::Iq:
            {
                QXmppIq iq(static_cast<QXmppIq const &> (*stanza));
                emit iqAcknowledged(iq, false);
                break;
            }
            case QXmppStanza::Presence:
            {
                QXmppPresence presence(static_cast<QXmppPresence const &>(*stanza));
                emit presenceAcknowledged(presence, false);
                break;
            }
            default:
                break;
      }
       d->outboundBuffer.remove(i.key());
       delete stanza;
       debug(QString("SM h:%1 removed from the buffer").arg(i.key()));
    }
}
