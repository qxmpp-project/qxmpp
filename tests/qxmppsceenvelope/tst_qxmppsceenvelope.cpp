// SPDX-FileCopyrightText: 2021 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

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

    QXmppSceEnvelopeReader reader(xmlToDom(xml));
    QCOMPARE(reader.from(), u"opportunity@mars.planet"_s);
    QCOMPARE(reader.to(), u"missioncontrol@houston.nasa.gov"_s);
    QCOMPARE(reader.timestamp(), QDateTime({ 2004, 01, 25 }, { 05, 05, 00 }, TimeZoneUTC));
    QCOMPARE(reader.contentElement().firstChildElement().tagName(), u"body"_s);
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
    envelope.writeTimestamp(QDateTime({ 2004, 01, 25 }, { 05, 05, 00 }, TimeZoneUTC));
    envelope.writeTo("missioncontrol@houston.nasa.gov");
    envelope.writeFrom("opportunity@mars.planet");
    envelope.writeRpad("C1DHN9HK-9A25tSmwK4hU!Jji9%GKYK^syIlHJT9TnI4");
    envelope.end();

    QCOMPARE(out, expectedXml);
}

QTEST_MAIN(tst_QXmppSceEnvelope)
#include "tst_qxmppsceenvelope.moc"
