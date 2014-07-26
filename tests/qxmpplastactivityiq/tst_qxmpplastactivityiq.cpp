#include <QObject>
#include "QXmppLastActivityIq.h"
#include "util.h"

class tst_QXmppLastActivityIq : public QObject
{
    Q_OBJECT

private slots:
    void testLastActivityGet();
    void testLastActivityResult();
};

void tst_QXmppLastActivityIq::testLastActivityGet()
{
    const QByteArray xmlGet(
    "<iq id=\"last_activity_1\" to=\"juliet@capulet.com/balcony\" "
    "from=\"romeo@montague.net/orchard\" type=\"get\">"
    "<query xmlns=\"jabber:iq:last\"/></iq>");

    QXmppLastActivityIq lastActivityIqGet;
    parsePacket(lastActivityIqGet, xmlGet);
    QCOMPARE(lastActivityIqGet.id(), QLatin1String("last_activity_1"));
    QCOMPARE(lastActivityIqGet.to(), QLatin1String("juliet@capulet.com/balcony"));
    QCOMPARE(lastActivityIqGet.from(), QLatin1String("romeo@montague.net/orchard"));
    QCOMPARE(lastActivityIqGet.type(), QXmppIq::Get);
    serializePacket(lastActivityIqGet, xmlGet);
}

void tst_QXmppLastActivityIq::testLastActivityResult()
{
    const QByteArray xmlResult(
    "<iq id=\"last_activity_1\" to=\"romeo@montague.net/orchard\" "
    "from=\"juliet@capulet.com/balcony\" type=\"result\">"
    "<query xmlns=\"jabber:iq:last\" seconds=\"666\">custom status</query></iq>");

    QXmppLastActivityIq lastActivityIqResult;
    parsePacket(lastActivityIqResult, xmlResult);
    QCOMPARE(lastActivityIqResult.id(), QLatin1String("last_activity_1"));
    QCOMPARE(lastActivityIqResult.to(), QLatin1String("romeo@montague.net/orchard"));
    QCOMPARE(lastActivityIqResult.from(), QLatin1String("juliet@capulet.com/balcony"));
    QCOMPARE(lastActivityIqResult.type(), QXmppIq::Result);
    QCOMPARE(lastActivityIqResult.status(), QString("custom status"));
    QCOMPARE(lastActivityIqResult.seconds(), 666);
    serializePacket(lastActivityIqResult, xmlResult);
}

QTEST_MAIN(tst_QXmppLastActivityIq)
#include "tst_qxmpplastactivityiq.moc"
