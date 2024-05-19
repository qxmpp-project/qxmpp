// SPDX-FileCopyrightText: 2023 Linus Jahn <lnj@kaidan.im>
// SPDX-FileCopyrightText: 2024 Filipe Azevedo <pasnox@gmail.com>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppAccountMigrationManager.h"

#include "QXmppClient.h"
#include "QXmppConstants_p.h"
#include "QXmppPromise.h"
#include "QXmppTask.h"
#include "QXmppUtils_p.h"

#include "StringLiterals.h"

#include <QDomElement>

using namespace QXmpp;
using namespace QXmpp::Private;

// Utilities for account data extensions

struct XmlElementId {
    QString tagName;
    QString xmlns;

    static XmlElementId fromDom(const QDomElement &el) { return { el.tagName(), el.namespaceURI() }; }

    bool operator==(const XmlElementId &other) const
    {
        return tagName == other.tagName && xmlns == other.xmlns;
    }
};

#ifndef QXMPP_DOC
template<>
struct std::hash<XmlElementId> {
    std::size_t operator()(const XmlElementId &m) const noexcept
    {
        std::size_t h1 = std::hash<QString> {}(m.tagName);
        std::size_t h2 = std::hash<QString> {}(m.xmlns);
        return h1 ^ (h2 << 1);
    }
};
#endif

using AnyParser = QXmppExportData::ExtensionParser<std::any>;
using AnySerializer = QXmppExportData::ExtensionSerializer<std::any>;

static std::unordered_map<XmlElementId, AnyParser> &accountDataParsers()
{
    thread_local static std::unordered_map<XmlElementId, AnyParser> registry;
    return registry;
}

static std::unordered_map<std::type_index, AnySerializer> &accountDataSerializers()
{
    thread_local static std::unordered_map<std::type_index, AnySerializer> registry;
    return registry;
}

struct QXmppExportDataPrivate : QSharedData {
    QString accountJid;
    std::unordered_map<std::type_index, std::any> extensions;
};

QXmppExportData::QXmppExportData()
    : d(new QXmppExportDataPrivate())
{
}

QXMPP_PRIVATE_DEFINE_RULE_OF_SIX(QXmppExportData)

std::variant<QXmppExportData, QXmppError> QXmppExportData::fromDom(const QDomElement &el)
{
    if (el.tagName() != u"account-data" || el.namespaceURI() != ns_qxmpp_export) {
        return QXmppError { u"Invalid XML document provided."_s, {} };
    }

    const auto &parsers = accountDataParsers();

    QXmppExportData data;
    data.setAccountJid(el.attribute(u"jid"_s));

    for (const auto &extension : iterChildElements(el)) {
        const auto parser = parsers.find(XmlElementId::fromDom(extension));
        if (parser != parsers.end()) {
            const auto &[_, parse] = *parser;

            auto result = parse(extension);
            if (std::holds_alternative<QXmppError>(result)) {
                return std::get<QXmppError>(std::move(result));
            }

            auto extensionValue = std::get<std::any>(std::move(result));
            data.d->extensions.emplace(extensionValue.type(), std::move(extensionValue));
        }
    }

    return data;
}

void QXmppExportData::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartDocument();
    writer->writeStartElement(QSL65("account-data"));
    writer->writeDefaultNamespace(toString65(ns_qxmpp_export));
    writer->writeAttribute(QSL65("jid"), d->accountJid);

    const auto &serializers = accountDataSerializers();
    for (const auto &[typeIndex, extension] : std::as_const(d->extensions)) {
        const auto serializer = serializers.find(typeIndex);
        if (serializer != serializers.end()) {
            const auto &[_, serialize] = *serializer;

            serialize(extension, *writer);
        }
    }

    writer->writeEndElement();
    writer->writeEndDocument();
}

const QString &QXmppExportData::accountJid() const
{
    return d->accountJid;
}

void QXmppExportData::setAccountJid(const QString &jid)
{
    d->accountJid = jid;
}

const std::unordered_map<std::type_index, std::any> &QXmppExportData::extensions() const
{
    return d->extensions;
}

void QXmppExportData::setExtension(std::any value)
{
    d->extensions.emplace(std::type_index(value.type()), std::move(value));
}

void QXmppExportData::registerExtensionInternal(std::type_index type, ExtensionParser<std::any> parse, ExtensionSerializer<std::any> serialize, QStringView tagName, QStringView xmlns)
{
    accountDataParsers().emplace(XmlElementId { tagName.toString(), xmlns.toString() }, parse);
    accountDataSerializers().emplace(type, serialize);
}

struct QXmppAccountMigrationManagerPrivate {
    template<typename T = Success>
    using Result = std::variant<T, QXmppError>;

    struct ExtensionData {
        std::function<QXmppTask<Result<>>(std::any)> importFunction;
        std::function<QXmppTask<Result<std::any>>()> exportFunction;
    };

    std::unordered_map<std::type_index, ExtensionData> extensions;
};

///
/// \brief Allows to export and import account data.
///
/// You can use exportData() to start a data export. Afterwards you can use the exported data to
/// start a data import on another account using importData().
///
/// The data that is exported (or imported) is determined by the other registered client
/// extensions. They can register callbacks for export and import using registerExtension().
///
/// \ingroup Managers
/// \since QXmpp 1.8
///

///
/// \class QXmppAccountMigrationManager
///
/// This manager help migrating a user account into another server.
///

///
/// \typedef QXmppAccountMigrationManager::Result<T>
///
/// Contains T or QXmppError.
///

///
/// \fn QXmppAccountMigrationManager::registerExportData(ImportFunc importFunc, ExportFunc exportFunc)
///
/// Registers a data type that can be imported to an account using importFunc and generated using
/// exportFunc.
///
/// The functions are used when importData() or exportData() is called. You can unregister the
/// functions using unregisterExportData().
///
/// The data type MUST also be registered via QXmppExportData::registerExtension() so it can be
/// serialized.
///

///
/// \fn QXmppAccountMigrationManager::unregisterExportData()
///
/// Unregisters a data type.
///

///
/// Constructs an account migration manager.
///
QXmppAccountMigrationManager::QXmppAccountMigrationManager()
    : d(std::make_unique<QXmppAccountMigrationManagerPrivate>())
{
}

QXmppAccountMigrationManager::~QXmppAccountMigrationManager() = default;

///
/// Imports QXmppExportData to the currently connected account.
///
QXmppTask<QXmppAccountMigrationManager::Result<>> QXmppAccountMigrationManager::importData(const QXmppExportData &account)
{
    if (account.extensions().empty()) {
        return makeReadyTask<Result<>>({});
    }

    QXmppPromise<Result<>> promise;

    auto counter = std::make_shared<int>(account.extensions().size());

    for (const auto &[typeIndex, value] : account.extensions()) {
        auto extensionType = d->extensions.find(typeIndex);

        // There is no registered extension to import this data type
        if (extensionType == d->extensions.cend()) {
            if (--(*counter) == 0) {
                promise.finish(Success());
            }
            continue;
        }

        auto &[_, extension] = *extensionType;
        extension.importFunction(value).then(this, [promise, counter](auto &&result) mutable {
            if (promise.task().isFinished()) {
                return;
            }

            if (std::holds_alternative<QXmppError>(result)) {
                return promise.finish(std::get<QXmppError>(std::move(result)));
            }

            if ((--(*counter)) == 0) {
                promise.finish(Success());
            }
        });
    }

    return promise.task();
}

///
/// Creates a data export of the current account.
///
QXmppTask<QXmppAccountMigrationManager::Result<QXmppExportData>> QXmppAccountMigrationManager::exportData()
{
    struct State {
        QXmppPromise<Result<QXmppExportData>> p;
        QXmppExportData data;
        uint counter = 0;
    };

    auto state = std::make_shared<State>();
    state->data.setAccountJid(client()->configuration().jidBare());
    state->counter = d->extensions.size();

    // early exit
    if (d->extensions.empty()) {
        return makeReadyTask<Result<QXmppExportData>>(std::move(state->data));
    }

    for (const auto &[dataTypeIndex, extension] : std::as_const(d->extensions)) {
        extension.exportFunction().then(this, [state](auto &&result) mutable {
            auto &[p, data, counter] = *state;

            if (p.task().isFinished()) {
                return;
            }

            if (std::holds_alternative<QXmppError>(result)) {
                return p.finish(std::move(std::get<QXmppError>(result)));
            }

            data.setExtension(std::get<std::any>(std::move(result)));

            if (--counter == 0) {
                p.finish(std::move(data));
            }
        });
    }

    return state->p.task();
}

void QXmppAccountMigrationManager::registerMigrationDataInternal(
    std::type_index dataType,
    std::function<QXmppTask<Result<>>(std::any)> importFunc,
    std::function<QXmppTask<Result<std::any>>()> exportFunc)
{
    d->extensions.emplace(dataType, QXmppAccountMigrationManagerPrivate::ExtensionData { importFunc, exportFunc });
}

void QXmppAccountMigrationManager::unregisterMigrationDataInternal(std::type_index dataType)
{
    d->extensions.erase(dataType);
}
