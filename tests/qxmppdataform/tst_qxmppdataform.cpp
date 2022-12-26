// SPDX-FileCopyrightText: 2012 Andrey Batyiev
// SPDX-FileCopyrightText: 2012 Jeremy Lainé <jeremy.laine@m4x.org>
// SPDX-FileCopyrightText: 2019 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include <QObject>
#include <QtGlobal>

// deprecated methods are also tested: this is used to avoid unnecessary warnings
#undef QT_DEPRECATED_X
#define QT_DEPRECATED_X(text)

#include "QXmppDataForm.h"

#include "util.h"

class tst_QXmppDataForm : public QObject
{
    Q_OBJECT

private:
    Q_SLOT void testSimple();
    Q_SLOT void testSubmit();
    Q_SLOT void testMedia();
    Q_SLOT void testMediaSource();
    Q_SLOT void testFormType();
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
    QVERIFY(form.formType().isNull());
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
        "</x>");

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
        QStringLiteral("http://www.victim.com/challenges/ocr.jpeg?F3A6292C"));
    QCOMPARE(
        form.fields().at(0).mediaSources().at(0).contentType(),
        QMimeDatabase().mimeTypeForName(QStringLiteral("image/jpeg")));
    QCOMPARE(
        form.fields().at(0).mediaSources().at(1).uri().toString(),
        QStringLiteral("cid:sha1+f24030b8d91d233bac14777be5ab531ca3b9f102@bob.xmpp.org"));
    QCOMPARE(
        form.fields().at(0).mediaSources().at(1).contentType(),
        QMimeDatabase().mimeTypeForName(QStringLiteral("image/png")));

    // deprecated
    QCOMPARE(form.fields().at(0).media().isNull(), false);
    QCOMPARE(form.fields().at(0).media().width(), 290);
    QCOMPARE(form.fields().at(0).media().height(), 80);
    QCOMPARE(form.fields().at(0).media().uris().size(), 2);
    QCOMPARE(
        form.fields().at(0).media().uris().at(0).first,
        QStringLiteral("image/jpeg"));
    QCOMPARE(
        form.fields().at(0).media().uris().at(0).second,
        QStringLiteral("http://www.victim.com/challenges/ocr.jpeg?F3A6292C"));
    QCOMPARE(
        form.fields().at(0).media().uris().at(1).first,
        QStringLiteral("image/png"));
    QCOMPARE(
        form.fields().at(0).media().uris().at(1).second,
        QStringLiteral("cid:sha1+f24030b8d91d233bac14777be5ab531ca3b9f102@bob.xmpp.org"));

    serializePacket(form, xml);

    //
    // test non-const getters
    //

    QXmppDataForm::Field mediaField1;
    mediaField1.mediaSize().setWidth(290);
    mediaField1.mediaSize().setHeight(80);
    mediaField1.mediaSources() << QXmppDataForm::MediaSource(
        QUrl(QStringLiteral("http://www.victim.com/challenges/ocr.jpeg?F3A6292C")),
        QMimeDatabase().mimeTypeForName(QStringLiteral("image/jpeg")));
    mediaField1.mediaSources() << QXmppDataForm::MediaSource(
        QUrl(QStringLiteral("cid:sha1+f24030b8d91d233bac14777be5ab531ca3b9f102@bob.xmpp.org")),
        QMimeDatabase().mimeTypeForName(QStringLiteral("image/png")));

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
        QMimeDatabase().mimeTypeForName(QStringLiteral("image/jpeg")));
    sources << QXmppDataForm::MediaSource(
        QUrl(QStringLiteral("cid:sha1+f24030b8d91d233bac14777be5ab531ca3b9f102@bob.xmpp.org")),
        QMimeDatabase().mimeTypeForName(QStringLiteral("image/png")));
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

void tst_QXmppDataForm::testFormType()
{
    const auto xml = QByteArrayLiteral(R"(<x xmlns='jabber:x:data' type='submit'>
    <field var='FORM_TYPE' type='hidden'>
        <value>http://jabber.org/protocol/pubsub#subscribe_options</value>
    </field>
    <field var='pubsub#deliver'><value>1</value></field>
    <field var='pubsub#digest'><value>0</value></field>
    <field var='pubsub#include_body'><value>false</value></field>
    <field var='pubsub#show-values'>
        <value>chat</value>
        <value>online</value>
        <value>away</value>
    </field>
</x>)");

    QXmppDataForm form;
    parsePacket(form, xml);

    QCOMPARE(form.formType(), QStringLiteral("http://jabber.org/protocol/pubsub#subscribe_options"));
}

QTEST_MAIN(tst_QXmppDataForm)
#include "tst_qxmppdataform.moc"
