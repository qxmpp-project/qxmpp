// SPDX-FileCopyrightText: 2009 Ian Reinhart Geiser <geiseri@kde.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPINVOKABLE_H
#define QXMPPINVOKABLE_H

#include "QXmppGlobal.h"

#include <QHash>
#include <QObject>
#include <QStringList>
#include <QVariant>
#include <QWriteLocker>

/**
This is the base class for all objects that will be invokable via RPC.  All public slots of objects derived from this class will be exposed to the RPC interface.  As a note for all methods, they can only understand types that QVariant knows about.

        @author Ian Reinhart Geiser <geiseri@kde.org>
*/
class QXMPP_EXPORT QXmppInvokable : public QObject
{
    Q_OBJECT
public:
    QXmppInvokable(QObject *parent = nullptr);

    ~QXmppInvokable() override;

    /**
     * Execute a method on an object. with a set of arguments. This method is reentrant, and the method
     * that is invoked will be done in a thread safe manner.  It should be noted that while this method
     * is threadsafe and reentrant the side affects of the methods invoked may not be.
     */
    QVariant dispatch(const QByteArray &method, const QList<QVariant> &args = QList<QVariant>());

    /**
     * Utility method to convert a QList<QVariant> to a list of types for type
     * checking.
     */
    static QList<QByteArray> paramTypes(const QList<QVariant> &params);

    /**
     * Reimplement this method to return a true if the invoking JID is allowed to execute the method.
     */
    virtual bool isAuthorized(const QString &jid) const = 0;

public Q_SLOTS:
    /**
     * This provides a list of interfaces for introspection of the presented interface.
     */
    QStringList interfaces() const;

private:
    void buildMethodHash();
    QHash<QByteArray, int> m_methodHash;
    QReadWriteLock m_lock;
};

#endif
