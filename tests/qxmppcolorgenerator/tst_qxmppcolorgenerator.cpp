/*
 * Copyright (C) 2008-2019 The QXmpp developers
 *
 * Author:
 *  Linus Jahn <lnj@kaidan.im>
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

#include <QObject>
#include "util.h"

#include "QXmppColorGenerator.h"

Q_DECLARE_METATYPE(QXmppColorGenerator::ColorVisionDeficiency)

class tst_QXmppColorGenerator : public QObject
{
    Q_OBJECT

private slots:
    void testBase_data();
    void testBase();
};

void tst_QXmppColorGenerator::testBase_data()
{
    QTest::addColumn<QString>("name");
    QTest::addColumn<QXmppColorGenerator::ColorVisionDeficiency>("deficiency");
    QTest::addColumn<QString>("result");

    QTest::newRow("normal")
        << "4223@localhost"
        << QXmppColorGenerator::NoDeficiency
        << "124,122,0";
    QTest::newRow("red-green-blindness-not-corrected")
        << "somebody-zz@localhost"
        << QXmppColorGenerator::NoDeficiency
        << "0,137,74";
    QTest::newRow("red-green-blindness-corrected")
        << "somebody-zz@localhost"
        << QXmppColorGenerator::RedGreenBlindness
        << "217,0,187";
    QTest::newRow("blue-blindness-not-corrected")
        << "357@localhost"
        << QXmppColorGenerator::NoDeficiency
        << "0,133,123";
    QTest::newRow("blue-blindness-corrected")
        << "357@localhost"
        << QXmppColorGenerator::BlueBlindness
        << "233,0,102";
}

void tst_QXmppColorGenerator::testBase()
{
    QFETCH(QString, name);
    QFETCH(QXmppColorGenerator::ColorVisionDeficiency, deficiency);
    QFETCH(QString, result);

    QXmppColorGenerator::RGBColor color = QXmppColorGenerator::generateColor(name, deficiency);
    // check color
    QCOMPARE(result, QString("%1,%2,%3").arg(color.red).arg(color.green).arg(color.blue));
}

QTEST_MAIN(tst_QXmppColorGenerator)
#include "tst_qxmppcolorgenerator.moc"

