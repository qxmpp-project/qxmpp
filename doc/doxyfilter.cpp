/*
 * Copyright (C) 2008-2010 The QXmpp developers
 *
 * Author:
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

#include <cstdlib>

#include <QCoreApplication>
#include <QFile>
#include <QRegExp>
#include <QTextStream>

#include "QXmppGlobal.h"

static void setField(QString &code, const QString &name, const QString &value)
{
    code.replace(
        QRegExp(QString("(%1\\s*=)[^\\r\\n]*").arg(name)),
        QString("\\1 %1").arg(value));
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    if (argc != 2) {
        qWarning("Usage: doxyfilter <sourcefile>");
        return 1;
    }

    // read source code
    QFile source(QString::fromLocal8Bit(argv[1]));
    if (!source.open(QIODevice::ReadOnly)) {
        qWarning("Could not open %s", qPrintable(source.fileName()));
        return 1;
    }
    QString code = QString::fromUtf8(source.readAll());
    source.close();

    if (source.fileName() == "Doxyfile") {
        // adjust doxyfile
        setField(code, "ALPHABETICAL_INDEX", "NO");
        setField(code, "EXCLUDE_PATTERNS", "*/moc_*");
        setField(code, "FULL_PATH_NAMES", "NO");
        setField(code, "HIDE_UNDOC_CLASSES", "YES");
        setField(code, "GENERATE_LATEX", "NO");
        setField(code, "HTML_TIMESTAMP", "NO");
        setField(code, "INPUT", "../src");
        setField(code, "INPUT_FILTER", QString::fromLocal8Bit(argv[0]));
        setField(code, "PROJECT_NAME", "QXmpp");
        setField(code, "PROJECT_NUMBER", QString("%1.%2.%3").arg(
                QString::number((QXMPP_VERSION >> 16) & 0xff),
                QString::number((QXMPP_VERSION >> 8) & 0xff),
                QString::number(QXMPP_VERSION & 0xff)));

        // write doxyfile
        if (!source.open(QIODevice::WriteOnly)) {
            qWarning("Could not write to %s", qPrintable(source.fileName()));
            return 1;
        }
        source.write(code.toUtf8());
        source.close();
    }
    else {
        // add links for XEPs
        code.replace(QRegExp("(XEP-([0-9]{4}))"), "<a href=\"http://xmpp.org/extensions/xep-\\2.html\">\\1</a>");

        QTextStream output(stdout);
        output << code;
    }
    return 0;
}

