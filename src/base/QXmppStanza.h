// SPDX-FileCopyrightText: 2009 Manjeet Dahiya <manjeetdahiya@gmail.com>
// SPDX-FileCopyrightText: 2010 Jeremy Lainé <jeremy.laine@m4x.org>
// SPDX-FileCopyrightText: 2015 Georg Rudoy <0xd34df00d@gmail.com>
// SPDX-FileCopyrightText: 2019 Linus Jahn <lnj@kaidan.im>
// SPDX-FileCopyrightText: 2022 Melvin Keskin <melvo@olomono.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPSTANZA_H
#define QXMPPSTANZA_H

#include <optional>

#include <QByteArray>
#include <QSharedData>

// forward declarations of QXmlStream* classes will not work on Mac, we need to
// include the whole header.
// See http://lists.trolltech.com/qt-interest/2008-07/thread00798-0.html
// for an explanation.
#include "QXmppElement.h"
#include "QXmppNonza.h"

#include <QXmlStreamWriter>

class QXmppE2eeMetadata;
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
    QXmppExtendedAddress(QXmppExtendedAddress &&);
    ~QXmppExtendedAddress();

    QXmppExtendedAddress &operator=(const QXmppExtendedAddress &);
    QXmppExtendedAddress &operator=(QXmppExtendedAddress &&);

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
class QXMPP_EXPORT QXmppStanza : public QXmppNonza
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
            NoType = -1,
            Cancel,    ///< The error is not temporary.
            Continue,  ///< The error was only a warning.
            Modify,    ///< The request needs to be changed and resent.
            Auth,      ///< The request needs to be resent after authentication.
            Wait       ///< The error is temporary, you should wait and resend.
        };

        /// A detailed condition of the error
        enum Condition {
            NoCondition = -1,
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
        Error(Error &&);
        Error(Type type, Condition cond, const QString &text = QString());
        Error(const QString &type, const QString &cond, const QString &text = QString());
        /// \cond
        Error(QSharedDataPointer<QXmppStanzaErrorPrivate> d);
        /// \endcond
        ~Error();

        Error &operator=(const Error &);
        Error &operator=(Error &&);

        int code() const;
        void setCode(int code);

        QString text() const;
        void setText(const QString &text);

        Condition condition() const;
        void setCondition(Condition cond);

        Type type() const;
        void setType(Type type);

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
        friend class QXmppStanza;

        QSharedDataPointer<QXmppStanzaErrorPrivate> d;
    };

    QXmppStanza(const QString &from = QString(), const QString &to = QString());
    QXmppStanza(const QXmppStanza &other);
    QXmppStanza(QXmppStanza &&);
    ~QXmppStanza() override;

    QXmppStanza &operator=(const QXmppStanza &other);
    QXmppStanza &operator=(QXmppStanza &&);

    QString to() const;
    void setTo(const QString &);

    QString from() const;
    void setFrom(const QString &);

    QString id() const;
    void setId(const QString &);

    QString lang() const;
    void setLang(const QString &);

    QXmppStanza::Error error() const;
    std::optional<Error> errorOptional() const;
    void setError(const QXmppStanza::Error &error);
    void setError(const std::optional<Error> &error);

    QXmppElementList extensions() const;
    void setExtensions(const QXmppElementList &elements);

    QList<QXmppExtendedAddress> extendedAddresses() const;
    void setExtendedAddresses(const QList<QXmppExtendedAddress> &extendedAddresses);

    std::optional<QXmppE2eeMetadata> e2eeMetadata() const;
    void setE2eeMetadata(const std::optional<QXmppE2eeMetadata> &e2eeMetadata);

    /// \cond
    void parse(const QDomElement &element) override;

protected:
    void extensionsToXml(QXmlStreamWriter *writer, QXmpp::SceMode = QXmpp::SceAll) const;
    void generateAndSetNextId();
    /// \endcond

private:
    QSharedDataPointer<QXmppStanzaPrivate> d;
    static uint s_uniqeIdNo;
    friend class TestClient;
};

Q_DECLARE_METATYPE(QXmppStanza::Error::Type);
Q_DECLARE_METATYPE(QXmppStanza::Error::Condition);

#endif  // QXMPPSTANZA_H
