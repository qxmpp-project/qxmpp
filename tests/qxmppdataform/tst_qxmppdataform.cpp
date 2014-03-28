/*
 * Copyright (C) 2008-2014 The QXmpp developers
 *
 * Authors:
 *  Andrey Batyiev
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

#include <QObject>
#include "QXmppDataForm.h"
#include "util.h"

class tst_QXmppDataForm : public QObject
{
    Q_OBJECT

private slots:
    void testSimple();
    void testSubmit();
    void testMedia();
};

void tst_QXmppDataForm::testSimple()
{
    const QByteArray xml(
        "<x xmlns=\"jabber:x:data\" type=\"form\">"
        "<title>Joggle Search</title>"
        "<instructions>Fill out this form to search for information!</instructions>"
        "<field type=\"text-single\" var=\"search_request\">"
        "<required/>"
        "</field>"
        "</x>");

    QXmppDataForm form;
    parsePacket(form, xml);

    QCOMPARE(form.isNull(), false);
    QCOMPARE(form.title(), QLatin1String("Joggle Search"));
    QCOMPARE(form.instructions(), QLatin1String("Fill out this form to search for information!"));
    QCOMPARE(form.fields().size(), 1);
    QCOMPARE(form.fields().at(0).type(), QXmppDataForm::Field::TextSingleField);
    QCOMPARE(form.fields().at(0).isRequired(), true);
    QCOMPARE(form.fields().at(0).key(), QString("search_request"));

    serializePacket(form, xml);
}

void tst_QXmppDataForm::testSubmit()
{
    const QByteArray xml(
        "<x xmlns=\"jabber:x:data\" type=\"submit\">"
        "<field type=\"text-single\" var=\"search_request\">"
        "<value>verona</value>"
        "</field>"
        "</x>");

    QXmppDataForm form;
    parsePacket(form, xml);
    QCOMPARE(form.isNull(), false);
    serializePacket(form, xml);
}

void tst_QXmppDataForm::testMedia()
{
    const QByteArray xml(
        "<x xmlns=\"jabber:x:data\" type=\"form\">"
        "<field type=\"text-single\" label=\"Enter the text you see\" var=\"ocr\">"
        "<value/>"
        "<media xmlns=\"urn:xmpp:media-element\" height=\"80\" width=\"290\">"
        "<uri type=\"image/jpeg\">"
        "http://www.victim.com/challenges/ocr.jpeg?F3A6292C"
        "</uri>"
        "<uri type=\"image/png\">"
        "cid:sha1+f24030b8d91d233bac14777be5ab531ca3b9f102@bob.xmpp.org"
        "</uri>"
        "</media>"
        "</field>"
        "</x>");

    QXmppDataForm form;
    parsePacket(form, xml);

    QCOMPARE(form.isNull(), false);
    QCOMPARE(form.fields().size(), 1);
    QCOMPARE(form.fields().at(0).type(), QXmppDataForm::Field::TextSingleField);
    QCOMPARE(form.fields().at(0).isRequired(), false);
    QCOMPARE(form.fields().at(0).media().uris().size(), 2);
    QCOMPARE(form.fields().at(0).media().isNull(), false);
    QCOMPARE(form.fields().at(0).media().height(), 80);
    QCOMPARE(form.fields().at(0).media().width(), 290);
    QCOMPARE(form.fields().at(0).media().uris().at(0).first, QString("image/jpeg"));
    QCOMPARE(form.fields().at(0).media().uris().at(0).second, QString("http://www.victim.com/challenges/ocr.jpeg?F3A6292C"));
    QCOMPARE(form.fields().at(0).media().uris().at(1).first, QString("image/png"));
    QCOMPARE(form.fields().at(0).media().uris().at(1).second, QString("cid:sha1+f24030b8d91d233bac14777be5ab531ca3b9f102@bob.xmpp.org"));

    serializePacket(form, xml);
}

QTEST_MAIN(tst_QXmppDataForm)
#include "tst_qxmppdataform.moc"
