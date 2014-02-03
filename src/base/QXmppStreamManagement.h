#ifndef QXMPPSTREAMMANAGEMENT_H
#define QXMPPSTREAMMANAGEMENT_H

#include <qxmlstream.h>
#include "QXmppGlobal.h"
#include "QXmppLogger.h"
#include "QXmppMessage.h"
#include "QXmppPresence.h"
#include "QXmppIbbIq.h"
#include "QXmppStanza.h"

class QXmppStreamManagementPrivate;

class QXMPP_EXPORT QXmppStreamManagement : public QXmppLoggable
{
    Q_OBJECT
public:
    QXmppStreamManagement(QObject* parent = 0);

    void enableStreamManagement();

    void stanzaSent(const QXmppStanza& stanza);
    void ackReceived(const int handled);

    void stanzaHandled();

    void enableToXml(QXmlStreamWriter *xmlStream, const bool resume);
    void ackToXml(QXmlStreamWriter *xmlStream);
    void requestToXml(QXmlStreamWriter *xmlStream);

    bool isStreamManagementEnabled() const;
    int  outboundCounter() const;

    void socketDisconnected();

signals:
    /// This singal is emitted when the server AKC/NACK a message
    void messageAcknowledged(const QXmppMessage&, const bool);

    /// This singal is emitted when the server AKC/NACK a presence
    void presenceAcknowledged(const QXmppPresence&, const bool);

    /// This singal is emitted when the server AKC a iq
    void iqAcknowledged(const QXmppIq&, const bool);

private:
    QXmppStreamManagementPrivate * const d;

};

#endif // QXMPPSTREAMMANAGEMENT_H
