// SPDX-FileCopyrightText: 2019 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPBITSOFBINARYCONTENTID_H
#define QXMPPBITSOFBINARYCONTENTID_H

#include "QXmppGlobal.h"

#include <QCryptographicHash>
#include <QSharedDataPointer>

class QXmppBitsOfBinaryContentIdPrivate;

class QXMPP_EXPORT QXmppBitsOfBinaryContentId
{
public:
    static QXmppBitsOfBinaryContentId fromCidUrl(const QString &input);
    static QXmppBitsOfBinaryContentId fromContentId(const QString &input);

    QXmppBitsOfBinaryContentId();
    QXmppBitsOfBinaryContentId(const QXmppBitsOfBinaryContentId &cid);
    QXmppBitsOfBinaryContentId(QXmppBitsOfBinaryContentId &&);
    ~QXmppBitsOfBinaryContentId();

    QXmppBitsOfBinaryContentId &operator=(const QXmppBitsOfBinaryContentId &other);
    QXmppBitsOfBinaryContentId &operator=(QXmppBitsOfBinaryContentId &&);

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
