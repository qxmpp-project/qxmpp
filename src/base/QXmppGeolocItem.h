// SPDX-FileCopyrightText: 2022 Cochise César <cochisecesar@zoho.com>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPGEOLOCITEM_H
#define QXMPPGEOLOCITEM_H

#include "QXmppPubSubBaseItem.h"

#include <optional>

#include <QSharedDataPointer>

class QXmppGeolocItemPrivate;

class QXMPP_EXPORT QXmppGeolocItem : public QXmppPubSubBaseItem
{
public:
    QXmppGeolocItem();
    QXmppGeolocItem(const QXmppGeolocItem &other);
    QXmppGeolocItem(QXmppGeolocItem &&);
    ~QXmppGeolocItem();

    QXmppGeolocItem &operator=(const QXmppGeolocItem &other);
    QXmppGeolocItem &operator=(QXmppGeolocItem &&);

    std::optional<double> accuracy() const;
    void setAccuracy(std::optional<double> accuracy);

    QString country() const;
    void setCountry(QString country);

    std::optional<double> latitude() const;
    void setLatitude(std::optional<double> lat);

    QString locality() const;
    void setLocality(QString locality);

    std::optional<double> longitude() const;
    void setLongitude(std::optional<double> lon);

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
