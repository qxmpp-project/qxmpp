// SPDX-FileCopyrightText: 2021 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPDATAFORMBASED_H
#define QXMPPDATAFORMBASED_H

#include "QXmppDataForm.h"

#include <optional>

class QXmppDataForm;

class QXMPP_EXPORT QXmppDataFormBase
{
public:
    virtual QXmppDataForm toDataForm() const;

protected:
    static bool fromDataForm(const QXmppDataForm &form, QXmppDataFormBase &parent);

    virtual QString formType() const = 0;
    virtual void parseForm(const QXmppDataForm &) = 0;
    virtual void serializeForm(QXmppDataForm &) const = 0;

    std::optional<quint32> parseUInt(const QVariant &variant)
    {
        bool ok;
        if (const auto result = variant.toString().toUInt(&ok); ok) {
            return result;
        }
        return std::nullopt;
    }

    std::optional<quint64> parseULongLong(const QVariant &variant)
    {
        bool ok;
        if (const auto result = variant.toString().toULongLong(&ok); ok) {
            return result;
        }
        return std::nullopt;
    }

    std::optional<bool> parseBool(const QVariant &variant)
    {
        if (variant.type() == QVariant::Bool) {
            return variant.toBool();
        }
        return std::nullopt;
    }

    template<typename T>
    static void serializeValue(QXmppDataForm &form, QXmppDataForm::Field::Type type, const QString &name, const T &value)
    {
        form.fields() << QXmppDataForm::Field(type, name, value);
    }

    template<typename T>
    static void serializeNullable(QXmppDataForm &form, QXmppDataForm::Field::Type type, QStringView name, const T &value)
    {
        if (!value.isNull()) {
            serializeValue(form, type, name.toString(), value);
        }
    }

    template<typename T>
    static void serializeEmptyable(QXmppDataForm &form, QXmppDataForm::Field::Type type, QStringView name, const T &value)
    {
        if (!value.isEmpty()) {
            serializeValue(form, type, name.toString(), value);
        }
    }

    template<typename T, typename ValueConverter = T (*)(T)>
    static void serializeOptional(
        QXmppDataForm &form, QXmppDataForm::Field::Type type, QStringView name, const std::optional<T> &optional, ValueConverter convert = [](T a) { return a; })
    {
        if (optional.has_value()) {
            serializeValue(form, type, name.toString(), convert(*optional));
        }
    }

    template<typename T>
    static void serializeOptionalNumber(QXmppDataForm &form, QXmppDataForm::Field::Type type, QStringView name, std::optional<T> optional)
    {
        if (optional.has_value()) {
            serializeValue(form, type, name.toString(), QString::number(*optional));
        }
    }

    static void serializeDatetime(QXmppDataForm &form, const QString &name, const QDateTime &datetime, QXmppDataForm::Field::Type type = QXmppDataForm::Field::TextSingleField);
};

class QXmppExtensibleDataFormBasePrivate;

class QXMPP_EXPORT QXmppExtensibleDataFormBase : public QXmppDataFormBase
{
public:
    QXmppExtensibleDataFormBase();
    /// \cond
    QXmppExtensibleDataFormBase(const QXmppExtensibleDataFormBase &);
    QXmppExtensibleDataFormBase(QXmppExtensibleDataFormBase &&);
    virtual ~QXmppExtensibleDataFormBase();

    QXmppExtensibleDataFormBase &operator=(const QXmppExtensibleDataFormBase &);
    QXmppExtensibleDataFormBase &operator=(QXmppExtensibleDataFormBase &&);
    /// \endcond

    QList<QXmppDataForm::Field> unknownFields() const;
    void setUnknownFields(const QList<QXmppDataForm::Field> &unknownFields);

protected:
    void parseForm(const QXmppDataForm &) override;
    void serializeForm(QXmppDataForm &) const override;

    virtual bool parseField(const QXmppDataForm::Field &);

private:
    QSharedDataPointer<QXmppExtensibleDataFormBasePrivate> d;
};

#endif  // QXMPPDATAFORMBASED_H
