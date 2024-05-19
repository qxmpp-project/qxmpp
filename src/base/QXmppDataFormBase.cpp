// SPDX-FileCopyrightText: 2021 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppDataFormBase.h"

#include "QXmppDataForm.h"

#include "StringLiterals.h"

#include <QDateTime>

///
/// \class QXmppDataFormBase
///
/// QXmppDataFormBase is an abstract class types that can be serialized to data
/// forms.
///
/// QXmppDataFormBase based types can easily be converted to QXmppDataForms, it
/// is as simple as this:
/// \code
/// MyDataFormBase foo;
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

///
/// Serializes all fields to a QXmppDataForm.
///
QXmppDataForm QXmppDataFormBase::toDataForm() const
{
    QXmppDataForm form(QXmppDataForm::Form);

    // add FORM_TYPE
    serializeNullable(form, QXmppDataForm::Field::HiddenField, u"FORM_TYPE"_s, formType());

    // manual serialization parts
    serializeForm(form);

    return form;
}

///
/// Parses the QXmppDataForm.
///
bool QXmppDataFormBase::fromDataForm(const QXmppDataForm &form, QXmppDataFormBase &output)
{
    output.parseForm(form);
    return true;
}

///
/// \fn QXmppDataFormBase::formType
///
/// Returns the 'FORM_TYPE' value of the parsed form.
///

///
/// \fn QXmppDataFormBase::parseForm
///
/// This is called when a QXmppDataForm is parsed. You can parse all values from
/// the given form and its fields.
///

///
/// \fn QXmppDataFormBase::serializeForm
///
/// This is called the object is serialized to a QXmppDataForm. You need to
/// create a new QXmppDataForm and serialize all fields and values.
///

///
/// \fn QXmppDataFormBase::parseUInt
///
/// Parses an unsigned int from a QVariant (QString). Returns std::nullopt if
/// the no number could be parsed.
///

///
/// \fn QXmppDataFormBase::parseULongLong
///
/// Parses an unsigned long long from a QVariant (QString). Returns std::nullopt
/// if the no number could be parsed.
///

///
/// \fn QXmppDataFormBase::parseBool
///
/// Returns the contained boolean value if the QVariant contains a bool.
///

///
/// \fn QXmppDataFormBase::serializeValue
///
/// Adds a new field to the form with the given field type, field name and value.
///

///
/// \fn QXmppDataFormBase::serializeNullable
///
/// Adds a new field to the form if \code !value.isNull() \endcode.
///

///
/// \fn QXmppDataFormBase::serializeEmptyable
///
/// Adds a new field to the form if \code !value.isEmpty() \endcode.
///

///
/// \fn QXmppDataFormBase::serializeOptional
///
/// Adds a new field to the form if \code optional.has_value() \endcode.
///

///
/// \fn QXmppDataFormBase::serializeOptionalNumber
///
/// Adds a new field to the form if \code optional.has_value() \endcode.
/// Converts the optional's value to QString using QString::number().
///

///
/// Adds a new field to the form if the passed QDateTime is valid and formats it
/// as ISO timestamp and always uses UTC.
///
void QXmppDataFormBase::serializeDatetime(QXmppDataForm &form, const QString &name, const QDateTime &datetime, QXmppDataForm::Field::Type type)
{
    if (datetime.isValid()) {
        serializeValue(form, type, name, datetime.toUTC().toString(Qt::ISODate));
    }
}

///
/// \class QXmppExtensibleDataFormBase
///
/// This class is used for parsing a QXmppDataForm in an extensible way with
/// inheritance and keeping additional unknown fields.
///
/// When inheriting you need to reimplement parseField(), serializeForm() and
/// formType(). Also you should add a static parsing function (e.g.
/// QXmppPubSubMetadata::fromDataForm()).
///
/// \since QXmpp 1.5
///

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
QXmppExtensibleDataFormBase::QXmppExtensibleDataFormBase(QXmppExtensibleDataFormBase &&) = default;
QXmppExtensibleDataFormBase::~QXmppExtensibleDataFormBase() = default;
QXmppExtensibleDataFormBase &QXmppExtensibleDataFormBase::operator=(const QXmppExtensibleDataFormBase &) = default;
QXmppExtensibleDataFormBase &QXmppExtensibleDataFormBase::operator=(QXmppExtensibleDataFormBase &&) = default;
/// \endcond

///
/// Returns all fields that couldn't be parsed.
///
QList<QXmppDataForm::Field> QXmppExtensibleDataFormBase::unknownFields() const
{
    return d->unknownFields;
}

///
/// Sets all additional fields to be serialized.
///
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
              field.key() == u"FORM_TYPE")) {
            d->unknownFields << field;
        }
    }
}

void QXmppExtensibleDataFormBase::serializeForm(QXmppDataForm &form) const
{
    form.fields() << d->unknownFields;
}

///
/// Returns true if a field has been parsed.
///
/// Should be reimplemented to do actual parsing. All fields that can't be
/// parsed end up as unknownFields().
///
bool QXmppExtensibleDataFormBase::parseField(const QXmppDataForm::Field &)
{
    return false;
}
