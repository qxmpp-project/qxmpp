/*
 * Copyright (C) 2008-2011 The QXmpp developers
 *
 * Author:
 *  Manjeet Dahiya
 *
 * Source:
 *  http://code.google.com/p/qxmpp
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

#include "QXmppElement.h"
#include "QXmppPacket.h"
#include <QString>

// forward declarations of QXmlStream* classes will not work on Mac, we need to
// include the whole header.
// See http://lists.trolltech.com/qt-interest/2008-07/thread00798-0.html
// for an explanation.
#include <QXmlStreamWriter>

/// \defgroup Stanzas

/// \brief The QXmppStanza class is the base class for all XMPP stanzas.
///
/// \ingroup Stanzas

class QXmppStanza : public QXmppPacket
{
public:
    class Error
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
            UnexpectedRequest
        };

        Error();
        Error(Type type, Condition cond, const QString& text="");
        Error(const QString& type, const QString& cond, const QString& text="");

        int code() const;
        void setCode(int code);

        QString text() const;
        void setText(const QString& text);

        Condition condition() const;
        void setCondition(Condition cond);

        void setType(Type type);
        Type type() const;

        // FIXME : remove this once is gone
        bool isValid();

        /// \cond
        void parse(const QDomElement &element);
        void toXml(QXmlStreamWriter *writer) const;

        // deprecated in release 0.2.0
        // deprecated accessors, use the form without "get" instead
        int Q_DECL_DEPRECATED getCode() const;
        QString Q_DECL_DEPRECATED getText() const;
        Condition Q_DECL_DEPRECATED getCondition() const;
        Type Q_DECL_DEPRECATED getType() const;
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
    ~QXmppStanza();
    
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

    /// \cond
    // FIXME : why is this needed?
    bool isErrorStanza();

    // deprecated in release 0.2.0
    // deprecated accessors, use the form without "get" instead
    QString Q_DECL_DEPRECATED getTo() const;  
    QString Q_DECL_DEPRECATED getFrom() const;
    QString Q_DECL_DEPRECATED getId() const;    
    QString Q_DECL_DEPRECATED getLang() const;    
    QXmppStanza::Error Q_DECL_DEPRECATED getError() const;

protected:
    void generateAndSetNextId();
    void parse(const QDomElement &element);
    /// \endcond

private:
    static uint s_uniqeIdNo;
    QString m_to;  
    QString m_from;
    QString m_id;    
    QString m_lang;
    QXmppStanza::Error m_error;
    QXmppElementList m_extensions;
};

#endif // QXMPPSTANZA_H
