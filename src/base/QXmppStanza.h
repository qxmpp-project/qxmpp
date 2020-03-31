/*
 * Copyright (C) 2008-2020 The QXmpp developers
 *
 * Authors:
 *  Manjeet Dahiya
 *  Jeremy Lainé
 *  Georg Rudoy
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

#ifndef QXMPPSTANZA_H
#define QXMPPSTANZA_H

#include <QByteArray>
#include <QSharedData>
#include <QString>

// forward declarations of QXmlStream* classes will not work on Mac, we need to
// include the whole header.
// See http://lists.trolltech.com/qt-interest/2008-07/thread00798-0.html
// for an explanation.
#include "QXmppElement.h"

#include <QXmlStreamWriter>

class QXmppExtendedAddressPrivate;

///
/// \brief Represents an extended address as defined by \xep{0033}: Extended
/// Stanza Addressing.
///
/// Extended addresses maybe of different types: some are defined by \xep{0033},
/// others are defined in separate XEPs (for instance \xep{0146}: Remote
/// Controlling Clients). That is why the "type" property is a string rather
/// than an enumerated type.
///
class QXMPP_EXPORT QXmppExtendedAddress
{
public:
    QXmppExtendedAddress();
    QXmppExtendedAddress(const QXmppExtendedAddress &);
    ~QXmppExtendedAddress();

    QXmppExtendedAddress &operator=(const QXmppExtendedAddress &);

    QString description() const;
    void setDescription(const QString &description);

    QString jid() const;
    void setJid(const QString &jid);

    QString type() const;
    void setType(const QString &type);

    bool isDelivered() const;
    void setDelivered(bool);

    bool isValid() const;

    /// \cond
    void parse(const QDomElement &element);
    void toXml(QXmlStreamWriter *writer) const;
    /// \endcond

private:
    QSharedDataPointer<QXmppExtendedAddressPrivate> d;
};

class QXmppStanzaPrivate;
class QXmppStanzaErrorPrivate;

///
/// \defgroup Stanzas Stanzas
///
/// All packets that are sent and received are serialized in Stanzas, so this
/// includes IQ stanzas, message stanzas, presence stanzas and other stanzas.
///

///
/// \brief The QXmppStanza class is the base class for all XMPP stanzas.
///
/// \ingroup Stanzas
///
class QXMPP_EXPORT QXmppStanza
{
public:
    ///
    /// \brief The Error class represents a stanza error.
    ///
    class QXMPP_EXPORT Error
    {
    public:
        /// The type represents the error type of stanza errors.
        ///
        /// The error descriptions are not detailed in here. The exact meaning
        /// can be found in the particular protocols using them.
        enum Type {
            Cancel,    ///< The error is not temporary.
            Continue,  ///< The error was only a warning.
            Modify,    ///< The request needs to be changed and resent.
            Auth,      ///< The request needs to be resent after authentication.
            Wait       ///< The error is temporary, you should wait and resend.
        };

        /// A detailed condition of the error
        enum Condition {
            BadRequest,             ///< The request does not contain a valid schema.
            Conflict,               ///< The request conflicts with another.
            FeatureNotImplemented,  ///< The feature is not implemented.
            Forbidden,              ///< The requesting entity does not posses the necessary privileges to perform the request.
            Gone,                   ///< The user or server can not be contacted at the address. This is used in combination with a redirection URI.
            InternalServerError,    ///< The server has expierienced an internal error and can not process the request.
            ItemNotFound,           ///< The requested item could not be found.
            JidMalformed,           ///< The given JID is not valid.
            NotAcceptable,          ///< The request does not meet the defined critera.
            NotAllowed,             ///< No entity is allowed to perform the request.
            NotAuthorized,          ///< The request should be resent after authentication.
#if QXMPP_DEPRECATED_SINCE(1, 3)
            /// Payment is required to perform the request.
            /// \deprecated This error condition is deprecated since QXmpp 1.3 as it was not adopted in RFC6120.
            PaymentRequired Q_DECL_ENUMERATOR_DEPRECATED_X("The <payment-required/> error was removed in RFC6120"),
#endif
            RecipientUnavailable = 12,  ///< The recipient is unavailable.
            Redirect,                   ///< The requested resource is available elsewhere. This is used in combination with a redirection URI.
            RegistrationRequired,       ///< The requesting entity needs to register first.
            RemoteServerNotFound,       ///< The remote server could not be found.
            RemoteServerTimeout,        ///< The connection to the server could not be established or timed out.
            ResourceConstraint,         ///< The recipient lacks system resources to perform the request.
            ServiceUnavailable,         ///< The service is currently not available.
            SubscriptionRequired,       ///< The requester needs to subscribe first.
            UndefinedCondition,         ///< An undefined condition was hit.
            UnexpectedRequest,          ///< The request was unexpected.
            PolicyViolation             ///< The entity has violated a local server policy. \since QXmpp 1.3
        };

        Error();
        Error(const Error &);
        Error(Type type, Condition cond, const QString &text = QString());
        Error(const QString &type, const QString &cond, const QString &text = QString());
        ~Error();

        Error &operator=(const Error &);

        int code() const;
        void setCode(int code);

        QString text() const;
        void setText(const QString &text);

        Condition condition() const;
        void setCondition(Condition cond);

        void setType(Type type);
        Type type() const;

        QString by() const;
        void setBy(const QString &by);

        QString redirectionUri() const;
        void setRedirectionUri(const QString &redirectionUri);

        // XEP-0363: HTTP File Upload
        bool fileTooLarge() const;
        void setFileTooLarge(bool);

        qint64 maxFileSize() const;
        void setMaxFileSize(qint64);

        QDateTime retryDate() const;
        void setRetryDate(const QDateTime &);

        /// \cond
        void parse(const QDomElement &element);
        void toXml(QXmlStreamWriter *writer) const;
        /// \endcond

    private:
        QString getConditionStr() const;
        void setConditionFromStr(const QString &cond);

        QString getTypeStr() const;
        void setTypeFromStr(const QString &type);

        QSharedDataPointer<QXmppStanzaErrorPrivate> d;
    };

    QXmppStanza(const QString &from = QString(), const QString &to = QString());
    QXmppStanza(const QXmppStanza &other);
    virtual ~QXmppStanza();

    QXmppStanza &operator=(const QXmppStanza &other);

    QString to() const;
    void setTo(const QString &);

    QString from() const;
    void setFrom(const QString &);

    QString id() const;
    void setId(const QString &);

    QString lang() const;
    void setLang(const QString &);

    QXmppStanza::Error error() const;
    void setError(const QXmppStanza::Error &error);

    QXmppElementList extensions() const;
    void setExtensions(const QXmppElementList &elements);

    QList<QXmppExtendedAddress> extendedAddresses() const;
    void setExtendedAddresses(const QList<QXmppExtendedAddress> &extendedAddresses);

    virtual bool isXmppStanza() const;

    /// \cond
    virtual void parse(const QDomElement &element);
    virtual void toXml(QXmlStreamWriter *writer) const = 0;

protected:
    void extensionsToXml(QXmlStreamWriter *writer) const;
    void generateAndSetNextId();
    /// \endcond

private:
    QSharedDataPointer<QXmppStanzaPrivate> d;
    static uint s_uniqeIdNo;
};

Q_DECLARE_METATYPE(QXmppStanza::Error::Type);
Q_DECLARE_METATYPE(QXmppStanza::Error::Condition);

#endif  // QXMPPSTANZA_H
