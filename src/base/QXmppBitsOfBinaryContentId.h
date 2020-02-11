/*
 * Copyright (C) 2008-2020 The QXmpp developers
 *
 * Author:
 *  Linus Jahn
 *
 * Source:
 *  https://github.com/qxmpp-project/qxmpp
 *
 * This file is a part of QXmpp library.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 */

#ifndef QXMPPBITSOFBINARYCONTENTID_H
#define QXMPPBITSOFBINARYCONTENTID_H

#include "QXmppGlobal.h"

#include <QCryptographicHash>
#include <QSharedDataPointer>

class QXmppBitsOfBinaryContentIdPrivate;

/// \class QXmppBitsOfBinaryContentId represents a link to or an identifier of
/// \xep{0231}: Bits of Binary data.
///
/// \since QXmpp 1.2

class QXMPP_EXPORT QXmppBitsOfBinaryContentId
{
public:
    static QXmppBitsOfBinaryContentId fromCidUrl(const QString &input);
    static QXmppBitsOfBinaryContentId fromContentId(const QString &input);

    QXmppBitsOfBinaryContentId();
    QXmppBitsOfBinaryContentId(const QXmppBitsOfBinaryContentId &cid);
    ~QXmppBitsOfBinaryContentId();

    QXmppBitsOfBinaryContentId &operator=(const QXmppBitsOfBinaryContentId &other);

    QString toContentId() const;
    QString toCidUrl() const;

    QByteArray hash() const;
    void setHash(const QByteArray &hash);

    QCryptographicHash::Algorithm algorithm() const;
    void setAlgorithm(QCryptographicHash::Algorithm algo);

    bool isValid() const;

    static bool isBitsOfBinaryContentId(const QString &uri, bool checkIsCidUrl = false);

    bool operator==(const QXmppBitsOfBinaryContentId &other) const;

private:
    QSharedDataPointer<QXmppBitsOfBinaryContentIdPrivate> d;
};

#endif  // QXMPPBITSOFBINARYCONTENTID_H
