/*
 * Copyright (C) 2008-2014 The QXmpp developers
 *
 * Authors:
 *  Jeremy Lainé
 *  Manjeet Dahiya
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

#include <QDomDocument>
#include <QtTest>
#include "QXmppPasswordChecker.h"

template <class T>
static void parsePacket(T &packet, const QByteArray &xml)
{
    //qDebug() << "parsing" << xml;
    QDomDocument doc;
    QCOMPARE(doc.setContent(xml, true), true);
    QDomElement element = doc.documentElement();
    packet.parse(element);
}

template <class T>
static void serializePacket(T &packet, const QByteArray &xml)
{
    QBuffer buffer;
    buffer.open(QIODevice::ReadWrite);
    QXmlStreamWriter writer(&buffer);
    packet.toXml(&writer);
    qDebug() << "expect " << xml;
    qDebug() << "writing" << buffer.data();
    QCOMPARE(buffer.data(), xml);
}

class TestPasswordChecker : public QXmppPasswordChecker
{
public:
    void addCredentials(const QString &user, const QString &password)
    {
        m_credentials.insert(user, password);
    };

    /// Retrieves the password for the given username.
    QXmppPasswordReply::Error getPassword(const QXmppPasswordRequest &request, QString &password)
    {
        if (m_credentials.contains(request.username()))
        {
            password = m_credentials.value(request.username());
            return QXmppPasswordReply::NoError;
        } else {
            return QXmppPasswordReply::AuthorizationError;
        }
    };

    /// Returns whether getPassword() is enabled.
    bool hasGetPassword() const
    {
        return true;
    };

private:
    QMap<QString, QString> m_credentials;
};
