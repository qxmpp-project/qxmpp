/*
 * Copyright (C) 2008-2011 The QXmpp developers
 *
 * Authors:
 *  Manjeet Dahiya
 *  Jeremy Lainé
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

#include <iostream>

#include <QChildEvent>
#include <QDateTime>
#include <QTextStream>
#include <QFile>

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

/// Constructs a new QXmppLoggable.
///
/// \param parent

QXmppLoggable::QXmppLoggable(QObject *parent)
    : QObject(parent)
{
    QXmppLoggable *logParent = qobject_cast<QXmppLoggable*>(parent);
    if (logParent) {
        connect(this, SIGNAL(logMessage(QXmppLogger::MessageType,QString)),
                logParent, SIGNAL(logMessage(QXmppLogger::MessageType,QString)));
    }
}

void QXmppLoggable::childEvent(QChildEvent *event)
{
    QXmppLoggable *child = qobject_cast<QXmppLoggable*>(event->child());
    if (!child)
        return;

    if (event->added()) {
        connect(child, SIGNAL(logMessage(QXmppLogger::MessageType,QString)),
                this, SIGNAL(logMessage(QXmppLogger::MessageType,QString)));
    } else if (event->removed()) {
        disconnect(child, SIGNAL(logMessage(QXmppLogger::MessageType,QString)),
                this, SIGNAL(logMessage(QXmppLogger::MessageType,QString)));
    }
}

/// Constructs a new QXmppLogger.
///
/// \param parent

QXmppLogger::QXmppLogger(QObject *parent)
    : QObject(parent),
    m_loggingType(QXmppLogger::NoLogging),
    m_logFilePath("QXmppClientLog.log"),
    m_messageTypes(QXmppLogger::AnyMessage)
{
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
    return m_loggingType;
}

/// Sets the handler for logging messages.
///
/// \param type

void QXmppLogger::setLoggingType(QXmppLogger::LoggingType type)
{
    m_loggingType = type;
}

/// Returns the types of messages to log.
///

QXmppLogger::MessageTypes QXmppLogger::messageTypes()
{
    return m_messageTypes;
}

/// Sets the types of messages to log.
///
/// \param types

void QXmppLogger::setMessageTypes(QXmppLogger::MessageTypes types)
{
    m_messageTypes = types;
}

/// Add a logging message.
///
/// \param type
/// \param text

void QXmppLogger::log(QXmppLogger::MessageType type, const QString& text)
{
    // filter messages
    if (!m_messageTypes.testFlag(type))
        return;

    switch(m_loggingType)
    {
    case QXmppLogger::FileLogging:
        {
            QFile file(m_logFilePath);
            file.open(QIODevice::Append);
            QTextStream stream(&file);
            stream << formatted(type, text) << "\n";
        }
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

/// Returns the path to which logging messages should be written.
///
/// \sa loggingType()

QString QXmppLogger::logFilePath()
{
    return m_logFilePath;
}

/// Sets the path to which logging messages should be written.
///
/// \param path
///
/// \sa setLoggingType()

void QXmppLogger::setLogFilePath(const QString &path)
{
    m_logFilePath = path;
}

