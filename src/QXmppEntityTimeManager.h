#ifndef QXMPPENTITYTIMEMANAGER_H
#define QXMPPENTITYTIMEMANAGER_H

#include "QXmppClientExtension.h"

class QXmppOutgoingClient;
class QXmppEntityTimeIq;

/// \ingroup Managers

class QXmppEntityTimeManager : public QXmppClientExtension
{
    Q_OBJECT

public:
    void requestTime(const QString& jid);

    /// \cond
    QStringList discoveryFeatures() const;
    bool handleStanza(QXmppStream *stream, const QDomElement &element);
    /// \endcond

signals:
    void timeReceived(const QXmppEntityTimeIq&);
};

#endif // QXMPPENTITYTIMEMANAGER_H
