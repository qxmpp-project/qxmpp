/*
 * Copyright (C) 2008-2014 The QXmpp developers
 *
 * Authors:
 *  Manjeet Dahiya
 *  Jeremy Lainé
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

#include <iostream>

#include <QChildEvent>
#include <QDateTime>
#include <QFile>
#include <QMetaType>
#include <QTextStream>

#include "QXmppLogger.h"

QXmppLogger* QXmppLogger::m_logger = 0;

static const char *typeName(QXmppLogger::MessageType type)
{
    switch (type)
    {
    case QXmppLogger::DebugMessage:
        return "DEBUG";
    case QXmppLogger::InformationMessage:
        return "INFO";
    case QXmppLogger::WarningMessage:
        return "WARNING";
    case QXmppLogger::ReceivedMessage:
        return "RECEIVED";
    case QXmppLogger::SentMessage:
        return "SENT";
    default:
        return "";
    }
}

static QString formatted(QXmppLogger::MessageType type, const QString& text)
{
    return QDateTime::currentDateTime().toString() + " " +
        QString::fromLatin1(typeName(type)) + " " +
        text;
}

static void relaySignals(QXmppLoggable *from, QXmppLoggable *to)
{
    QObject::connect(from, SIGNAL(logMessage(QXmppLogger::MessageType,QString)),
                     to, SIGNAL(logMessage(QXmppLogger::MessageType,QString)));
    QObject::connect(from, SIGNAL(setGauge(QString,double)),
                     to, SIGNAL(setGauge(QString,double)));
    QObject::connect(from, SIGNAL(updateCounter(QString,qint64)),
                     to, SIGNAL(updateCounter(QString,qint64)));
}

/// Constructs a new QXmppLoggable.
///
/// \param parent

QXmppLoggable::QXmppLoggable(QObject *parent)
    : QObject(parent)
{
    QXmppLoggable *logParent = qobject_cast<QXmppLoggable*>(parent);
    if (logParent) {
        relaySignals(this, logParent);
    }
}

/// \cond
void QXmppLoggable::childEvent(QChildEvent *event)
{
    QXmppLoggable *child = qobject_cast<QXmppLoggable*>(event->child());
    if (!child)
        return;

    if (event->added()) {
        relaySignals(child, this);
    } else if (event->removed()) {
        disconnect(child, SIGNAL(logMessage(QXmppLogger::MessageType,QString)),
                this, SIGNAL(logMessage(QXmppLogger::MessageType,QString)));
        disconnect(child, SIGNAL(setGauge(QString,double)),
                this, SIGNAL(setGauge(QString,double)));
        disconnect(child, SIGNAL(updateCounter(QString,qint64)),
                this, SIGNAL(updateCounter(QString,qint64)));
    }
}
/// \endcond

class QXmppLoggerPrivate
{
public:
    QXmppLoggerPrivate(QXmppLogger *qq);

    QXmppLogger::LoggingType loggingType;
    QFile *logFile;
    QString logFilePath;
    QXmppLogger::MessageTypes messageTypes;

private:
    QXmppLogger *q;
};

QXmppLoggerPrivate::QXmppLoggerPrivate(QXmppLogger *qq)
    : loggingType(QXmppLogger::NoLogging),
    logFile(0),
    logFilePath("QXmppClientLog.log"),
    messageTypes(QXmppLogger::AnyMessage),
    q(qq)
{
}

/// Constructs a new QXmppLogger.
///
/// \param parent

QXmppLogger::QXmppLogger(QObject *parent)
    : QObject(parent)
{
    d = new QXmppLoggerPrivate(this);

    // make it possible to pass QXmppLogger::MessageType between threads
    qRegisterMetaType< QXmppLogger::MessageType >("QXmppLogger::MessageType");
}

QXmppLogger::~QXmppLogger()
{
    delete d;
}

/// Returns the default logger.
///

QXmppLogger* QXmppLogger::getLogger()
{
    if(!m_logger)
        m_logger = new QXmppLogger();

    return m_logger;
}

/// Returns the handler for logging messages.
///

QXmppLogger::LoggingType QXmppLogger::loggingType()
{
    return d->loggingType;
}

/// Sets the handler for logging messages.
///
/// \param type

void QXmppLogger::setLoggingType(QXmppLogger::LoggingType type)
{
    if (d->loggingType != type) {
        d->loggingType = type;
        reopen();
    }
}

/// Returns the types of messages to log.
///

QXmppLogger::MessageTypes QXmppLogger::messageTypes()
{
    return d->messageTypes;
}

/// Sets the types of messages to log.
///
/// \param types

void QXmppLogger::setMessageTypes(QXmppLogger::MessageTypes types)
{
    d->messageTypes = types;
}

/// Add a logging message.
///
/// \param type
/// \param text

void QXmppLogger::log(QXmppLogger::MessageType type, const QString& text)
{
    // filter messages
    if (!d->messageTypes.testFlag(type))
        return;

    switch(d->loggingType)
    {
    case QXmppLogger::FileLogging:
        if (!d->logFile) {
            d->logFile = new QFile(d->logFilePath);
            d->logFile->open(QIODevice::WriteOnly | QIODevice::Append);
        }
        QTextStream(d->logFile) << formatted(type, text) << "\n";
        break;
    case QXmppLogger::StdoutLogging:
        std::cout << qPrintable(formatted(type, text)) << std::endl;
        break;
    case QXmppLogger::SignalLogging:
        emit message(type, text);
        break;
    default:
        break;
    }
}

/// Sets the given \a gauge to \a value.
///
/// NOTE: the base implementation does nothing.

void QXmppLogger::setGauge(const QString &gauge, double value)
{
    Q_UNUSED(gauge);
    Q_UNUSED(value);
}

/// Updates the given \a counter by \a amount.
///
/// NOTE: the base implementation does nothing.

void QXmppLogger::updateCounter(const QString &counter, qint64 amount)
{
    Q_UNUSED(counter);
    Q_UNUSED(amount);
}

/// Returns the path to which logging messages should be written.
///
/// \sa loggingType()

QString QXmppLogger::logFilePath()
{
    return d->logFilePath;
}

/// Sets the path to which logging messages should be written.
///
/// \param path
///
/// \sa setLoggingType()

void QXmppLogger::setLogFilePath(const QString &path)
{
    if (d->logFilePath != path) {
        d->logFilePath = path;
        reopen();
    }
}

/// If logging to a file, causes the file to be re-opened.
///

void QXmppLogger::reopen()
{
    if (d->logFile) {
        delete d->logFile;
        d->logFile = 0;
    }
}

