/*
 * Copyright (C) 2010 Bolloré telecom
 *
 * Author:
 *	Jeremy Lainé
 *
 * Source:
 *	http://code.google.com/p/qxmpp
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

#ifndef QXMPPSTREAMINITIATIONIQ_H
#define QXMPPSTREAMINITIATIONIQ_H

#include <QDateTime>

#include "QXmppIq.h"

class QDomElement;
class QXmlStreamWriter;

class QXmppStreamInitiationIq : public QXmppIq
{
public:
    enum Profile {
        None = 0,
        FileTransfer,
    };

    QString getMimeType() const;
    void setMimeType(const QString &mimeType);

    QXmppStreamInitiationIq::Profile getProfile() const;
    void setProfile(QXmppStreamInitiationIq::Profile profile);

    QString getSiId() const;
    void setSiId(const QString &id);

    QXmppElementList getSiItems() const;
    void setSiItems(const QXmppElementList &items);

    void parse(QDomElement &element);
    void toXmlElementFromChild(QXmlStreamWriter *writer) const;
    static bool isStreamInitiationIq(QDomElement &element);

private:
    QString m_mimeType;
    Profile m_profile;
    QString m_siId;
    QXmppElementList m_siItems;
};

#endif
