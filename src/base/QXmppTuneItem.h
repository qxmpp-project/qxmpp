// SPDX-FileCopyrightText: 2020 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

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
