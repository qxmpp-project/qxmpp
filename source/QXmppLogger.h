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


#ifndef QXMPPLOGGER_H
#define QXMPPLOGGER_H

#include <QTextStream>
#include <QFile>

/// Singleton class
class QXmppLogger
{
public:
    enum LoggingType
    {
        NONE = 0,
        FILE,
        STDOUT
    };

    static QXmppLogger &defaultLogger();
    QXmppLogger::LoggingType loggingType();
    void setLoggingType(QXmppLogger::LoggingType);

    QXmppLogger& operator<<(const QByteArray &b);
    QXmppLogger& operator<<(const QString &str);

    // deprecated methods
    static QXmppLogger* Q_DECL_DEPRECATED getLogger();
    QXmppLogger::LoggingType Q_DECL_DEPRECATED getLoggingType();

private:
    QXmppLogger();
    static QXmppLogger* m_logger;
    static QXmppLogger::LoggingType m_loggingType;
    static QFile m_file;
    static QTextStream m_stream;
};

#endif // QXMPPLOGGER_H
