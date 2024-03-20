// SPDX-FileCopyrightText: 2024 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPFALLBACK_H
#define QXMPPFALLBACK_H

#include "QXmppGlobal.h"

#include <optional>

#include <QSharedDataPointer>

class QDomElement;
class QXmlStreamWriter;

struct QXmppFallbackPrivate;

class QXMPP_EXPORT QXmppFallback
{
public:
    enum Element {
        Body,
        Subject,
    };

    struct Range {
        /// Start index of the range
        uint32_t start;
        /// End index of the range
        uint32_t end;
    };

    struct Reference {
        /// Element of the message stanza this refers to
        Element element;
        /// Optional character range in the text
        std::optional<Range> range;
    };

    QXmppFallback(const QString &forNamespace, const QVector<Reference> &references);
    QXMPP_PRIVATE_DECLARE_RULE_OF_SIX(QXmppFallback)

    const QString &forNamespace() const;
    void setForNamespace(const QString &);

    const QVector<Reference> &references() const;
    void setReferences(const QVector<Reference> &);

    static std::optional<QXmppFallback> fromDom(const QDomElement &);
    void toXml(QXmlStreamWriter *) const;

private:
    QSharedDataPointer<QXmppFallbackPrivate> d;
};

#endif  // QXMPPFALLBACK_H
