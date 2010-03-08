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

#include <QDebug>
#include <QFile>
#include <QTime>

#include "QXmppLogger.h"

QXmppLogger* QXmppLogger::m_logger = 0;

QXmppLogger::QXmppLogger()
    : m_loggingType(QXmppLogger::NONE), m_device(0)
{
}

QXmppLogger::~QXmppLogger()
{
    delete m_device;
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
    if (m_device)
    {
        delete m_device;
        m_device = 0;
    }

    QFile *file = 0;
    switch(log)
    {
    case QXmppLogger::FILE:
        file = new QFile("QXmppClientLog.log");
        file->open(QIODevice::Append);
        break;
    case QXmppLogger::STDOUT:
        file = new QFile();
        file->open(stdout, QIODevice::WriteOnly);
        break;
    case QXmppLogger::NONE:
        break;
    }
    m_device = file;
    m_loggingType = log;
}

QXmppLogger::LoggingType QXmppLogger::loggingType()
{
    return m_loggingType;
}

QDebug QXmppLogger::debug()
{
    if (m_device)
        return QDebug(m_device) << QTime::currentTime().toString("hh:mm:ss.zzz") << ":";
    return QDebug(m_device);
}

QXmppLogger::LoggingType QXmppLogger::getLoggingType()
{
    return m_loggingType;
}

