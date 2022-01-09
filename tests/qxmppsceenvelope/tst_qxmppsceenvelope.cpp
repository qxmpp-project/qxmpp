/*
 * Copyright (C) 2008-2022 The QXmpp developers
 *
 * Author:
 *  Linus Jahn
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

#include "QXmppSceEnvelope_p.h"

#include "util.h"
#include <QObject>

class tst_QXmppSceEnvelope : public QObject
{
    Q_OBJECT

private:
    Q_SLOT void testReader();
    Q_SLOT void testWriter();
};

void tst_QXmppSceEnvelope::testReader()
{
    const auto xml = QStringLiteral(
        "<envelope xmlns=\"urn:xmpp:sce:1\">"
        "<content><body xmlns=\"jabber:client\">Hello</body><x xmlns=\"jabber:x:oob\"><url>https://en.wikipedia.org/wiki/Fight_Club#Plot</url></x></content>"
        "<time stamp=\"2004-01-25T06:05:00+01:00\"/>"
        "<to jid=\"missioncontrol@houston.nasa.gov\"/>"
        "<from jid=\"opportunity@mars.planet\"/>"
        "<rpad>C1DHN9HK-9A25tSmwK4hU!Jji9%GKYK^syIlHJT9TnI4</rpad>"
        "</envelope>");
    const auto dom = xmlToDom(xml);

    QXmppSceEnvelopeReader reader(dom);
    QCOMPARE(reader.from(), QStringLiteral("opportunity@mars.planet"));
    QCOMPARE(reader.to(), QStringLiteral("missioncontrol@houston.nasa.gov"));
    QCOMPARE(reader.timestamp(), QDateTime({2004, 01, 25}, {05, 05, 00}, Qt::UTC));
    QCOMPARE(reader.contentElement().firstChildElement().tagName(), QStringLiteral("body"));
}

void tst_QXmppSceEnvelope::testWriter()
{
    const auto expectedXml = QStringLiteral(
        "<envelope xmlns=\"urn:xmpp:sce:1\">"
        "<content><body xmlns=\"jabber:client\">Hello</body><x xmlns=\"jabber:x:oob\"><url>https://en.wikipedia.org/wiki/Fight_Club#Plot</url></x></content>"
        "<time stamp=\"2004-01-25T05:05:00Z\"/>"
        "<to jid=\"missioncontrol@houston.nasa.gov\"/>"
        "<from jid=\"opportunity@mars.planet\"/>"
        "<rpad>C1DHN9HK-9A25tSmwK4hU!Jji9%GKYK^syIlHJT9TnI4</rpad>"
        "</envelope>");

    QString out;
    QXmlStreamWriter writer(&out);
    QXmppSceEnvelopeWriter envelope(writer);
    envelope.start();
    envelope.writeContent([&writer] {
        writer.writeStartElement("body");
        writer.writeDefaultNamespace("jabber:client");
        writer.writeCharacters("Hello");
        writer.writeEndElement();
        writer.writeStartElement("x");
        writer.writeDefaultNamespace("jabber:x:oob");
        writer.writeTextElement("url", "https://en.wikipedia.org/wiki/Fight_Club#Plot");
        writer.writeEndElement();
    });
    envelope.writeTimestamp(QDateTime({2004, 01, 25}, {05, 05, 00}, Qt::UTC));
    envelope.writeTo("missioncontrol@houston.nasa.gov");
    envelope.writeFrom("opportunity@mars.planet");
    envelope.writeRpad("C1DHN9HK-9A25tSmwK4hU!Jji9%GKYK^syIlHJT9TnI4");
    envelope.end();

    QCOMPARE(out, expectedXml);
}

QTEST_MAIN(tst_QXmppSceEnvelope)
#include "tst_qxmppsceenvelope.moc"
