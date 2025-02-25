// SPDX-FileCopyrightText: 2012 Jeremy Lainé <jeremy.laine@m4x.org>
// SPDX-FileCopyrightText: 2012 Manjeet Dahiya <manjeetdahiya@gmail.com>
// SPDX-FileCopyrightText: 2020 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef TESTS_UTIL_H
#define TESTS_UTIL_H

#include "QXmppError.h"
#include "QXmppPasswordChecker.h"
#include "QXmppTask.h"

#include "StringLiterals.h"

#include <any>
#include <memory>
#include <variant>

#include <QDomDocument>
#include <QtTest>

struct QXmppError;

// QVERIFY2 with empty return value (return {};)
#define QVERIFY_RV(statement, description)                                       \
    if (!QTest::qVerify(statement, #statement, description, __FILE__, __LINE__)) \
        return {};

#define VERIFY2(statement, description)                                                                           \
    if (!QTest::qVerify(bool(statement), #statement, static_cast<const char *>(description), __FILE__, __LINE__)) \
        throw std::runtime_error(description);

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
constexpr auto TimeZoneUTC = QTimeZone::Initialization::UTC;
#else
constexpr auto TimeZoneUTC = Qt::UTC;
#endif

template<typename String>
inline QDomDocument xmlToDomDoc(const String &xml, bool namespaceProcessing)
{
    QDomDocument doc;
    QString errorText;
    bool success = false;
    if constexpr (std::is_same_v<String, QString> || std::is_same_v<String, QByteArray>) {
        success = doc.setContent(xml, namespaceProcessing, &errorText);
    } else {
        success = doc.setContent(QString(xml), namespaceProcessing, &errorText);
    }
    if (!success) {
        qDebug() << "Parsing error:";
        qDebug().noquote() << xml;
        qDebug().noquote() << "Error:" << errorText;
        QTest::qFail("Invalid XML", __FILE__, __LINE__);
    }
    return doc;
}

template<typename String>
inline QDomElement xmlToDom(const String &xml)
{
    return xmlToDomDoc(xml, true).documentElement();
}

template<typename String>
inline QByteArray xmlToFormattedByteArray(const String &xml)
{
    return xmlToDomDoc(xml, false).toByteArray(4);
}

template<typename String>
QString rewriteXml(const String &inputXml)
{
    QString outputXml;
    QXmlStreamReader reader(inputXml);
    QXmlStreamWriter writer(&outputXml);
    while (reader.readNext() != QXmlStreamReader::EndDocument) {
        if (reader.hasError()) {
            qDebug() << "Parsing error:";
            qDebug().noquote() << inputXml;
            qDebug().noquote() << reader.error() << reader.errorString();
            throw std::exception();
        }

        // do not generate '<?xml version="1.0"?>'
        if (reader.tokenType() == QXmlStreamReader::StartDocument) {
            continue;
        }
        writer.writeCurrentToken(reader);
    }
    return outputXml;
}

template<typename String>
std::tuple<QString, QString> rewriteXmlWithoutStanzaId(const String &inputXml)
{
    QString outputXml;
    QString id;
    QXmlStreamReader reader(inputXml);
    QXmlStreamWriter writer(&outputXml);

    // find start
    reader.readNextStartElement();
    Q_ASSERT(reader.isStartElement());

    // write element, but without 'id' attribute
    writer.writeStartElement(reader.name().toString());
    const auto attributes = reader.attributes();
    for (const auto &attribute : attributes) {
        if (attribute.name() == u"id") {
            id = attribute.value().toString();
        } else {
            writer.writeAttribute(attribute);
        }
    }

    // copy rest of the xml
    while (reader.readNext() != QXmlStreamReader::EndDocument) {
        if (reader.hasError()) {
            qDebug() << "Parsing error:";
            qDebug().noquote() << inputXml;
            qDebug().noquote() << reader.error() << reader.errorString();
            throw std::exception();
        }

        // do not generate '<?xml version="1.0"?>'
        if (reader.tokenType() == QXmlStreamReader::StartDocument) {
            continue;
        }
        writer.writeCurrentToken(reader);
    }
    return { outputXml, id };
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

    // Remove newlines and needless spaces from raw strings.
    processedXml = processedXml.simplified();
    processedXml.replace("> <", "><");

    const auto data = packetToXml(packet);
    if (data != processedXml) {
        qDebug() << "expect " << processedXml;
        qDebug() << "writing" << data;
    }
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
    std::string message =
        "Variant ("s + typeid(Variant).name() +
        ") contains wrong type ("s + std::to_string(var.index()) +
        "); expected '"s + typeid(T).name() + "'."s;
    VERIFY2(std::holds_alternative<T>(var), message.c_str());
    return std::get<T>(std::move(var));
}

template<typename T, typename Input>
T expectFutureVariant(const QFuture<Input> &future)
{
    VERIFY2(future.isFinished(), "Future is still running!");
    return expectVariant<T>(future.result());
}

template<typename T, typename Input>
T expectFutureVariant(QXmppTask<Input> &task)
{
    VERIFY2(task.isFinished(), "Task is still running!");
    return expectVariant<T>(task.result());
}

template<typename T>
const T &unwrap(const std::optional<T> &v)
{
    VERIFY2(v.has_value(), "Expected value, got empty optional");
    return *v;
}

template<typename T>
T unwrap(std::optional<T> &&v)
{
    VERIFY2(v.has_value(), "Expected value, got empty optional");
    return *v;
}

template<typename T>
T unwrap(std::variant<T, QXmppError> &&v)
{
    if (std::holds_alternative<QXmppError>(v)) {
        auto message = u"Expected value, got error: %1."_s.arg(std::get<QXmppError>(v).description);
        VERIFY2(v.index() == 1, message.toLocal8Bit().constData());
    }
    return std::get<T>(std::move(v));
}

template<typename T>
const T &unwrap(const std::variant<T, QXmppError> &v)
{
    if (std::holds_alternative<QXmppError>(v)) {
        auto message = u"Expected value, got error: %1."_s.arg(std::get<QXmppError>(v).description);
        VERIFY2(v.index() == 1, message.toLocal8Bit().constData());
    }
    return std::get<T>(v);
}

template<typename T>
const T &unwrap(const std::any &v)
{
    VERIFY2(v.has_value(), "Expected non-empty std::any");
    VERIFY2(v.type() == typeid(T), "Got std::any with wrong type");
    return std::any_cast<T>(v);
}

template<typename T>
T unwrap(std::any &&v)
{
    VERIFY2(v.has_value(), "Expected non-empty std::any");
    VERIFY2(v.type() == typeid(T), "Got std::any with wrong type");
    return std::any_cast<T>(std::move(v));
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
