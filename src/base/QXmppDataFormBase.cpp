/*
 * Copyright (C) 2008-2020 The QXmpp developers
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

#include "QXmppDataFormBase.h"

#include "QXmppDataForm.h"

#include <QDateTime>

///
/// \class QXmppDataFormBased
///
/// QXmppDataFormBased is an abstract class types that can be serialized to data
/// forms.
///
/// QXmppDataFormBased based types can easily be converted to QXmppDataForms, it
/// is as simple as this:
/// \code
/// MyDataFormBased foo;
/// QXmppDataForm dataForm(foo);
/// \endcode
///
/// To make this work, you will need to at least implement the toDataForm()
/// method. For parsing your type you should also create a static creator
/// method, like this:
/// \code
/// static std::optional<MyType> fromDataForm(const QXmppDataForm &);
/// \endcode
///
/// \since QXmpp 1.5
///

QXmppDataForm QXmppDataFormBase::toDataForm() const
{
    QXmppDataForm form(QXmppDataForm::Form);

    // add FORM_TYPE
    if (!formType().isEmpty()) {
        form.fields() << QXmppDataForm::Field(QXmppDataForm::Field::HiddenField,
                                              QStringLiteral("FORM_TYPE"),
                                              formType());
    }

    // manual serialization parts
    serializeForm(form);

    return form;
}

bool QXmppDataFormBase::fromDataForm(const QXmppDataForm &form, QXmppDataFormBase &output)
{
    output.parseForm(form);
    return true;
}

void QXmppDataFormBase::serializeDatetime(QXmppDataForm &form, const QString &name, const QDateTime &datetime, QXmppDataForm::Field::Type type)
{
    if (!datetime.isNull() && datetime.isValid()) {
        serializeValue(form, type, name, datetime.toUTC().toString(Qt::ISODate));
    }
}

class QXmppExtensibleDataFormBasePrivate : public QSharedData
{
public:
    QList<QXmppDataForm::Field> unknownFields;
};

QXmppExtensibleDataFormBase::QXmppExtensibleDataFormBase()
    : d(new QXmppExtensibleDataFormBasePrivate)
{
}

/// \cond
QXmppExtensibleDataFormBase::QXmppExtensibleDataFormBase(const QXmppExtensibleDataFormBase &) = default;
QXmppExtensibleDataFormBase::~QXmppExtensibleDataFormBase() = default;
QXmppExtensibleDataFormBase &QXmppExtensibleDataFormBase::operator=(const QXmppExtensibleDataFormBase &) = default;
/// \endcond

QList<QXmppDataForm::Field> QXmppExtensibleDataFormBase::unknownFields() const
{
    return d->unknownFields;
}

void QXmppExtensibleDataFormBase::setUnknownFields(const QList<QXmppDataForm::Field> &unknownFields)
{
    d->unknownFields = unknownFields;
}

void QXmppExtensibleDataFormBase::parseForm(const QXmppDataForm &form)
{
    const auto fields = form.fields();
    for (const auto &field : fields) {
        // FORM_TYPE fields are not saved (override this function to save them)
        if (!parseField(field) &&
                !(field.type() == QXmppDataForm::Field::HiddenField &&
                  field.key() == QStringLiteral("FORM_TYPE"))) {
            d->unknownFields << field;
        }
    }
}

void QXmppExtensibleDataFormBase::serializeForm(QXmppDataForm &form) const
{
    form.fields() << d->unknownFields;
}

bool QXmppExtensibleDataFormBase::parseField(const QXmppDataForm::Field &)
{
    return false;
}
