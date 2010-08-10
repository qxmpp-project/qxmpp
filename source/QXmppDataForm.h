/*
 * Copyright (C) 2008-2010 The QXmpp developers
 *
 * Author:
 *	Jeremy Lainé
 *
 * Source:
 *	http://code.google.com/p/qxmpp
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

#ifndef QXMPPDATAFORM_H
#define QXMPPDATAFORM_H

#include <QPair>
#include <QString>
#include <QVariant>
#include <QXmlStreamWriter>

class QDomElement;

/// \brief The QXmppDataForm class represents a data form as defined by
/// XEP-0004: Data Forms.
///

class QXmppDataForm
{
public:
    class Field
    {
    public:
        enum Type
        {
            BooleanField,
            FixedField,
            HiddenField,
            JidMultiField,
            JidSingleField,
            ListMultiField,
            ListSingleField,
            TextMultiField,
            TextPrivateField,
            TextSingleField,
        };

        Field(QXmppDataForm::Field::Type type = QXmppDataForm::Field::TextSingleField);

        QString description() const;
        void setDescription(const QString &description);

        QString key() const;
        void setKey(const QString &key);

        QString label() const;
        void setLabel(const QString &label);

        QList<QPair<QString, QString> > options() const;
        void setOptions(const QList<QPair<QString, QString> > &options);

        bool isRequired() const;
        void setRequired(bool required);

        QXmppDataForm::Field::Type type() const;
        void setType(QXmppDataForm::Field::Type type);

        QVariant value() const;
        void setValue(const QVariant &value);

    private:
        QString m_description;
        QString m_key;
        QString m_label;
        QList<QPair<QString, QString> > m_options;
        bool m_required;
        QXmppDataForm::Field::Type m_type;
        QVariant m_value;
    };

    enum Type
    {
        None,
        Form,
        Submit,
        Cancel,
        Result,
    };

    QXmppDataForm(QXmppDataForm::Type type = QXmppDataForm::None);

    QString instructions() const;
    void setInstructions(const QString &instructions);

    QList<Field> fields() const;
    QList<Field> &fields();
    void setFields(const QList<QXmppDataForm::Field> &fields);

    QString title() const;
    void setTitle(const QString &title);

    QXmppDataForm::Type type() const;
    void setType(QXmppDataForm::Type type);

    bool isNull() const;

    void parse(const QDomElement &element);
    void toXml(QXmlStreamWriter *writer) const;

private:
    QString m_instructions;
    QList<Field> m_fields;
    QString m_title;
    QXmppDataForm::Type m_type;
};

#endif
