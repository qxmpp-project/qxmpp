// SPDX-FileCopyrightText: 2023 Linus Jahn <lnj@kaidan.im>
// SPDX-FileCopyrightText: 2024 Filipe Azevedo <pasnox@gmail.com>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPACCOUNTMIGRATIONMANAGER_H
#define QXMPPACCOUNTMIGRATIONMANAGER_H

#include "QXmppClientExtension.h"
#include "QXmppFutureUtils_p.h"
#include "QXmppTask.h"

#include <any>
#include <typeindex>

struct QXmppExportDataPrivate;

class QXMPP_EXPORT QXmppExportData
{
public:
    template<typename T = QXmpp::Success>
    using Result = std::variant<T, QXmppError>;

    QXmppExportData();
    QXMPP_PRIVATE_DECLARE_RULE_OF_SIX(QXmppExportData)

    static std::variant<QXmppExportData, QXmppError> fromDom(const QDomElement &);
    void toXml(QXmlStreamWriter *) const;

    const QString &accountJid() const;
    void setAccountJid(const QString &jid);

    template<typename T>
    using ExtensionParser = Result<T> (*)(const QDomElement &);
    template<typename T>
    using ExtensionSerializer = void (*)(const T &, QXmlStreamWriter &);

    template<typename T, ExtensionParser<T> parse, ExtensionSerializer<T> serialize>
    static void registerExtension(QStringView tagName, QStringView xmlns)
    {
        using namespace QXmpp::Private;
        using AnyParser = ExtensionParser<std::any>;
        using AnySerializer = ExtensionSerializer<std::any>;

        AnyParser parseAny = [](const QDomElement &el) {
            return std::visit(overloaded {
                                  [](T data) -> Result<std::any> { return std::move(data); },
                                  [](QXmppError e) -> Result<std::any> { return std::move(e); },
                              },
                              parse(el));
        };

        AnySerializer serializeAny = [](const std::any &data, QXmlStreamWriter &w) {
            return std::invoke(serialize, std::any_cast<const T &>(data), w);
        };

        registerExtensionInternal(std::type_index(typeid(T)), parseAny, serializeAny, tagName, xmlns);
    }

private:
    friend class QXmppAccountMigrationManager;
    friend class tst_QXmppAccountMigrationManager;

    const std::unordered_map<std::type_index, std::any> &extensions() const;
    void setExtension(std::any value);

    static void registerExtensionInternal(std::type_index, ExtensionParser<std::any>, ExtensionSerializer<std::any>, QStringView tagName, QStringView xmlns);

    QSharedDataPointer<QXmppExportDataPrivate> d;
};

struct QXmppAccountMigrationManagerPrivate;

class QXMPP_EXPORT QXmppAccountMigrationManager : public QXmppClientExtension
{
    Q_OBJECT

    friend struct QXmppAccountMigrationManagerPrivate;

public:
    template<typename T = QXmpp::Success>
    using Result = std::variant<T, QXmppError>;

    QXmppAccountMigrationManager();
    ~QXmppAccountMigrationManager() override;

    QXmppTask<Result<>> importData(const QXmppExportData &account);
    QXmppTask<Result<QXmppExportData>> exportData();

    template<typename DataType, typename ImportFunc, typename ExportFunc>
    void registerExportData(ImportFunc importFunc, ExportFunc exportFunc);

    template<typename DataType>
    void unregisterExportData();

private:
    void registerMigrationDataInternal(std::type_index dataType, std::function<QXmppTask<Result<>>(std::any)>, std::function<QXmppTask<Result<std::any>>()>);
    void unregisterMigrationDataInternal(std::type_index dataType);

    std::unique_ptr<QXmppAccountMigrationManagerPrivate> d;
};

template<typename DataType, typename ImportFunc, typename ExportFunc>
void QXmppAccountMigrationManager::registerExportData(ImportFunc importFunc, ExportFunc exportFunc)
{
    using namespace QXmpp::Private;

    static_assert(std::is_constructible_v<std::function<QXmppTask<Result<>>(const DataType &)>, ImportFunc>);
    static_assert(std::is_constructible_v<std::function<QXmppTask<Result<DataType>>()>, ExportFunc>);
    static_assert(std::is_invocable_v<ImportFunc, const DataType &>);
    static_assert(std::is_invocable_v<ExportFunc>);
    static_assert(std::is_same_v<first_argument_t<ImportFunc>, const DataType &>);

    auto importInternal = [importFunc = std::move(importFunc)](std::any data) -> QXmppTask<Result<>> {
        Q_ASSERT(std::type_index(data.type()) == std::type_index(typeid(DataType)));
        return importFunc(std::any_cast<DataType>(std::move(data)));
    };

    using AnyResult = std::variant<std::any, QXmppError>;
    auto exportInternal = [this, exportFunc = std::move(exportFunc)]() -> QXmppTask<AnyResult> {
        return chain<AnyResult>(exportFunc(), this, [](Result<DataType> &&result) {
            return std::visit(overloaded {
                                  [](DataType data) -> AnyResult { return std::any(std::move(data)); },
                                  [](QXmppError err) -> AnyResult { return err; } },
                              std::move(result));
        });
    };

    registerMigrationDataInternal(std::type_index(typeid(DataType)), std::move(importInternal), std::move(exportInternal));
}

template<typename DataType>
void QXmppAccountMigrationManager::unregisterExportData()
{
    unregisterMigrationDataInternal(std::type_index(typeid(DataType)));
}

#endif
