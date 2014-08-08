#ifndef QXMPPPEPMANAGER_H
#define QXMPPPEPMANAGER_H

#include "QXmppClientExtension.h"
#include "QXmppReachAddress.h"
#include "QXmppPubSubIq.h"
#include "QXmppGaming.h"

class QXMPP_EXPORT QXmppPEPManager :  public QXmppClientExtension
{
     Q_OBJECT
public:

    QXmppPEPManager();
    QXmppPEPManager(const bool reachActive, const bool gamingActive = false);

    void sendGaming(const QXmppGaming& gaming);

    /// \cond
    virtual QStringList discoveryFeatures() const;
    virtual bool handleStanza(const QDomElement &stanza);
    /// \endcond

signals:
    void reachabilityAddressReceived(const QString &jid, const QString &id, const QXmppReachAddress& addr);
    void gamingReceived(const QString &jid, const QString &id, const QXmppGaming& game);

private:
    // XEP-0152
    bool m_reachActive;
    // XEP-0196: User Gaming
    bool m_gamingActive;

};


#endif // QXMPPPEPMANAGER_H
