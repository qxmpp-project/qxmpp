/*
 * Copyright (C) 2008-2014 The QXmpp developers
 *
 * Authors:
 *  Ian Reinhart Geiser
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

#ifndef QXMPPINVOKABLE_H
#define QXMPPINVOKABLE_H

#include <QObject>
#include <QHash>
#include <QVariant>
#include <QWriteLocker>
#include <QStringList>

#include "QXmppGlobal.h"

/**
This is the base class for all objects that will be invokable via RPC.  All public slots of objects derived from this class will be exposed to the RPC interface.  As a note for all methods, they can only understand types that QVariant knows about.

        @author Ian Reinhart Geiser <geiseri@kde.org>
*/
class QXMPP_EXPORT QXmppInvokable : public QObject
{
        Q_OBJECT
public:
        QXmppInvokable( QObject *parent = 0 );

        ~QXmppInvokable();

        /**
         * Execute a method on an object. with a set of arguments. This method is reentrant, and the method
         * that is invoked will be done in a thread safe manner.  It should be noted that while this method
         * is threadsafe and reentrant the side affects of the methods invoked may not be.
         */
        QVariant dispatch( const QByteArray &method, const QList<QVariant> &args = QList<QVariant>() );

        /**
         * Utility method to convert a QList<QVariant> to a list of types for type
         * checking.
         */
        static QList<QByteArray> paramTypes( const QList<QVariant> &params );

        /**
          * Reimplement this method to return a true if the invoking JID is allowed to execute the method.
          */
        virtual bool isAuthorized( const QString &jid ) const = 0;

public slots:
        /**
          * This provides a list of interfaces for introspection of the presented interface.
          */
        QStringList interfaces() const;

private:
        void buildMethodHash();
        QHash<QByteArray,int> m_methodHash;
        QReadWriteLock m_lock;
};


#endif
