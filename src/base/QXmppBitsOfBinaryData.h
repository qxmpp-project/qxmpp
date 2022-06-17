// SPDX-FileCopyrightText: 2019 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPBITSOFBINARYDATA_H
#define QXMPPBITSOFBINARYDATA_H

#include "QXmppGlobal.h"

#include <QSharedDataPointer>

class QDomElement;
class QMimeType;
class QXmlStreamWriter;
class QXmppBitsOfBinaryDataPrivate;
class QXmppBitsOfBinaryContentId;

class QXMPP_EXPORT QXmppBitsOfBinaryData
{
public:
    QXmppBitsOfBinaryData();
    QXmppBitsOfBinaryData(const QXmppBitsOfBinaryData &);
    QXmppBitsOfBinaryData(QXmppBitsOfBinaryData &&);
    ~QXmppBitsOfBinaryData();

    QXmppBitsOfBinaryData &operator=(const QXmppBitsOfBinaryData &);
    QXmppBitsOfBinaryData &operator=(QXmppBitsOfBinaryData &&);

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
