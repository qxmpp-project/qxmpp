// SPDX-FileCopyrightText: 2010 Manjeet Dahiya <manjeetdahiya@gmail.com>
// SPDX-FileCopyrightText: 2010 Jeremy Lainé <jeremy.laine@m4x.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppIbbIq.h"

#include "QXmppConstants_p.h"
#include "QXmppUtils_p.h"

#include "StringLiterals.h"

#include <QDomElement>
#include <QXmlStreamWriter>

using namespace QXmpp::Private;

///
/// \class QXmppIbbOpenIq
///
/// The QXmppIbbOpenIq represents an IBB open request as defined by \xep{0047,
/// In-Band Bytestreams}.
///
/// \ingroup Stanzas
///

QXmppIbbOpenIq::QXmppIbbOpenIq() : QXmppIq(QXmppIq::Set), m_block_size(1024)
{
}

///
/// Returns the maximum size in bytes of each data chunk (which MUST NOT be
/// greater than 65535).
///
long QXmppIbbOpenIq::blockSize() const
{
    return m_block_size;
}

///
/// Sets the maximum size in bytes of each data chunk (which MUST NOT be greater
/// than 65535).
///
void QXmppIbbOpenIq::setBlockSize(long block_size)
{
    m_block_size = block_size;
}

///
/// Returns the unique session ID for this IBB session (which MUST match the
/// NMTOKEN datatype).
///
QString QXmppIbbOpenIq::sid() const
{
    return m_sid;
}

///
/// Sets the unique session ID for this IBB session (which MUST match the
/// NMTOKEN datatype).
///
void QXmppIbbOpenIq::setSid(const QString &sid)
{
    m_sid = sid;
}

/// \cond
bool QXmppIbbOpenIq::isIbbOpenIq(const QDomElement &element)
{
    return isIqType(element, u"open", ns_ibb);
}

void QXmppIbbOpenIq::parseElementFromChild(const QDomElement &element)
{
    QDomElement openElement = firstChildElement(element, u"open");
    m_sid = openElement.attribute(u"sid"_s);
    m_block_size = openElement.attribute(u"block-size"_s).toLong();
}

void QXmppIbbOpenIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QSL65("open"));
    writer->writeDefaultNamespace(toString65(ns_ibb));
    writer->writeAttribute(QSL65("sid"), m_sid);
    writer->writeAttribute(QSL65("block-size"), QString::number(m_block_size));
    writer->writeEndElement();
}
/// \endcond

///
/// \class QXmppIbbCloseIq
///
/// The QXmppIbbCloseIq represents an IBB close request as defined by \xep{0047,
/// In-Band Bytestreams}.
///
/// \ingroup Stanzas
///

QXmppIbbCloseIq::QXmppIbbCloseIq() : QXmppIq(QXmppIq::Set)
{
}

///
/// Returns the unique session ID for this IBB session (which MUST match the
/// NMTOKEN datatype).
///
QString QXmppIbbCloseIq::sid() const
{
    return m_sid;
}

///
/// Sets the unique session ID for this IBB session (which MUST match the
/// NMTOKEN datatype).
///
void QXmppIbbCloseIq::setSid(const QString &sid)
{
    m_sid = sid;
}

/// \cond
bool QXmppIbbCloseIq::isIbbCloseIq(const QDomElement &element)
{
    return isIqType(element, u"close", ns_ibb);
}

void QXmppIbbCloseIq::parseElementFromChild(const QDomElement &element)
{
    QDomElement openElement = firstChildElement(element, u"close");
    m_sid = openElement.attribute(u"sid"_s);
}

void QXmppIbbCloseIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QSL65("close"));
    writer->writeDefaultNamespace(toString65(ns_ibb));
    writer->writeAttribute(QSL65("sid"), m_sid);
    writer->writeEndElement();
}
/// \endcond

///
/// \class QXmppIbbCloseIq
///
/// The QXmppIbbCloseIq represents an IBB data request as defined by \xep{0047,
/// In-Band Bytestreams}.
///
/// \ingroup Stanzas
///

QXmppIbbDataIq::QXmppIbbDataIq() : QXmppIq(QXmppIq::Set), m_seq(0)
{
}

///
/// Returns the data chunk sequence counter.
///
/// The value starts at 0 (zero) for each sender and MUST be incremented for
/// each packet sent by that entity. The counter loops at maximum, so that after
/// value 65535 the sequence MUST start again at 0.
///
quint16 QXmppIbbDataIq::sequence() const
{
    return m_seq;
}

///
/// Sets the data chunk sequence counter.
///
/// The value starts at 0 (zero) for each sender and MUST be incremented for
/// each packet sent by that entity. The counter loops at maximum, so that after
/// value 65535 the sequence MUST start again at 0.
///
void QXmppIbbDataIq::setSequence(quint16 seq)
{
    m_seq = seq;
}

///
/// Returns the unique session ID for this IBB session (which MUST match the
/// NMTOKEN datatype).
///
QString QXmppIbbDataIq::sid() const
{
    return m_sid;
}

///
/// Sets the unique session ID for this IBB session (which MUST match the
/// NMTOKEN datatype).
///
void QXmppIbbDataIq::setSid(const QString &sid)
{
    m_sid = sid;
}

///
/// Returns the current data chunk
///
QByteArray QXmppIbbDataIq::payload() const
{
    return m_payload;
}

///
/// Sets the current data chunk
///
void QXmppIbbDataIq::setPayload(const QByteArray &data)
{
    m_payload = data;
}

/// \cond
bool QXmppIbbDataIq::isIbbDataIq(const QDomElement &element)
{
    return isIqType(element, u"data", ns_ibb);
}

void QXmppIbbDataIq::parseElementFromChild(const QDomElement &element)
{
    QDomElement dataElement = firstChildElement(element, u"data");
    m_sid = dataElement.attribute(u"sid"_s);
    m_seq = parseInt<uint16_t>(dataElement.attribute(u"seq"_s)).value_or(0);
    m_payload = QByteArray::fromBase64(dataElement.text().toLatin1());
}

void QXmppIbbDataIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QSL65("data"));
    writer->writeDefaultNamespace(toString65(ns_ibb));
    writer->writeAttribute(QSL65("sid"), m_sid);
    writer->writeAttribute(QSL65("seq"), QString::number(m_seq));
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    writer->writeCharacters(m_payload.toBase64());
#else
    writer->writeCharacters(QString::fromUtf8(m_payload.toBase64()));
#endif
    writer->writeEndElement();
}
/// \endcond
