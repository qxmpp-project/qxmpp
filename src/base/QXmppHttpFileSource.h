// SPDX-FileCopyrightText: 2022 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPHTTPFILESOURCE_H
#define QXMPPHTTPFILESOURCE_H

#include "QXmppGlobal.h"

#include <QUrl>

class QDomElement;
class QXmlStreamWriter;

class QXMPP_EXPORT QXmppHttpFileSource
{
public:
    QXmppHttpFileSource();
    QXmppHttpFileSource(QUrl url);
    QXMPP_PRIVATE_DECLARE_RULE_OF_SIX(QXmppHttpFileSource)

    const QUrl &url() const;
    void setUrl(QUrl url);

    /// \cond
    bool parse(const QDomElement &el);
    void toXml(QXmlStreamWriter *writer) const;
    /// \endcond

private:
    static_assert(sizeof(QUrl) == sizeof(void *));
    QUrl m_url;
};

#endif  // QXMPPHTTPFILESOURCE_H
