#ifndef QXMPPPEPMANAGER_H
#define QXMPPPEPMANAGER_H

#include "QXmppClientExtension.h"
#include "QXmppReachAddress.h"
#include "QXmppPubSubIq.h"

class QXMPP_EXPORT QXmppPEPManager :  public QXmppClientExtension
{
     Q_OBJECT
public:

    QXmppPEPManager();
    QXmppPEPManager(const bool reachActive);
    /// \cond
    virtual QStringList discoveryFeatures() const;
    virtual bool handleStanza(const QDomElement &stanza);
    /// \endcond

signals:
    void reachabilityAddressReceived(const QString &jid, const QString &id, const QXmppReachAddress& addr);

private:
    // XEP-0152
    bool m_reachActive;

};


#endif // QXMPPPEPMANAGER_H
