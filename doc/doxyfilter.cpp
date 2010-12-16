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

    // add links for XEPs
    code.replace(QRegExp("(XEP-([0-9]{4}))"), "<a href=\"http://xmpp.org/extensions/xep-\\2.html\">\\1</a>");
    QTextStream output(stdout);
    output << code;
    return 0;
}

