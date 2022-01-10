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

#ifndef QXMPPGEOLOCITEM_H
#define QXMPPGEOLOCITEM_H

#include "QXmppPubSubItem.h"

#include <QSharedDataPointer>

class QXmppGeolocItemPrivate;
class QUrl;

class QXMPP_EXPORT QXmppGeolocItem : public QXmppPubSubItem
{
public:
    QXmppGeolocItem();
    QXmppGeolocItem(const QXmppGeolocItem &other);
    ~QXmppGeolocItem();

    QXmppGeolocItem &operator=(const QXmppGeolocItem &other);

    quint8 accuracy() const;
    void setAccuracy(const quint8 &accuracy);

    QString country() const;
    void setCountry(const QString &country);

    qfloat16 lat() const;
    void setLat(qfloat16 &lat);

    QString locality() const;
    void setLocality(const QString &locality);

    qfloat16 lon() const;
    void setLon(qfloat16 &lon);

    static bool isItem(const QDomElement &itemElement);

protected:
    /// \cond
    void parsePayload(const QDomElement &payloadElement) override;
    void serializePayload(QXmlStreamWriter *writer) const override;
    /// \endcond

private:
    QSharedDataPointer<QXmppGeolocItemPrivate> d;
};

Q_DECLARE_METATYPE(QXmppGeolocItem)

#endif  // QXMPPGEOLOCITEM_H
