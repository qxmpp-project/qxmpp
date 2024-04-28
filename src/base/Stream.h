// SPDX-FileCopyrightText: 2024 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef STREAM_H
#define STREAM_H

#include <optional>

#include <QMetaType>
#include <QString>

class QDomElement;
class QXmlStreamReader;
class QXmlStreamWriter;

namespace QXmpp::Private {

struct StreamOpen {
    static StreamOpen fromXml(QXmlStreamReader &reader);
    void toXml(QXmlStreamWriter *) const;

    QString to;
    QString from;
    QString id;
    QString version;
    QString xmlns;
};

struct StarttlsRequest {
    static std::optional<StarttlsRequest> fromDom(const QDomElement &);
    void toXml(QXmlStreamWriter *) const;
};

struct StarttlsProceed {
    static std::optional<StarttlsProceed> fromDom(const QDomElement &);
    void toXml(QXmlStreamWriter *) const;
};

struct CsiActive {
    void toXml(QXmlStreamWriter *w) const;
};

struct CsiInactive {
    void toXml(QXmlStreamWriter *w) const;
};

}  // namespace QXmpp::Private

Q_DECLARE_METATYPE(QXmpp::Private::StreamOpen)

#endif  // STREAM_H
