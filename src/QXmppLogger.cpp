/*
 * Copyright (C) 2008-2010 The QXmpp developers
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

#include <QTextStream>
#include <QFile>
#include <QTime>

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

/// Constructs a new QXmppLogger.
///
/// \param parent

QXmppLogger::QXmppLogger(QObject *parent)
    : QObject(parent), m_loggingType(QXmppLogger::NoLogging),
    m_logFilePath("QXmppClientLog.log")
{
}

/// Returns the default logger.
///

QXmppLogger* QXmppLogger::getLogger()
{
    if(!m_logger)
    {
        m_logger = new QXmppLogger();
        m_logger->setLoggingType(FileLogging);
    }

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

/// Add a logging message.
///
/// \param type
/// \param text

void QXmppLogger::log(QXmppLogger::MessageType type, const QString& text)
{
    switch(m_loggingType)
    {
    case QXmppLogger::FileLogging:
        {
            QFile file(m_logFilePath);
            file.open(QIODevice::Append);
            QTextStream stream(&file);
            stream << QTime::currentTime().toString("hh:mm:ss.zzz") <<
                " " << typeName(type) << " " <<
                text << "\n\n";
        }
        break;
    case QXmppLogger::StdoutLogging:
        std::cout << typeName(type) << " " << qPrintable(text) << std::endl;
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

