#include <QDomElement>

#include "QXmppConstants.h"
#include "QXmppLastActivityIq.h"

class QXmppLastActivityIqPrivate : public QSharedData
{
public:
    QXmppLastActivityIqPrivate()
        : seconds(0)
        , status()
    {}

    quint64 seconds;
    QString status;
};

QXmppLastActivityIq::QXmppLastActivityIq(const QString& to)
    : QXmppIq()
    , d(new QXmppLastActivityIqPrivate)
{
    // for self jid should be empty
    setTo(to);
}

QXmppLastActivityIq::QXmppLastActivityIq(const QXmppLastActivityIq& other)
    : QXmppIq(other)
    , d(other.d)
{
}

QXmppLastActivityIq::~QXmppLastActivityIq()
{
}

QXmppLastActivityIq& QXmppLastActivityIq::operator=(const QXmppLastActivityIq& other)
{
    QXmppIq::operator=(other);
    d = other.d;
    return *this;
}

quint64 QXmppLastActivityIq::seconds() const
{
    return d->seconds;
}

void QXmppLastActivityIq::setSeconds(quint64 seconds)
{
    d->seconds = seconds;
}

QString QXmppLastActivityIq::status() const
{
    return d->status;
}

void QXmppLastActivityIq::setStatus(const QString &status)
{
    d->status = status;
}

bool QXmppLastActivityIq::isLastActivityIq(const QDomElement& element)
{
    return element.firstChildElement("query").namespaceURI() == ns_last_activity;
}

void QXmppLastActivityIq::parseElementFromChild(const QDomElement& element)
{
    QDomElement queryElement = element.firstChildElement("query");
    d->seconds = queryElement.attribute("seconds").toInt();
    d->status = queryElement.text();
}

void QXmppLastActivityIq::toXmlElementFromChild(QXmlStreamWriter* writer) const
{
    writer->writeStartElement("query");
    writer->writeAttribute("xmlns", ns_last_activity);
    if (d->seconds)
        writer->writeAttribute("seconds", QString::number(d->seconds));
    if (!d->status.isNull())
        writer->writeCharacters(d->status);
    writer->writeEndElement();
}
