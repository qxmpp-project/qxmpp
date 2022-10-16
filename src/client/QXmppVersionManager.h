// SPDX-FileCopyrightText: 2010 Manjeet Dahiya <manjeetdahiya@gmail.com>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPVERSIONMANAGER_H
#define QXMPPVERSIONMANAGER_H

#include "QXmppClientExtension.h"

class QXmppVersionIq;
class QXmppVersionManagerPrivate;

///
/// \brief The QXmppVersionManager class makes it possible to request for
/// the software version of an entity as defined by \xep{0092}: Software Version.
///
/// \note Its object should not be created using its constructor. Instead
/// \c QXmppClient::findExtension<QXmppVersionManager>() should be used to get
/// the instantiated object of this class.
///
/// \ingroup Managers
///
class QXMPP_EXPORT QXmppVersionManager : public QXmppClientExtension
{
    Q_OBJECT

public:
    QXmppVersionManager();
    ~QXmppVersionManager() override;

    QString requestVersion(const QString &jid);

    void setClientName(const QString &);
    void setClientVersion(const QString &);
    void setClientOs(const QString &);

    QString clientName() const;
    QString clientVersion() const;
    QString clientOs() const;

    /// \cond
    QStringList discoveryFeatures() const override;
    bool handleStanza(const QDomElement &element) override;
    QXmppVersionIq handleIq(QXmppVersionIq &&iq);
    /// \endcond

Q_SIGNALS:
    /// \brief This signal is emitted when a version response is received.
    void versionReceived(const QXmppVersionIq &);

private:
    const std::unique_ptr<QXmppVersionManagerPrivate> d;
};

#endif  // QXMPPVERSIONMANAGER_H
