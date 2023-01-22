// SPDX-FileCopyrightText: 2012 Jeremy Lainé <jeremy.laine@m4x.org>
// SPDX-FileCopyrightText: 2012 Manjeet Dahiya <manjeetdahiya@gmail.com>
// SPDX-FileCopyrightText: 2020 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef TESTS_UTIL_H
#define TESTS_UTIL_H

#include "QXmppPasswordChecker.h"
#include "QXmppTask.h"

#include <memory>
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
    if constexpr (std::is_same_v<String, QString> || std::is_same_v<String, QByteArray>) {
        QVERIFY_RV(doc.setContent(xml, true), "XML is not valid");
    } else {
        QVERIFY_RV(doc.setContent(QString(xml), true), "XML is not valid");
    }
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
    // qDebug() << "parsing" << xml;
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

template<typename T, typename Variant>
T expectVariant(Variant var)
{
    using namespace std::string_literals;
    [&]() {
        std::string message =
            "Variant ("s + typeid(Variant).name() +
            ") contains wrong type ("s + std::to_string(var.index()) +
            "); expected '"s + typeid(T).name() + "'."s;
        QVERIFY2(std::holds_alternative<T>(var), message.c_str());
    }();
    return std::get<T>(std::move(var));
}

template<typename T, typename Input>
T expectFutureVariant(const QFuture<Input> &future)
{
    [&]() {
        QVERIFY(future.isFinished());
    }();
    return expectVariant<T>(future.result());
}

template<typename T, typename Input>
T expectFutureVariant(QXmppTask<Input> &future)
{
#define return \
    return     \
    {          \
    }
    QVERIFY(future.isFinished());
#undef return
    return expectVariant<T>(future.result());
}

template<typename T>
T wait(const QFuture<T> &future)
{
    auto watcher = std::make_unique<QFutureWatcher<T>>();
    QSignalSpy spy(watcher.get(), &QFutureWatcherBase::finished);
    watcher->setFuture(future);
    [&]() { QVERIFY(spy.wait()); }();
    if constexpr (!std::is_same_v<T, void>) {
        return future.result();
    }
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
