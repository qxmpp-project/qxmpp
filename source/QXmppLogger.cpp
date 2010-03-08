/*
 * Copyright (C) 2008-2010 Manjeet Dahiya
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

#include <iostream>

#include <QTextStream>
#include <QFile>
#include <QTime>

#include "QXmppLogger.h"

QXmppLogger* QXmppLogger::m_logger = 0;

QXmppLogger::QXmppLogger(QObject *parent)
    : QObject(parent), m_loggingType(QXmppLogger::NONE)
{
}

QXmppLogger* QXmppLogger::getLogger()
{
    if(!m_logger)
    {
        m_logger = new QXmppLogger();
        m_logger->setLoggingType(FILE);
    }

    return m_logger;
}

void QXmppLogger::setLoggingType(QXmppLogger::LoggingType log)
{
    m_loggingType = log;
}

QXmppLogger::LoggingType QXmppLogger::loggingType()
{
    return m_loggingType;
}

void QXmppLogger::debug(const QString &str)
{
    log(QtDebugMsg, str);
}

void QXmppLogger::warning(const QString &str)
{
    log(QtWarningMsg, str);
}

void QXmppLogger::log(QtMsgType type, const QString& str)
{
    switch(m_loggingType)
    {
    case QXmppLogger::FILE:
        {
            QFile file("QXmppClientLog.log");
            file.open(QIODevice::Append);
            QTextStream stream(&file);
            stream << QTime::currentTime().toString("hh:mm:ss.zzz") << " : " <<
                str << "\n\n";
        }
        break;
    case QXmppLogger::STDOUT:
        std::cout << qPrintable(str) << std::endl;
        break;
    default:
        break;
    }
    emit message(type, str);
}

QXmppLogger::LoggingType QXmppLogger::getLoggingType()
{
    return m_loggingType;
}

