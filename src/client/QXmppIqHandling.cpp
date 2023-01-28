// SPDX-FileCopyrightText: 2022 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppIqHandling.h"

void QXmpp::Private::sendIqReply(QXmppClient *client,
                                 const QString &requestId,
                                 const QString &requestFrom,
                                 const std::optional<QXmppE2eeMetadata> &e2eeMetadata,
                                 QXmppIq &&iq)
{
    // default type is 'result'
    switch (iq.type()) {
    case QXmppIq::Get:
    case QXmppIq::Set:
        iq.setType(QXmppIq::Result);
        break;
    case QXmppIq::Error:
    case QXmppIq::Result:
        break;
    }

    iq.setTo(requestFrom);
    iq.setId(requestId);
    client->reply(std::move(iq), e2eeMetadata);
}

std::tuple<bool, QString, QString> QXmpp::Private::checkIsIqRequest(const QDomElement &el)
{
    if (el.tagName() != QStringLiteral("iq")) {
        return { false, {}, {} };
    }
    auto queryElement = el.firstChildElement();
    auto iqType = el.attribute(QStringLiteral("type"));
    if (iqType != QStringLiteral("get") && iqType != QStringLiteral("set")) {
        return { false, {}, {} };
    }
    return { true, queryElement.tagName(), queryElement.namespaceURI() };
}
