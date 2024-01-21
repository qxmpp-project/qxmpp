// SPDX-FileCopyrightText: 2023 Linus Jahn <lnj@kaidan.im>
// SPDX-FileCopyrightText: 2024 Filipe Azevedo <pasnox@gmail.com>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPACCOUNTMIGRATIONMANAGER_H
#define QXMPPACCOUNTMIGRATIONMANAGER_H

#include "QXmppClientExtension.h"
#include "QXmppError.h"
#include "QXmppFutureUtils_p.h"
#include "QXmppTask.h"

#include <any>
#include <typeindex>

template<typename T>
class QXmppTask;

struct QXmppAccountDataPrivate;

class QXMPP_EXPORT QXmppAccountData
{
public:
    QXMPP_PRIVATE_DECLARE_RULE_OF_SIX(QXmppAccountData)
    QXmppAccountData();
    explicit QXmppAccountData(const QString &sourceBareJid);

    static std::variant<QXmppAccountData, QXmppError> fromDom(const QDomElement &);
    void toXml(QXmlStreamWriter *) const;

    const QString &sourceBareJid() const;
    void setSourceBareJid(const QString &jid);

    const QVector<std::any> &extensions() const;
    void setExtensions(const QVector<std::any> &);
    void addExtension(const std::any &);

    int extensionsCount() const;

    template<typename T>
    using ExtensionParserResult = std::variant<T, QXmppError>;
    template<typename T>
    using ExtensionParser = ExtensionParserResult<T> (*)(const QDomElement &);
    using AnyParser = ExtensionParser<std::any>;

    template<typename T>
    using ExtensionSerializerResult = void;
    template<typename T>
    using ExtensionSerializer = ExtensionSerializerResult<T> (*)(const T &, QXmlStreamWriter &);
    using AnySerializer = ExtensionSerializer<std::any>;

    template<typename T, ExtensionParser<T> parse, ExtensionSerializer<T> serialize>
    static void registerExtension(QStringView tagName, QStringView xmlns)
    {
        using namespace QXmpp::Private;

        AnyParser parseAny = [](const QDomElement &el) {
            return std::visit(overloaded {
                                  [](T data) -> ExtensionParserResult<std::any> { return std::move(data); },
                                  [](QXmppError e) -> ExtensionParserResult<std::any> { return std::move(e); },
                              },
                              parse(el));
        };

        AnySerializer serializeAny = [](const std::any &data, QXmlStreamWriter &w) {
            return serialize(std::any_cast<T>(data), w);
        };

        registerExtensionInternal(std::type_index(typeid(T)), parseAny, serializeAny, tagName, xmlns);
    }

private:
    static void registerExtensionInternal(std::type_index, AnyParser, AnySerializer, QStringView tagName, QStringView xmlns);

    QSharedDataPointer<QXmppAccountDataPrivate> d;
};

struct QXmppAccountMigrationManagerPrivate;

class QXMPP_EXPORT QXmppAccountMigrationManager : public QXmppClientExtension
{
    Q_OBJECT

    friend struct QXmppAccountMigrationManagerPrivate;

public:
    using ImportResult = std::variant<QXmpp::Success, QXmppError>;
    using ExportResult = std::variant<QXmppAccountData, QXmppError>;

    using ImportDataResult = std::variant<QXmpp::Success, QXmppError>;
    using ImportDataTask = QXmppTask<ImportDataResult>;
    template<typename DataType>
    using ExportDataResult = std::variant<DataType, QXmppError>;
    template<typename DataType>
    using ExportDataTask = QXmppTask<ExportDataResult<DataType>>;

    QXmppAccountMigrationManager();
    ~QXmppAccountMigrationManager() override;

    QXmppTask<ImportResult> importData(const QXmppAccountData &account);
    QXmppTask<ExportResult> exportData();

    template<typename DataType, typename ImportFunc, typename ExportFunc>
    void registerExtension(ImportFunc importFunc, ExportFunc exportFunc);

    template<typename DataType>
    void unregisterExtension();

    std::size_t registeredExtensionsCount() const;

private:
    using ImportAnyResult = ImportDataResult;
    using ImportAnyTask = QXmppTask<ImportDataResult>;
    using ImportAnyCallback = std::function<ImportAnyTask(std::any)>;
    using ExportAnyResult = ExportDataResult<std::any>;
    using ExportAnyTask = QXmppTask<ExportAnyResult>;
    using ExportAnyCallback = std::function<ExportAnyTask()>;

    void registerMigrationDataInternal(std::type_index dataType, ImportAnyCallback, ExportAnyCallback);
    void unregisterMigrationDataInternal(std::type_index dataType);

    std::unique_ptr<QXmppAccountMigrationManagerPrivate> d;
};

template<typename DataType, typename ImportFunc, typename ExportFunc>
void QXmppAccountMigrationManager::registerExtension(ImportFunc importFunc, ExportFunc exportFunc)
{
    using namespace QXmpp::Private;

    static_assert(std::is_constructible_v<std::function<ImportDataTask(DataType)>, ImportFunc>);
    static_assert(std::is_constructible_v<std::function<ExportDataTask<DataType>()>, ExportFunc>);
    // static_assert(std::is_invocable_v<ImportFunc(DataType)>);
    // static_assert(std::is_invocable_v<ExportFunc()>);
    static_assert(std::is_same_v<std::result_of_t<ImportFunc(DataType)>, ImportDataTask>);
    static_assert(std::is_same_v<std::result_of_t<ExportFunc()>, ExportDataTask<DataType>>);
    static_assert(std::is_same_v<first_argument_t<ImportFunc>, DataType>);

    auto importInternal = [importFunc = std::move(importFunc)](std::any data) -> ImportAnyTask {
        Q_ASSERT(std::type_index(data.type()) == std::type_index(typeid(DataType)));
        return importFunc(std::any_cast<DataType>(std::move(data)));
    };

    auto exportInternal = [this, exportFunc = std::move(exportFunc)]() -> ExportAnyTask {
        return chain<ExportAnyResult>(exportFunc(), this, [](ExportDataResult<DataType> &&result) {
            return std::visit(overloaded {
                                  [](DataType data) -> ExportAnyResult { return std::any(std::move(data)); },
                                  [](QXmppError err) -> ExportAnyResult { return err; } },
                              std::move(result));
        });
    };

    registerMigrationDataInternal(std::type_index(typeid(DataType)), std::move(importInternal), std::move(exportInternal));
}

template<typename DataType>
void QXmppAccountMigrationManager::unregisterExtension()
{
    unregisterMigrationDataInternal(std::type_index(typeid(DataType)));
}

#endif
