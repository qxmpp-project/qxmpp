// SPDX-FileCopyrightText: 2010 Jeremy Lainé <jeremy.laine@m4x.org>
// SPDX-FileCopyrightText: 2019 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

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

class QXmppDataFormBase;
class QXmppDataFormPrivate;
class QXmppDataFormFieldPrivate;
class QXmppDataFormMediaPrivate;
class QXmppDataFormMediaSourcePrivate;

class QXMPP_EXPORT QXmppDataForm
{
public:
    class QXMPP_EXPORT MediaSource
    {
    public:
        MediaSource();
        MediaSource(const QUrl &uri, const QMimeType &contentType);
        MediaSource(const QXmppDataForm::MediaSource &);
        MediaSource(QXmppDataForm::MediaSource &&);
        ~MediaSource();

        MediaSource &operator=(const MediaSource &);
        MediaSource &operator=(MediaSource &&);

        QUrl uri() const;
        void setUri(const QUrl &uri);

        QMimeType contentType() const;
        void setContentType(const QMimeType &contentType);

        bool operator==(const MediaSource &other) const;

    private:
        QSharedDataPointer<QXmppDataFormMediaSourcePrivate> d;
    };

#if QXMPP_DEPRECATED_SINCE(1, 1)
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

        Field(Type type = TextSingleField,
              const QString &key = {},
              const QVariant &value = {},
              bool isRequired = false,
              const QString &label = {},
              const QString &description = {},
              const QList<QPair<QString, QString>> &options = {});
        Field(const QXmppDataForm::Field &other);
        Field(QXmppDataForm::Field &&);
        ~Field();

        QXmppDataForm::Field &operator=(const QXmppDataForm::Field &other);
        QXmppDataForm::Field &operator=(QXmppDataForm::Field &&);

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

    QXmppDataForm(Type type = None,
                  const QList<Field> &fields = {},
                  const QString &title = {},
                  const QString &instructions = {});
    QXmppDataForm(const QXmppDataFormBase &based);
    QXmppDataForm(const QXmppDataForm &other);
    QXmppDataForm(QXmppDataForm &&);
    ~QXmppDataForm();

    QXmppDataForm &operator=(const QXmppDataForm &other);
    QXmppDataForm &operator=(QXmppDataForm &&);

    QString instructions() const;
    void setInstructions(const QString &instructions);

    QList<Field> fields() const;
    QList<Field> &fields();
    void setFields(const QList<QXmppDataForm::Field> &fields);

    QString title() const;
    void setTitle(const QString &title);

    QXmppDataForm::Type type() const;
    void setType(QXmppDataForm::Type type);

    QString formType() const;

    bool isNull() const;

    /// \cond
    void parse(const QDomElement &element);
    void toXml(QXmlStreamWriter *writer) const;
    /// \endcond

private:
    QSharedDataPointer<QXmppDataFormPrivate> d;
};

Q_DECLARE_METATYPE(QXmppDataForm)

#endif
