// SPDX-FileCopyrightText: 2022 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "QXmppGlobal.h"

#include <optional>

#include <QSharedDataPointer>

class QDomElement;
class QMimeType;
class QXmlStreamWriter;
class QXmppThumbnailPrivate;

class QXMPP_EXPORT QXmppThumbnail
{
public:
    QXmppThumbnail();
    QXmppThumbnail(const QXmppThumbnail &);
    QXmppThumbnail(QXmppThumbnail &&) noexcept;
    ~QXmppThumbnail();

    QXmppThumbnail &operator=(const QXmppThumbnail &);
    QXmppThumbnail &operator=(QXmppThumbnail &&) noexcept;

    const QString &uri() const;
    void setUri(const QString &newUri);

    const QMimeType &mediaType() const;
    void setMediaType(const QMimeType &);

    std::optional<uint32_t> width() const;
    void setWidth(std::optional<uint32_t>);

    std::optional<uint32_t> height() const;
    void setHeight(std::optional<uint32_t>);

    /// \cond
    bool parse(const QDomElement &);
    void toXml(QXmlStreamWriter *writer) const;
    /// \endcond

private:
    QSharedDataPointer<QXmppThumbnailPrivate> d;
};
