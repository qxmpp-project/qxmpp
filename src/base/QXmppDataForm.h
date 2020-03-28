/*
 * Copyright (C) 2008-2020 The QXmpp developers
 *
 * Author:
 *  Jeremy Lainé
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

#ifndef QXMPPDATAFORM_H
#define QXMPPDATAFORM_H

#include "QXmppStanza.h"

#if QXMPP_DEPRECATED_SINCE(1, 1)
#include <QPair>
#endif
#include <QVariant>
#include <QVector>

class QMimeType;
class QUrl;

class QXmppDataFormPrivate;
class QXmppDataFormFieldPrivate;
class QXmppDataFormMediaPrivate;
class QXmppDataFormMediaSourcePrivate;

///
/// \brief The QXmppDataForm class represents a data form as defined by
/// \xep{0004}: Data Forms.
///
class QXMPP_EXPORT QXmppDataForm
{
public:
    ///
    /// \brief The \c QXmppDataForm::MediaSource class represents a link to one
    /// of possibly multiple sources for a media element from \xep{0221}: Data
    /// Forms Media Element consisting of a MIME type and a QUrl.
    ///
    /// \since QXmpp 1.1
    ///
    class QXMPP_EXPORT MediaSource
    {
    public:
        MediaSource();
        MediaSource(const QUrl &uri, const QMimeType &contentType);
        MediaSource(const QXmppDataForm::MediaSource &);
        ~MediaSource();

        MediaSource &operator=(const MediaSource &);

        QUrl uri() const;
        void setUri(const QUrl &uri);

        QMimeType contentType() const;
        void setContentType(const QMimeType &contentType);

        bool operator==(const MediaSource &other) const;

    private:
        QSharedDataPointer<QXmppDataFormMediaSourcePrivate> d;
    };

#if QXMPP_DEPRECATED_SINCE(1, 1)
    ///
    /// \brief The QXmppDataForm::Media class represents a media field as
    /// defined by \xep{0221}: Data Forms Media Element.
    ///
    /// \deprecated This class is deprecated since QXmpp 1.1.
    ///
    class QXMPP_EXPORT Media
    {
    public:
        QT_DEPRECATED_X("Use QXmppDataForm::Field() instead")
        Media();
        QT_DEPRECATED_X("Use QXmppDataForm::Field() instead")
        Media(const QXmppDataForm::Media &other);
        ~Media();

        QXmppDataForm::Media &operator=(const QXmppDataForm::Media &other);

        QT_DEPRECATED_X("Use QXmppDataForm::Field::mediaSize().height() instead")
        int height() const;
        QT_DEPRECATED_X("Use QXmppDataForm::Field::mediaSize().setHeight() instead")
        void setHeight(int height);

        QT_DEPRECATED_X("Use QXmppDataForm::Field::mediaSize().width() instead")
        int width() const;
        QT_DEPRECATED_X("Use QXmppDataForm::Field::mediaSize().setWidth() instead")
        void setWidth(int width);

        QT_DEPRECATED_X("Use QXmppDataForm::Field::mediaSources() instead")
        QList<QPair<QString, QString>> uris() const;
        QT_DEPRECATED_X("Use QXmppDataForm::Field::setMediaSources() instead")
        void setUris(const QList<QPair<QString, QString>> &uris);

        QT_DEPRECATED_X("Use QXmppDataForm::Field::mediaSources().isEmpty() instead")
        bool isNull() const;

    private:
        QSharedDataPointer<QXmppDataFormMediaPrivate> d;
    };
#endif

    ///
    /// \brief The QXmppDataForm::Field class represents a data form field
    /// as defined by \xep{0004}: Data Forms.
    ///
    class QXMPP_EXPORT Field
    {
    public:
        /// This enum is used to describe a field's type.
        enum Type {
            BooleanField,
            FixedField,
            HiddenField,
            JidMultiField,
            JidSingleField,
            ListMultiField,
            ListSingleField,
            TextMultiField,
            TextPrivateField,
            TextSingleField
        };

        // ### QXmpp2: merge ctors
        Field(QXmppDataForm::Field::Type type = QXmppDataForm::Field::TextSingleField);
        Field(QXmppDataForm::Field::Type type,
              const QString &key,
              const QVariant &value = {},
              bool isRequired = false,
              const QString &label = {},
              const QString &description = {},
              const QList<QPair<QString, QString>> &options = {});
        Field(const QXmppDataForm::Field &other);
        ~Field();

        QXmppDataForm::Field &operator=(const QXmppDataForm::Field &other);

        QString description() const;
        void setDescription(const QString &description);

        QString key() const;
        void setKey(const QString &key);

        QString label() const;
        void setLabel(const QString &label);

#if QXMPP_DEPRECATED_SINCE(1, 1)
        QT_DEPRECATED_X("Use QXmppDataForm::Field::mediaSources() or QXmppDataForm::Field::mediaSize() instead")
        Media media() const;

        QT_DEPRECATED_X("Use QXmppDataForm::Field::setMediaSources() or QXmppDataForm::Field::setMediaSize() instead")
        void setMedia(const Media &media);
#endif

        QList<QPair<QString, QString>> options() const;
        void setOptions(const QList<QPair<QString, QString>> &options);

        bool isRequired() const;
        void setRequired(bool required);

        QXmppDataForm::Field::Type type() const;
        void setType(QXmppDataForm::Field::Type type);

        QVariant value() const;
        void setValue(const QVariant &value);

        QVector<QXmppDataForm::MediaSource> &mediaSources();
        QVector<QXmppDataForm::MediaSource> mediaSources() const;
        void setMediaSources(const QVector<QXmppDataForm::MediaSource> &mediaSources);

        QSize mediaSize() const;
        QSize &mediaSize();
        void setMediaSize(const QSize &size);

        bool operator==(const Field &other) const;

    private:
        QSharedDataPointer<QXmppDataFormFieldPrivate> d;
    };

    /// This enum is used to describe a form's type.
    enum Type {
        None,    ///< Unknown form type
        Form,    ///< The form-processing entity is asking the form-submitting
                 ///< entity to complete a form.
        Submit,  ///< The form-submitting entity is submitting data to the
                 ///< form-processing entity.
        Cancel,  ///< The form-submitting entity has cancelled submission
                 ///< of data to the form-processing entity.
        Result   ///< The form-processing entity is returning data
                 ///< (e.g., search results) to the form-submitting entity,
                 ///< or the data is a generic data set.
    };

    // ### QXmpp2: merge ctors
    QXmppDataForm(QXmppDataForm::Type type = QXmppDataForm::None);
    QXmppDataForm(QXmppDataForm::Type type,
                  const QList<Field> &fields,
                  const QString &title = {},
                  const QString &instructions = {});
    QXmppDataForm(const QXmppDataForm &other);
    ~QXmppDataForm();

    QXmppDataForm &operator=(const QXmppDataForm &other);

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

    /// \cond
    void parse(const QDomElement &element);
    void toXml(QXmlStreamWriter *writer) const;
    /// \endcond

private:
    QSharedDataPointer<QXmppDataFormPrivate> d;
};

#endif
