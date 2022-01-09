/*
 * Copyright (C) 2008-2022 The QXmpp developers
 *
 * Author:
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

#ifndef QXMPPDATAFORMBASED_H
#define QXMPPDATAFORMBASED_H

#include "QXmppDataForm.h"

#include <optional>

class QXmppDataForm;

class QXMPP_EXPORT QXmppDataFormBase
{
public:
    virtual ~QXmppDataFormBase() = default;

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
    static void serializeNullable(QXmppDataForm &form, QXmppDataForm::Field::Type type, const QString &name, const T &value)
    {
        if (!value.isNull()) {
            serializeValue(form, type, name, value);
        }
    }

    template<typename T>
    static void serializeEmptyable(QXmppDataForm &form, QXmppDataForm::Field::Type type, const QString &name, const T &value)
    {
        if (!value.isEmpty()) {
            serializeValue(form, type, name, value);
        }
    }

    template<typename T, typename ValueConverter = T (*)(T)>
    static void serializeOptional(QXmppDataForm &form, QXmppDataForm::Field::Type type, const QString &name, const std::optional<T> &optional, ValueConverter convert = [](T a) { return a; })
    {
        if (optional.has_value()) {
            serializeValue(form, type, name, convert(*optional));
        }
    }

    template<typename T>
    static void serializeOptionalNumber(QXmppDataForm &form, QXmppDataForm::Field::Type type, const QString &name, std::optional<T> optional)
    {
        if (optional.has_value()) {
            serializeValue(form, type, name, QString::number(*optional));
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
    virtual ~QXmppExtensibleDataFormBase();

    QXmppExtensibleDataFormBase &operator=(const QXmppExtensibleDataFormBase &);
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

#endif // QXMPPDATAFORMBASED_H
