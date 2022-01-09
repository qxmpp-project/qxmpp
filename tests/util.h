/*
 * Copyright (C) 2008-2022 The QXmpp developers
 *
 * Authors:
 *  Jeremy Lainé
 *  Manjeet Dahiya
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

#ifndef TESTS_UTIL_H
#define TESTS_UTIL_H

#include "QXmppPasswordChecker.h"

#include <variant>

#include <QDomDocument>
#include <QtTest>

// QVERIFY2 with empty return value (return {};)
#define QVERIFY_RV(statement, description)                                       \
    if (!QTest::qVerify(statement, #statement, description, __FILE__, __LINE__)) \
        return {};

template<typename String>
inline QDomElement xmlToDom(const String &xml)
{
    QDomDocument doc;
    QVERIFY_RV(doc.setContent(xml, true), "XML is not valid");
    return doc.documentElement();
}

template<typename T>
static QByteArray packetToXml(const T &packet)
{
    QBuffer buffer;
    buffer.open(QIODevice::ReadWrite);
    QXmlStreamWriter writer(&buffer);
    packet.toXml(&writer);
    auto data = buffer.data();
    data.replace(u'\'', "&apos;");
    return data;
}

template<class T>
static void parsePacket(T &packet, const QByteArray &xml)
{
    //qDebug() << "parsing" << xml;
    packet.parse(xmlToDom(xml));
}

template<class T>
static void serializePacket(T &packet, const QByteArray &xml)
{
    auto processedXml = xml;
    processedXml.replace(u'\'', u'"');

    const auto data = packetToXml(packet);
    qDebug() << "expect " << processedXml;
    qDebug() << "writing" << data;
    QCOMPARE(data, processedXml);
}

template<class T>
QDomElement writePacketToDom(T packet)
{
    QBuffer buffer;
    buffer.open(QIODevice::ReadWrite);
    QXmlStreamWriter writer(&buffer);
    packet.toXml(&writer);

    QDomDocument doc;
    doc.setContent(buffer.data(), true);

    return doc.documentElement();
}

template<typename T, typename Input>
T expectFutureVariant(const QFuture<Input> &future)
{
#define return \
    return { }
    QVERIFY(future.isFinished());
    QVERIFY2(std::holds_alternative<T>(future.result()), "Variant contains wrong type!");
#undef return
    return std::get<T>(future.result());
}

class TestPasswordChecker : public QXmppPasswordChecker
{
public:
    void addCredentials(const QString &user, const QString &password)
    {
        m_credentials.insert(user, password);
    };

    /// Retrieves the password for the given username.
    QXmppPasswordReply::Error getPassword(const QXmppPasswordRequest &request, QString &password) override
    {
        if (m_credentials.contains(request.username())) {
            password = m_credentials.value(request.username());
            return QXmppPasswordReply::NoError;
        } else {
            return QXmppPasswordReply::AuthorizationError;
        }
    };

    /// Returns whether getPassword() is enabled.
    bool hasGetPassword() const override
    {
        return true;
    };

private:
    QMap<QString, QString> m_credentials;
};

#endif  // TESTS_UTIL_H
