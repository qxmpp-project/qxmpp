/*
 * Copyright (C) 2008-2019 The QXmpp developers
 *
 * Authors:
 *  Andrey Batyiev
 *  Jeremy Lainé
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

#include <QtGlobal>
#include <QObject>

// deprecated methods are also tested: this is used to avoid unnecessary warnings
#undef QT_DEPRECATED_X
#define QT_DEPRECATED_X(text)

#include "QXmppDataForm.h"
#include "util.h"

class tst_QXmppDataForm : public QObject
{
    Q_OBJECT

private slots:
    void testSimple();
    void testSubmit();
    void testMedia();
    void testMediaSource();
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
    const QByteArray xml = QByteArrayLiteral(
        "<x xmlns=\"jabber:x:data\" type=\"form\">"
            "<field type=\"text-single\">"
                "<media xmlns=\"urn:xmpp:media-element\" width=\"290\" height=\"80\">"
                    "<uri type=\"image/jpeg\">"
                        "http://www.victim.com/challenges/ocr.jpeg?F3A6292C"
                    "</uri>"
                    "<uri type=\"image/png\">"
                        "cid:sha1+f24030b8d91d233bac14777be5ab531ca3b9f102@bob.xmpp.org"
                    "</uri>"
                "</media>"
            "</field>"
        "</x>"
    );

    //
    // test parsing
    //

    QXmppDataForm form;
    parsePacket(form, xml);

    QCOMPARE(form.isNull(), false);
    QCOMPARE(form.fields().size(), 1);
    QCOMPARE(form.fields().at(0).type(), QXmppDataForm::Field::TextSingleField);
    QCOMPARE(form.fields().at(0).isRequired(), false);
    QCOMPARE(form.fields().at(0).mediaSize(), QSize(290, 80));
    QCOMPARE(form.fields().at(0).mediaSources().size(), 2);
    QCOMPARE(
        form.fields().at(0).mediaSources().at(0).uri().toString(),
        QStringLiteral("http://www.victim.com/challenges/ocr.jpeg?F3A6292C")
    );
    QCOMPARE(
        form.fields().at(0).mediaSources().at(0).contentType(),
        QMimeDatabase().mimeTypeForName(QStringLiteral("image/jpeg"))
    );
    QCOMPARE(
        form.fields().at(0).mediaSources().at(1).uri().toString(),
        QStringLiteral("cid:sha1+f24030b8d91d233bac14777be5ab531ca3b9f102@bob.xmpp.org")
    );
    QCOMPARE(
        form.fields().at(0).mediaSources().at(1).contentType(),
        QMimeDatabase().mimeTypeForName(QStringLiteral("image/png"))
    );

    // deprecated
    QCOMPARE(form.fields().at(0).media().isNull(), false);
    QCOMPARE(form.fields().at(0).media().width(), 290);
    QCOMPARE(form.fields().at(0).media().height(), 80);
    QCOMPARE(form.fields().at(0).media().uris().size(), 2);
    QCOMPARE(
        form.fields().at(0).media().uris().at(0).first,
        QStringLiteral("image/jpeg")
    );
    QCOMPARE(
        form.fields().at(0).media().uris().at(0).second,
        QStringLiteral("http://www.victim.com/challenges/ocr.jpeg?F3A6292C")
    );
    QCOMPARE(
        form.fields().at(0).media().uris().at(1).first,
        QStringLiteral("image/png")
    );
    QCOMPARE(
        form.fields().at(0).media().uris().at(1).second,
        QStringLiteral("cid:sha1+f24030b8d91d233bac14777be5ab531ca3b9f102@bob.xmpp.org")
    );

    serializePacket(form, xml);

    //
    // test non-const getters
    //

    QXmppDataForm::Field mediaField1;
    mediaField1.mediaSize().setWidth(290);
    mediaField1.mediaSize().setHeight(80);
    mediaField1.mediaSources() << QXmppDataForm::MediaSource(
        QUrl(QStringLiteral("http://www.victim.com/challenges/ocr.jpeg?F3A6292C")),
        QMimeDatabase().mimeTypeForName(QStringLiteral("image/jpeg"))
    );
    mediaField1.mediaSources() << QXmppDataForm::MediaSource(
        QUrl(QStringLiteral("cid:sha1+f24030b8d91d233bac14777be5ab531ca3b9f102@bob.xmpp.org")),
        QMimeDatabase().mimeTypeForName(QStringLiteral("image/png"))
    );

    QXmppDataForm form2;
    form2.setType(QXmppDataForm::Form);
    form2.setFields(QList<QXmppDataForm::Field>() << mediaField1);
    serializePacket(form2, xml);

    //
    // test setters
    //

    QXmppDataForm::Field mediaField2;
    mediaField2.setMediaSize(QSize(290, 80));
    QVector<QXmppDataForm::MediaSource> sources;
    sources << QXmppDataForm::MediaSource(
        QUrl(QStringLiteral("http://www.victim.com/challenges/ocr.jpeg?F3A6292C")),
        QMimeDatabase().mimeTypeForName(QStringLiteral("image/jpeg"))
    );
    sources << QXmppDataForm::MediaSource(
        QUrl(QStringLiteral("cid:sha1+f24030b8d91d233bac14777be5ab531ca3b9f102@bob.xmpp.org")),
        QMimeDatabase().mimeTypeForName(QStringLiteral("image/png"))
    );
    mediaField2.setMediaSources(sources);

    QXmppDataForm form3;
    form3.setType(QXmppDataForm::Form);
    form3.fields().append(mediaField2);
    serializePacket(form3, xml);

    //
    // test compatibility of deprecated methods
    //

    QXmppDataForm::Field mediaFieldBefore = mediaField1;
    mediaField1.setMedia(mediaField1.media());
    QCOMPARE(mediaField1, mediaFieldBefore);

    QXmppDataForm::Field mediaField2Before = mediaField2;
    mediaField2.setMedia(mediaField2.media());
    QCOMPARE(mediaField2, mediaField2Before);
}

void tst_QXmppDataForm::testMediaSource()
{
    QXmppDataForm::MediaSource source;
    QCOMPARE(source.uri().toString(), QString());
    QCOMPARE(source.contentType(), QMimeType());

    source.setUri(QUrl("https://xmpp.org/index.html"));
    QCOMPARE(source.uri(), QUrl("https://xmpp.org/index.html"));
    source.setContentType(QMimeDatabase().mimeTypeForName("application/xml"));
    QCOMPARE(source.contentType(), QMimeDatabase().mimeTypeForName("application/xml"));
}

QTEST_MAIN(tst_QXmppDataForm)
#include "tst_qxmppdataform.moc"
