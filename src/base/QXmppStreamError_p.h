// SPDX-FileCopyrightText: 2024 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPSTREAMERROR_P_H
#define QXMPPSTREAMERROR_P_H

#include "QXmppError.h"
#include "QXmppStreamError.h"

#include <variant>

class QDomElement;
class QXmlStreamWriter;

namespace QXmpp::Private {

// implemented in Stream.cpp
struct StreamErrorElement {
    struct SeeOtherHost {
        QString host;
        quint16 port;

        bool operator==(const SeeOtherHost &o) const { return host == o.host && port == o.port; }
    };

    using Condition = std::variant<StreamError, SeeOtherHost>;

    static QString streamErrorToString(StreamError);
    static std::variant<StreamErrorElement, QXmppError> fromDom(const QDomElement &);
    void toXml(QXmlStreamWriter *) const;

    Condition condition;
    QString text;

    bool operator==(const StreamErrorElement &o) const { return condition == o.condition && text == o.text; }
};

}  // namespace QXmpp::Private

#endif  // QXMPPSTREAMERROR_P_H
