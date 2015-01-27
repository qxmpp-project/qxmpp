/*
 * Copyright (C) 2008-2014 The QXmpp developers
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
#include <QString>
#include <QSharedData>

// forward declarations of QXmlStream* classes will not work on Mac, we need to
// include the whole header.
// See http://lists.trolltech.com/qt-interest/2008-07/thread00798-0.html
// for an explanation.
#include <QXmlStreamWriter>

#include "QXmppElement.h"

class QXmppExtendedAddressPrivate;

/// \brief Represents an extended address as defined by XEP-0033: Extended Stanza Addressing.
///
/// Extended addresses maybe of different types: some are defined by XEP-0033,
/// others are defined in separate XEPs (for instance XEP-0146: Remote Controlling Clients).
/// That is why the "type" property is a string rather than an enumerated type.

class QXMPP_EXPORT QXmppExtendedAddress
{
public:
    QXmppExtendedAddress();
    QXmppExtendedAddress(const QXmppExtendedAddress&);
    ~QXmppExtendedAddress();

    QXmppExtendedAddress& operator=(const QXmppExtendedAddress&);

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

/// \defgroup Stanzas

/// \brief The QXmppStanza class is the base class for all XMPP stanzas.
///
/// \ingroup Stanzas

class QXMPP_EXPORT QXmppStanza
{
public:
    class QXMPP_EXPORT Error
    {
    public:
        enum Type
        {
            Cancel,
            Continue,
            Modify,
            Auth,
            Wait
        };

        enum Condition
        {
            BadRequest,
            Conflict,
            FeatureNotImplemented,
            Forbidden,
            Gone,
            InternalServerError,
            ItemNotFound,
            JidMalformed,
            NotAcceptable,
            NotAllowed,
            NotAuthorized,
            PaymentRequired,
            RecipientUnavailable,
            Redirect,
            RegistrationRequired,
            RemoteServerNotFound,
            RemoteServerTimeout,
            ResourceConstraint,
            ServiceUnavailable,
            SubscriptionRequired,
            UndefinedCondition,
            UnexpectedRequest,
            BadAuth
        };

        Error();
        Error(Type type, Condition cond, const QString& text = QString());
        Error(const QString& type, const QString& cond, const QString& text = QString());

        int code() const;
        void setCode(int code);

        QString text() const;
        void setText(const QString& text);

        Condition condition() const;
        void setCondition(Condition cond);

        void setType(Type type);
        Type type() const;

        /// \cond
        void parse(const QDomElement &element);
        void toXml(QXmlStreamWriter *writer) const;
        /// \endcond

    private:
        QString getConditionStr() const;
        void setConditionFromStr(const QString& cond);

        QString getTypeStr() const;
        void setTypeFromStr(const QString& type);

        int m_code;
        Type m_type;
        Condition m_condition;
        QString m_text;
    };

    QXmppStanza(const QString& from = QString(), const QString& to = QString());
    QXmppStanza(const QXmppStanza &other);
    virtual ~QXmppStanza();

    QXmppStanza& operator=(const QXmppStanza &other);

    QString to() const;
    void setTo(const QString&);

    QString from() const;
    void setFrom(const QString&);

    QString id() const;
    void setId(const QString&);

    QString lang() const;
    void setLang(const QString&);

    QXmppStanza::Error error() const;
    void setError(const QXmppStanza::Error& error);

    QXmppElementList extensions() const;
    void setExtensions(const QXmppElementList &elements);

    QList<QXmppExtendedAddress> extendedAddresses() const;
    void setExtendedAddresses(const QList<QXmppExtendedAddress> &extendedAddresses);

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

#endif // QXMPPSTANZA_H
