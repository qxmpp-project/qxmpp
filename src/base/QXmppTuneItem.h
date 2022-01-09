/*
 * Copyright (C) 2008-2022 The QXmpp developers
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

#ifndef QXMPPTUNEITEM_H
#define QXMPPTUNEITEM_H

#include "QXmppPubSubItem.h"

#include <QSharedDataPointer>

class QXmppTuneItemPrivate;
class QUrl;

class QXMPP_EXPORT QXmppTuneItem : public QXmppPubSubItem
{
public:
    QXmppTuneItem();
    QXmppTuneItem(const QXmppTuneItem &other);
    ~QXmppTuneItem();

    QXmppTuneItem &operator=(const QXmppTuneItem &other);

    QString artist() const;
    void setArtist(const QString &artist);

    quint16 length() const;
    void setLength(quint16 length);

    quint8 rating() const;
    void setRating(quint8 rating);

    QString source() const;
    void setSource(const QString &source);

    QString title() const;
    void setTitle(const QString &title);

    QString track() const;
    void setTrack(const QString &track);

    QUrl uri() const;
    void setUri(const QUrl &uri);

    static bool isItem(const QDomElement &itemElement);

protected:
    /// \cond
    void parsePayload(const QDomElement &payloadElement) override;
    void serializePayload(QXmlStreamWriter *writer) const override;
    /// \endcond

private:
    QSharedDataPointer<QXmppTuneItemPrivate> d;
};

Q_DECLARE_METATYPE(QXmppTuneItem)

#endif  // QXMPPTUNEITEM_H
