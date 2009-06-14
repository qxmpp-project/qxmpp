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


#ifndef QXMPPLOGGER_H
#define QXMPPLOGGER_H

#include <QTextStream>
#include <QFile>

/// Singelton class
class QXmppLogger
{
public:
    enum LoggingType
    {
        NONE = 0,
        FILE,
        STDOUT
    };

    static QXmppLogger* getLogger();
    QXmppLogger::LoggingType getLoggingType();
    void setLoggingType(QXmppLogger::LoggingType);
    void log(const QString& str);
    void log(const QByteArray& str);

private:
    QXmppLogger();
    static QXmppLogger* m_logger;
    static QXmppLogger::LoggingType m_loggingType;
    static QFile m_file;
    static QTextStream m_stream;
};

#endif // QXMPPLOGGER_H
