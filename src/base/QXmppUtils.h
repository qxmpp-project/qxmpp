// SPDX-FileCopyrightText: 2009 Manjeet Dahiya <manjeetdahiya@gmail.com>
// SPDX-FileCopyrightText: 2010 Jeremy Lainé <jeremy.laine@m4x.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPUTILS_H
#define QXMPPUTILS_H

// forward declarations of QXmlStream* classes will not work on Mac, we need to
// include the whole header.
// See http://lists.trolltech.com/qt-interest/2008-07/thread00798-0.html
// for an explanation.
#include "QXmppGlobal.h"

#include <QXmlStreamWriter>

class QByteArray;
class QDateTime;
class QDomElement;
class QString;

/// \brief The QXmppUtils class contains static utility functions.
///
class QXMPP_EXPORT QXmppUtils
{
public:
    // XEP-0082: XMPP Date and Time Profiles
    static QDateTime datetimeFromString(QStringView str);
    /// \cond
    static QDateTime datetimeFromString(const QString &str);
    /// \endcond
    static QString datetimeToString(const QDateTime &dt);
    static int timezoneOffsetFromString(const QString &str);
    static QString timezoneOffsetToString(int secs);

    static QString jidToDomain(const QString &jid);
    static QString jidToResource(const QString &jid);
    static QString jidToUser(const QString &jid);
    static QString jidToBareJid(const QString &jid);

    static quint32 generateCrc32(const QByteArray &input);
    static QByteArray generateHmacMd5(const QByteArray &key, const QByteArray &text);
    static QByteArray generateHmacSha1(const QByteArray &key, const QByteArray &text);
    static int generateRandomInteger(int N);
    static QByteArray generateRandomBytes(int length);
    static QString generateStanzaUuid();
    static QString generateStanzaHash(int length = 36);
};

#endif  // QXMPPUTILS_H
