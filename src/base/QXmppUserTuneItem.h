// SPDX-FileCopyrightText: 2020 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPUSERTUNEITEM_H
#define QXMPPUSERTUNEITEM_H

#include "QXmppPubSubBaseItem.h"

#include <chrono>
#include <optional>

#include <QSharedDataPointer>
#include <QTime>

class QXmppTuneItemPrivate;
class QUrl;

class QXMPP_EXPORT QXmppTuneItem : public QXmppPubSubBaseItem
{
public:
    QXmppTuneItem();
    QXmppTuneItem(const QXmppTuneItem &other);
    QXmppTuneItem(QXmppTuneItem &&);
    ~QXmppTuneItem();

    QXmppTuneItem &operator=(const QXmppTuneItem &other);
    QXmppTuneItem &operator=(QXmppTuneItem &&);

    QString artist() const;
    void setArtist(QString artist);

    std::optional<quint16> length() const;
    void setLength(std::optional<quint16> length);
    inline QTime lengthAsTime() const
    {
        if (auto len = length()) {
            return QTime::fromMSecsSinceStartOfDay(len.value() * 1000);
        }
        return {};
    }
    inline void setLength(const QTime &time)
    {
        if (time.isValid()) {
            setLength(time.msecsSinceStartOfDay() / 1000);
        }
        setLength(std::optional<quint16>());
    }
    inline std::optional<std::chrono::seconds> lengthAsDuration() const
    {
        if (auto len = length()) {
            return std::chrono::seconds(*len);
        }
        return {};
    }
    inline void setLength(std::optional<std::chrono::seconds> time)
    {
        if (time) {
            setLength(quint16(time->count()));
        }
        setLength(std::optional<quint16>());
    }

    std::optional<quint8> rating() const;
    void setRating(std::optional<quint8> rating);

    QString source() const;
    void setSource(QString source);

    QString title() const;
    void setTitle(QString title);

    QString track() const;
    void setTrack(QString track);

    QUrl uri() const;
    void setUri(QUrl uri);

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

#endif  // QXMPPUSERTUNEITEM_H
