// SPDX-FileCopyrightText: 2010 Jeremy Lainé <jeremy.laine@m4x.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPVERSIONIQ_H
#define QXMPPVERSIONIQ_H

#include "QXmppIq.h"

/// \brief The QXmppVersionIq class represents an IQ for conveying a software
/// version as defined by \xep{0092}: Software Version.
///
/// \ingroup Stanzas

class QXMPP_EXPORT QXmppVersionIq : public QXmppIq
{
public:
    QString name() const;
    void setName(const QString &name);

    QString os() const;
    void setOs(const QString &os);

    QString version() const;
    void setVersion(const QString &version);

    /// \cond
    static bool isVersionIq(const QDomElement &element);
    static bool checkIqType(const QString &tagName, const QString &xmlNamespace);
    /// \endcond

protected:
    /// \cond
    void parseElementFromChild(const QDomElement &element) override;
    void toXmlElementFromChild(QXmlStreamWriter *writer) const override;
    /// \endcond

private:
    QString m_name;
    QString m_os;
    QString m_version;
};

#endif
