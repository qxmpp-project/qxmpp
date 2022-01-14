// SPDX-FileCopyrightText: 2011 Manjeet Dahiya <manjeetdahiya@gmail.com>
// SPDX-FileCopyrightText: 2010 Jeremy Lainé <jeremy.laine@m4x.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPBINDIQ_H
#define QXMPPBINDIQ_H

#include "QXmppIq.h"

/// \brief The QXmppBindIq class represents an IQ used for resource
/// binding as defined by RFC 3921.
///
/// \ingroup Stanzas

class QXMPP_EXPORT QXmppBindIq : public QXmppIq
{
public:
    QString jid() const;
    void setJid(const QString &);

    QString resource() const;
    void setResource(const QString &);

    /// \cond
    static bool isBindIq(const QDomElement &element);
    /// \endcond

protected:
    /// \cond
    void parseElementFromChild(const QDomElement &element) override;
    void toXmlElementFromChild(QXmlStreamWriter *writer) const override;
    /// \endcond

private:
    QString m_jid;
    QString m_resource;
};

#endif  // QXMPPBIND_H
