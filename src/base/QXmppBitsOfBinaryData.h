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

#ifndef QXMPPBITSOFBINARYDATA_H
#define QXMPPBITSOFBINARYDATA_H

#include "QXmppGlobal.h"

#include <QSharedDataPointer>

class QDomElement;
class QMimeType;
class QXmlStreamWriter;
class QXmppBitsOfBinaryDataPrivate;
class QXmppBitsOfBinaryContentId;

/// \class QXmppBitsOfBinaryData represents a data element for \xep{0231}: Bits
/// of Binary. It can be used as an extension in other stanzas.
///
/// \see QXmppBitsOfBinaryIq, QXmppBitsOfBinaryDataList
///
/// \since QXmpp 1.2

class QXMPP_EXPORT QXmppBitsOfBinaryData
{
public:
    QXmppBitsOfBinaryData();
    QXmppBitsOfBinaryData(const QXmppBitsOfBinaryData &);
    ~QXmppBitsOfBinaryData();

    QXmppBitsOfBinaryData &operator=(const QXmppBitsOfBinaryData &);

    QXmppBitsOfBinaryContentId cid() const;
    void setCid(const QXmppBitsOfBinaryContentId &cid);

    int maxAge() const;
    void setMaxAge(int maxAge);

    QMimeType contentType() const;
    void setContentType(const QMimeType &contentType);

    QByteArray data() const;
    void setData(const QByteArray &data);

    bool static isBitsOfBinaryData(const QDomElement &element);

    /// \cond
    void parseElementFromChild(const QDomElement &dataElement);
    void toXmlElementFromChild(QXmlStreamWriter *writer) const;
    /// \endcond

    bool operator==(const QXmppBitsOfBinaryData &other) const;

private:
    QSharedDataPointer<QXmppBitsOfBinaryDataPrivate> d;
};

#endif  // QXMPPBITSOFBINARYDATA_H
