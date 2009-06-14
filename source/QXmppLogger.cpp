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


#include "QXmppLogger.h"
#include <iostream>
#include <QTime>

QXmppLogger* QXmppLogger::m_logger = 0;
QXmppLogger::LoggingType QXmppLogger::m_loggingType = QXmppLogger::FILE;
QFile QXmppLogger::m_file("QXmppClientLog.log");
QTextStream QXmppLogger::m_stream;

QXmppLogger::QXmppLogger()
{
}

QXmppLogger* QXmppLogger::getLogger()
{
    if(!m_logger)
        m_logger = new QXmppLogger();

    return m_logger;
}

void QXmppLogger::setLoggingType(QXmppLogger::LoggingType log)
{
    m_loggingType = log;
}

QXmppLogger::LoggingType QXmppLogger::getLoggingType()
{
    return m_loggingType;
}

void QXmppLogger::log(const QString& str)
{
    switch(m_loggingType)
    {
    case QXmppLogger::FILE:
        m_file.open(QIODevice::Append);
        m_stream.setDevice(&m_file);
        m_stream << QTime::currentTime().toString("hh:mm:ss.zzz") << " : "<< str << "\n\n";
        m_file.close();
        break;
    case QXmppLogger::STDOUT:
        std::cout<<qPrintable(str)<<std::endl;
        break;
    case QXmppLogger::NONE:
        break;
    default:
        break;
    }
}

void QXmppLogger::log(const QByteArray& str)
{
    switch(m_loggingType)
    {
    case QXmppLogger::FILE:
        m_file.open(QIODevice::Append);
        m_stream.setDevice(&m_file);
        m_stream << QTime::currentTime().toString("hh:mm:ss.zzz") << " : "<< str << "\n\n";
        m_file.close();
        break;
    case QXmppLogger::STDOUT:
        std::cout<<str.constData()<<std::endl;
        break;
    case QXmppLogger::NONE:
        break;
    default:
        break;
    }
}
