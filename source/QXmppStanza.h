/*
 * Copyright (C) 2008-2009 Manjeet Dahiya
 *
 * Author:
 *	Manjeet Dahiya
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


#ifndef QXMPPSTANZA_H
#define QXMPPSTANZA_H

#include "QXmppPacket.h"
#include <QString>

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

        void setText(const QString& text);
        void setCondition(Condition cond);
        void setConditionFromStr(const QString& cond);
        void setType(Type type);
        void setTypeFromStr(const QString& type);
        QString getText() const;
        Condition getCondition() const;
        Type getType() const;
        QString toXml() const;
        QString getConditionStr() const;
        QString getTypeStr() const;

    private:
        Type m_type;
        Condition m_condition;
        QString m_text;
    };

    QXmppStanza(const QString& from = "", const QString& to = "");
    ~QXmppStanza();
    
    QString getTo() const;  
    QString getFrom() const;
    QString getId() const;    
    QString getLang() const;    
    QXmppStanza::Error getError() const;    

    void setTo(const QString&);  
    void setFrom(const QString&);
    void setId(const QString&);
    void generateAndSetNextId();
    void setLang(const QString&);
    void setError(QXmppStanza::Error& error);    
    bool isErrorStanza();

private:
    static int s_uniqeIdNo;
    QString m_to;  
    QString m_from;
    QString m_id;    
    QString m_lang;
    QXmppStanza::Error m_error;
};

#endif // QXMPPSTANZA_H
