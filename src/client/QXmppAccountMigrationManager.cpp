// SPDX-FileCopyrightText: 2023 Linus Jahn <lnj@kaidan.im>
// SPDX-FileCopyrightText: 2024 Filipe Azevedo <pasnox@gmail.com>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppAccountMigrationManager.h"

#include "QXmppClient.h"
#include "QXmppPromise.h"
#include "QXmppTask.h"
#include "QXmppUtils_p.h"

#include <QDomElement>

// Utilities for account data extensions

struct XmlElementId
{
    QString tagName;
    QString xmlns;

    bool operator==(const XmlElementId &other) const
    {
        return tagName == other.tagName && xmlns == other.xmlns;
    }
};

template<>
struct std::hash<XmlElementId>
{
    std::size_t operator()(const XmlElementId &m) const noexcept
    {
        QtPrivate::QHashCombine hash;
        std::size_t seed = 0;
        seed = hash(seed, m.tagName);
        seed = hash(seed, m.xmlns);
        return seed;
    }
};

static std::unordered_map<XmlElementId, QXmppAccountData::AnyParser> &accountDataParsers()
{
    thread_local static std::unordered_map<XmlElementId, QXmppAccountData::AnyParser> registry;
    return registry;
}

static std::unordered_map<std::type_index, QXmppAccountData::AnySerializer> &accountDataSerializers()
{
    thread_local static std::unordered_map<std::type_index, QXmppAccountData::AnySerializer> registry;
    return registry;
}

struct QXmppAccountDataPrivate : public QSharedData
{
    QString sourceBareJid;
    QVector<std::any> extensions;
};

QXMPP_PRIVATE_DEFINE_RULE_OF_SIX(QXmppAccountData)

QXmppAccountData::QXmppAccountData()
    : QXmppAccountData(QString())
{
}

QXmppAccountData::QXmppAccountData(const QString &sourceBareJid)
    : d(new QXmppAccountDataPrivate())
{
    setSourceBareJid(sourceBareJid);
}

std::variant<QXmppAccountData, QXmppError> QXmppAccountData::fromDom(const QDomElement &el)
{
    if (el.tagName() != u"account-data") {
        return QXmppError { QStringLiteral("Not an account data document"), {} };
    }

    const auto &parsers = accountDataParsers();
    const auto nodes = el.childNodes();
    QXmppAccountData data;

    data.setSourceBareJid(el.attribute(QStringLiteral("sourceBareJid")));

    for (const auto &iq: QXmpp::Private::iterChildElements(el)) {
        const auto element = iq.firstChildElement();
        const auto parser = parsers.find(XmlElementId { element.tagName(), element.namespaceURI() });

        if (parser != parsers.end()) {
            const auto result = parser->second(iq);

            if (std::holds_alternative<QXmppError>(result)) {
                return std::get<QXmppError>(std::move(result));
            }

            data.addExtension(std::get<std::any>(std::move(result)));
        }
    }

    return data;
}

void QXmppAccountData::toXml(QXmlStreamWriter *writer) const
{
    const auto &serializers = accountDataSerializers();
    const auto &extensions = d->extensions;

    writer->writeStartDocument();
    writer->writeStartElement(QStringLiteral("org.qxmpp.account-data"), QStringLiteral("account-data"));
    writer->writeAttribute(QStringLiteral("sourceBareJid"), d->sourceBareJid);

    for (const auto &extension : extensions) {
        const auto serializer = serializers.find(std::type_index(extension.type()));

        if (serializer != serializers.end()) {
            serializer->second(extension, *writer);
        }
    }

    writer->writeEndElement();
    writer->writeEndDocument();
}

const QString &QXmppAccountData::sourceBareJid() const
{
    return d->sourceBareJid;
}

void QXmppAccountData::setSourceBareJid(const QString &jid)
{
    d->sourceBareJid = jid;
}

const QVector<std::any> &QXmppAccountData::extensions() const
{
    return d->extensions;
}

void QXmppAccountData::setExtensions(const QVector<std::any> &extensions)
{
    d->extensions = extensions;
}

void QXmppAccountData::addExtension(const std::any &extension)
{
    d->extensions.append(extension);
}

int QXmppAccountData::extensionsCount() const
{
    return d->extensions.size();
}

void QXmppAccountData::registerExtensionInternal(std::type_index type, AnyParser parse, AnySerializer serialize, QStringView tagName, QStringView xmlns)
{
    accountDataParsers().emplace(XmlElementId { tagName.toString(), xmlns.toString() }, parse);
    accountDataSerializers().emplace(type, serialize);
}

struct QXmppAccountMigrationManagerPrivate
{
    using ImportFunction = QXmppAccountMigrationManager::ImportAnyCallback;
    using ExportFunction = QXmppAccountMigrationManager::ExportAnyCallback;

    struct ExtensionData
    {
        ImportFunction importFunction;
        ExportFunction exportFunction;
    };

    std::unordered_map<std::type_index, ExtensionData> extensions;
};

///
/// \brief The QXmppAccountMigrationManager class provides access to account migration.
///
/// It allow to export server and client side data and import them into another server.
/// Use \c QXmppAccountMigrationManager::exportData to start export task.
/// When the application is ready to import the previously exported data use
/// \c QXmppAccountMigrationManager::importData to start the import task.
/// Note than before importing data, it is important to change the client credentials
/// with the new user account, failing to do that would result in an import error.
///
/// \ingroup Managers
/// \since QXmpp 1.7
///

///
/// \class QXmppAccountMigrationManager
///
/// This manager help migrating a user account into another server.
///

///
/// \typedef QXmppAccountMigrationManager::ClientDataExport
///
/// A callback signature to export client side data of the given jid.
///

///
/// \typedef QXmppAccountMigrationManager::ClientDataImport
///
/// A callback signature to import client side data to the given jid.
///

///
/// \typedef QXmppAccountMigrationManager::ExportResult
///
/// Contains Data<ClientData> for success or a QXmppError for an error.
///

///
/// \typedef QXmppAccountMigrationManager::ImportResult
///
/// Contains a QXmppError error.
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
/// \brief Create an import task.
/// \param data The data to import.
/// \return Returns QXmppError, if there is no error the error is empty.
///
QXmppTask<QXmppAccountMigrationManager::ImportResult> QXmppAccountMigrationManager::importData(const QXmppAccountData &account)
{
    const auto &extensions = account.extensions();
    QXmppPromise<ImportResult> promise;
    auto counter = std::make_shared<int>(extensions.size());

    for (auto it = extensions.cbegin(), end = extensions.cend(); it != end; ++it) {
        const auto index = std::type_index(it->type());
        const auto functions = d->extensions.find(index);

        // There is no registered extension to import this data type
        if (functions == d->extensions.cend()) {
            continue;
        }

        functions->second.importFunction(*it).then(this, [promise, counter](ImportAnyResult &&result) mutable {
            if (promise.task().isFinished()) {
                return;
            }

            if (std::holds_alternative<QXmppError>(result)) {
                return promise.finish(std::move(std::get<QXmppError>(result)));
            }

            if ((--(*counter)) == 0) {
                promise.finish(std::move(std::get<QXmpp::Success>(result)));
            }
        });
    }

    if (extensions.empty()) {
        promise.finish(QXmppError { tr("There is no data to import."), {} });
    }

    return promise.task();
}

///
/// \brief Create an export task.
/// \return Returns Data on success or QXmppError on error.
///
QXmppTask<QXmppAccountMigrationManager::ExportResult> QXmppAccountMigrationManager::exportData()
{
    QXmppPromise<ExportResult> promise;
    auto account = std::make_shared<QXmppAccountData>(client()->configuration().jidBare());
    auto counter = std::make_shared<int>(d->extensions.size());

    for (auto it = d->extensions.cbegin(), end = d->extensions.cend(); it != end; ++it) {
        it->second.exportFunction().then(this, [promise, account, counter](ExportAnyResult &&result) mutable {
            if (promise.task().isFinished()) {
                return;
            }

            if (std::holds_alternative<QXmppError>(result)) {
                return promise.finish(std::move(std::get<QXmppError>(result)));
            }

            account->addExtension(std::move(std::get<std::any>(result)));

            if ((--(*counter)) == 0) {
                promise.finish(*account);
            }
        });
    }

    if (d->extensions.empty()) {
        promise.finish(QXmppError { tr("There is no data to export."), {} });
    }

    return promise.task();
}

std::size_t QXmppAccountMigrationManager::registeredExtensionsCount() const
{
    return d->extensions.size();
}

void QXmppAccountMigrationManager::registerMigrationDataInternal(std::type_index dataType,
                                                                 ImportAnyCallback importFunc,
                                                                 ExportAnyCallback exportFunc)
{
    d->extensions.emplace(dataType, QXmppAccountMigrationManagerPrivate::ExtensionData { importFunc, exportFunc });
}

void QXmppAccountMigrationManager::unregisterMigrationDataInternal(std::type_index dataType)
{
    d->extensions.erase(dataType);
}
