// SPDX-FileCopyrightText: 2022 Linus Jahn <lnj@kaidan.im>
// SPDX-FileCopyrightText: 2022 Melvin Keskin <melvo@olomono.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPUTILS_P_H
#define QXMPPUTILS_P_H

#include "QXmppGlobal.h"

#include <stdint.h>

#include <QByteArray>

class QDomElement;
class QXmlStreamWriter;

namespace QXmpp::Private {

// Helper for Q(Any)StringView overloads that were added later
inline auto toString65(QStringView s)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    return s;
#else
    return s.toString();
#endif
}

// XML streams
void writeOptionalXmlAttribute(QXmlStreamWriter *stream, const QString &name, const QString &value);
void writeXmlTextElement(QXmlStreamWriter *stream, const QString &name, const QString &value);

// DOM
QDomElement firstChildElement(const QDomElement &, QStringView tagName, QStringView xmlNs = {});
QDomElement firstChildElement(const QDomElement &, QStringView tagName, const char *xmlNs);

QXMPP_EXPORT QByteArray generateRandomBytes(uint32_t minimumByteCount, uint32_t maximumByteCount);
QXMPP_EXPORT void generateRandomBytes(uint8_t *bytes, uint32_t byteCount);
float calculateProgress(qint64 transferred, qint64 total);

}  // namespace QXmpp::Private

#endif  // QXMPPUTILS_P_H
