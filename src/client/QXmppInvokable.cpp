// SPDX-FileCopyrightText: 2009 Ian Reinhart Geiser <geiseri@kde.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppInvokable.h"

#include <QMetaMethod>
#include <QStringList>
#include <QVariant>
#include <qdebug.h>

/// Constructs a QXmppInvokable with the specified \a parent.
QXmppInvokable::QXmppInvokable(QObject *parent)
    : QObject(parent)
{
}

QXmppInvokable::~QXmppInvokable() = default;

QVariant QXmppInvokable::dispatch(const QByteArray &method, const QList<QVariant> &args)
{
    buildMethodHash();

    if (!m_methodHash.contains(method)) {
        return QVariant();
    }

    int idx = m_methodHash[method];
    if (paramTypes(args) != metaObject()->method(idx).parameterTypes()) {
        return QVariant();
    }

    const char *typeName = metaObject()->method(idx).typeName();
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QMetaType resultType = metaObject()->method(idx).returnMetaType();

    void *result = resultType.create(nullptr);
#else
    int resultType = QMetaType::type(typeName);

    void *result = QMetaType::create(resultType, nullptr);
#endif

    QGenericReturnArgument ret(typeName, result);
    QList<QGenericArgument> genericArgs;
    QList<QVariant>::ConstIterator iter = args.begin();
    while (iter != args.end()) {
        const void *data = iter->data();
        const char *name = iter->typeName();
        genericArgs << QGenericArgument(name, data);
        ++iter;
    }

    if (QMetaObject::invokeMethod(this, method.constData(), ret,
                                  genericArgs.value(0, QGenericArgument()),
                                  genericArgs.value(1, QGenericArgument()),
                                  genericArgs.value(2, QGenericArgument()),
                                  genericArgs.value(3, QGenericArgument()),
                                  genericArgs.value(4, QGenericArgument()),
                                  genericArgs.value(5, QGenericArgument()),
                                  genericArgs.value(6, QGenericArgument()),
                                  genericArgs.value(7, QGenericArgument()),
                                  genericArgs.value(8, QGenericArgument()),
                                  genericArgs.value(9, QGenericArgument()))) {
        QVariant returnValue(resultType, result);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        resultType.destroy(result);
#else
        QMetaType::destroy(resultType, result);
#endif
        return returnValue;
    } else {
        qDebug("No such method '%s'", method.constData());
        return QVariant();
    }
}

QList<QByteArray> QXmppInvokable::paramTypes(const QList<QVariant> &params)
{
    QList<QByteArray> types;
    for (const auto &variant : std::as_const(params)) {
        types << variant.typeName();
    }
    return types;
}

void QXmppInvokable::buildMethodHash()
{
    QWriteLocker locker(&m_lock);
    if (m_methodHash.size() > 0) {
        return;
    }

    int methodCount = metaObject()->methodCount();
    for (int idx = 0; idx < methodCount; ++idx) {
        QByteArray signature = metaObject()->method(idx).methodSignature();
        m_methodHash[signature.left(signature.indexOf('('))] = idx;
        //         qDebug() << metaObject()->method(idx).parameterTypes();
    }
}

QStringList QXmppInvokable::interfaces() const
{
    QStringList results;
    int methodCount = metaObject()->methodCount();
    for (int idx = 0; idx < methodCount; ++idx) {
        if (metaObject()->method(idx).methodType() == QMetaMethod::Slot) {
            QByteArray signature = metaObject()->method(idx).methodSignature();
            results << QString::fromUtf8(signature.left(signature.indexOf(u'(')));
        }
    }
    return results;
}
