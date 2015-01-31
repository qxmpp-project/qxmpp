/*
 * Copyright (C) 2008-2014 The QXmpp developers
 *
 * Author:
 *  Jeremy Lainé
 *
 * Source:
 *  https://github.com/qxmpp-project/qxmpp
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
#include <iostream>

#include <QCoreApplication>
#include <QFile>
#include <QProcess>
#include <QRegExp>
#include <QTextStream>

#include "QXmppGlobal.h"

static void setField(QString &code, const QString &name, const QString &value)
{
    code.replace(
        QRegExp(QString("(%1\\s*=)[^\\r\\n]*").arg(name)),
        QString("\\1 %1").arg(value));
}

static void usage() {
    QTextStream output(stderr);
    output << "Usage:" << endl;
    output << "  doxyfilter              Generate documentation" << endl;
    output << "  doxyfilter -g           Generate Doxyfile" << endl;
    output << "  doxyfilter <sourcefile> Filter the given file's code" << endl;
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    if (argc == 1)
        return QProcess::execute("doxygen");
    else if (argc < 2) {
        usage();
        return 1;
    }

    if (!strcmp(argv[1], "-g")) {
        // generate default Doxyfile
        QProcess process;
        process.start("doxygen", QStringList() << "-g" << "-");
        if (!process.waitForFinished()) {
            qWarning("Could not run doxygen");
            return 1;
        }
        QString code = QString::fromUtf8(process.readAll());

        QString docDir = (argc > 2) ? (QString::fromLocal8Bit(argv[2]) + "/") : "";
        QStringList docFiles = QStringList() << "index.doc" << "using.doc" << "xep.doc" << "../src";
        for (int i = 0; i < docFiles.size(); ++i) {
            docFiles[i] = docDir + docFiles[i];
        }

        // adjust Doxyfile
        setField(code, "ALPHABETICAL_INDEX", "NO");
        setField(code, "EXCLUDE_PATTERNS", "*/moc_* */mod_* */qdnslookup* */*_p.h */QXmppCodec.cpp */QXmppSasl.cpp");
        setField(code, "FULL_PATH_NAMES", "NO");
        setField(code, "HIDE_UNDOC_CLASSES", "YES");
        setField(code, "GENERATE_LATEX", "NO");
        setField(code, "HTML_TIMESTAMP", "NO");
        setField(code, "INPUT", docFiles.join(" "));
        setField(code, "INPUT_FILTER", QString::fromLocal8Bit(argv[0]));
        setField(code, "PROJECT_NAME", "QXmpp");
        setField(code, "PROJECT_NUMBER", QString("Version: %1.%2.%3").arg(
                QString::number((QXMPP_VERSION >> 16) & 0xff),
                QString::number((QXMPP_VERSION >> 8) & 0xff),
                QString::number(QXMPP_VERSION & 0xff)));
        setField(code, "QUIET", "YES");
        setField(code, "RECURSIVE", "YES");

        // write doxyfile
        QFile output("Doxyfile");
        if (!output.open(QIODevice::WriteOnly)) {
            qWarning("Could not write to %s", qPrintable(output.fileName()));
            return 1;
        }
        output.write(code.toUtf8());
        output.close();

    } else if (!strcmp(argv[1], "-h")) {
        usage();
        return 0;
    } else {
        // read source code
        QFile source(QString::fromLocal8Bit(argv[1]));
        if (!source.open(QIODevice::ReadOnly)) {
            qWarning("Could not open %s", qPrintable(source.fileName()));
            return 1;
        }
        QString code = QString::fromUtf8(source.readAll());
        source.close();

        // add links for RFCs
        QRegExp rfcRegexp("(RFC ([0-9]{4})(: [^\\s.]+( [A-Z][^\\s.]*)*)?)");
        code.replace(rfcRegexp, "<a href=\"http://www.rfc-editor.org/rfc/rfc\\2.txt\">\\1</a>");

        // add links for XEPs
        QRegExp regexp("(XEP-([0-9]{4})(: [^\\s.]+( [A-Z][^\\s.]*)*)?)");
        code.replace(regexp, "<a href=\"http://xmpp.org/extensions/xep-\\2.html\">\\1</a>");

        QTextStream output(stdout);
        output << code;
    }

    return 0;
}

