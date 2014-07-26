#ifndef QXMPPLASTACTIVITYMANAGER_H
#define QXMPPLASTACTIVITYMANAGER_H

#include "QXmppClientExtension.h"

/// \brief The QXmppLastActivityManager class makes it possible to get the most
/// recent presence information from an offline contact.
/// It is an  implementation of XEP-0012: Last Activity.
/// http://xmpp.org/extensions/xep-0012.html
///
/// \ingroup Managers

class QXmppLastActivityIq;

class QXMPP_EXPORT QXmppLastActivityManager : public QXmppClientExtension
{
    Q_OBJECT

public:
    QXmppLastActivityManager();
    ~QXmppLastActivityManager();

    QString requestLastActivity(const QString& to = "");
    QStringList requestLastActivityList(const QStringList& list);

    /// \cond
    QStringList discoveryFeatures() const;
    bool handleStanza(const QDomElement& element);
    /// \endcond

signals:
    /// \brief This signal is emitted when a last activity response is received.
    void lastActivityReceived(const QXmppLastActivityIq&);
};


#endif // QXMPPLASTACTIVITYMANAGER_H
