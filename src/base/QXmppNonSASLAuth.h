// SPDX-FileCopyrightText: 2009 Manjeet Dahiya <manjeetdahiya@gmail.com>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXmppNonSASLAuth_H
#define QXmppNonSASLAuth_H

#include "QXmppIq.h"

class QXMPP_EXPORT QXmppNonSASLAuthIq : public QXmppIq
{
public:
    QXmppNonSASLAuthIq();

    QString username() const;
    void setUsername(const QString &username);

    QByteArray digest() const;
    void setDigest(const QString &streamId, const QString &password);

    QString password() const;
    void setPassword(const QString &password);

    QString resource() const;
    void setResource(const QString &resource);

    /// \cond
    static bool isNonSASLAuthIq(const QDomElement &element);

protected:
    void parseElementFromChild(const QDomElement &element) override;
    void toXmlElementFromChild(QXmlStreamWriter *writer) const override;
    /// \endcond

private:
    QString m_username;
    QByteArray m_digest;
    QString m_password;
    QString m_resource;
};

#endif  // QXmppNonSASLAuth_H
