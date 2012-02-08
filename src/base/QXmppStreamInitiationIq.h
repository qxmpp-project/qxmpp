/*
 * Copyright (C) 2008-2011 The QXmpp developers
 *
 * Author:
 *  Jeremy Lainé
 *
 * Source:
 *  http://code.google.com/p/qxmpp
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

    QString mimeType() const;
    void setMimeType(const QString &mimeType);

    QXmppStreamInitiationIq::Profile profile() const;
    void setProfile(QXmppStreamInitiationIq::Profile profile);

    QString siId() const;
    void setSiId(const QString &id);

    QXmppElementList siItems() const;
    void setSiItems(const QXmppElementList &items);

    static bool isStreamInitiationIq(const QDomElement &element);

protected:
    /// \cond
    void parseElementFromChild(const QDomElement &element);
    void toXmlElementFromChild(QXmlStreamWriter *writer) const;
    /// \endcond

private:
    QString m_mimeType;
    Profile m_profile;
    QString m_siId;
    QXmppElementList m_siItems;
};

#endif
