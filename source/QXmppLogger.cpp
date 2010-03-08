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

#include <QFile>
#include <QTime>

#include "QXmppLogger.h"

QXmppLogger* QXmppLogger::m_logger = 0;

QXmppLogger::QXmppLogger()
    : m_loggingType(QXmppLogger::NONE), m_file(0)
{
}

QXmppLogger::~QXmppLogger()
{
    delete m_file;
}

QXmppLogger* QXmppLogger::getLogger()
{
    if(!m_logger)
    {
        m_logger = new QXmppLogger();
        m_logger->setLoggingType(QXmppLogger::FILE);
    }

    return m_logger;
}

void QXmppLogger::setLoggingType(QXmppLogger::LoggingType log)
{
    if (m_file)
    {
        delete m_file;
        m_file = 0;
    }
    switch(log)
    {
    case QXmppLogger::FILE:
        m_file = new QFile("QXmppClientLog.log");
        m_file->open(QIODevice::Append);
        break;
    case QXmppLogger::STDOUT:
        m_file = new QFile();
        m_file->open(stdout, QIODevice::WriteOnly);
        break;
    case QXmppLogger::NONE:
        break;
    }
    m_stream.setDevice(m_file);
    m_loggingType = log;
}

QXmppLogger::LoggingType QXmppLogger::loggingType()
{
    return m_loggingType;
}

QXmppLogger &QXmppLogger::operator<<(const QByteArray &str)
{
    if (m_loggingType != NONE)
    {
        m_stream << QTime::currentTime().toString("hh:mm:ss.zzz") << " : "<<
                str << "\n\n";
        m_stream.flush();
    }
    return *this;
}

QXmppLogger &QXmppLogger::operator<<(const QString &str)
{
    return (*this << str.toLocal8Bit());
}

QXmppLogger::LoggingType QXmppLogger::getLoggingType()
{
    return m_loggingType;
}

