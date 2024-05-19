// SPDX-FileCopyrightText: 2012 Jeremy Lainé <jeremy.laine@m4x.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppBookmarkSet.h"

#include "QXmppConstants_p.h"
#include "QXmppUtils_p.h"

#include "StringLiterals.h"

#include <QDomElement>

using namespace QXmpp::Private;

/// Constructs a new conference room bookmark.
QXmppBookmarkConference::QXmppBookmarkConference()
    : m_autoJoin(false)
{
}

/// Returns whether the client should automatically join the conference room
/// on login.
bool QXmppBookmarkConference::autoJoin() const
{
    return m_autoJoin;
}

/// Sets whether the client should automatically join the conference room
/// on login.
void QXmppBookmarkConference::setAutoJoin(bool autoJoin)
{
    m_autoJoin = autoJoin;
}

/// Returns the JID of the conference room.
QString QXmppBookmarkConference::jid() const
{
    return m_jid;
}

/// Sets the JID of the conference room.
void QXmppBookmarkConference::setJid(const QString &jid)
{
    m_jid = jid;
}

/// Returns the friendly name for the bookmark.
QString QXmppBookmarkConference::name() const
{
    return m_name;
}

/// Sets the friendly name for the bookmark.
void QXmppBookmarkConference::setName(const QString &name)
{
    m_name = name;
}

/// Returns the preferred nickname for the conference room.
QString QXmppBookmarkConference::nickName() const
{
    return m_nickName;
}

/// Sets the preferred nickname for the conference room.
void QXmppBookmarkConference::setNickName(const QString &nickName)
{
    m_nickName = nickName;
}

/// Returns the friendly name for the bookmark.
QString QXmppBookmarkUrl::name() const
{
    return m_name;
}

/// Sets the friendly name for the bookmark.
void QXmppBookmarkUrl::setName(const QString &name)
{
    m_name = name;
}

/// Returns the URL for the web page.
QUrl QXmppBookmarkUrl::url() const
{
    return m_url;
}

/// Sets the URL for the web page.
void QXmppBookmarkUrl::setUrl(const QUrl &url)
{
    m_url = url;
}

/// Returns the conference rooms bookmarks in this bookmark set.
QList<QXmppBookmarkConference> QXmppBookmarkSet::conferences() const
{
    return m_conferences;
}

/// Sets the conference rooms bookmarks in this bookmark set.
void QXmppBookmarkSet::setConferences(const QList<QXmppBookmarkConference> &conferences)
{
    m_conferences = conferences;
}

/// Returns the web page bookmarks in this bookmark set.
QList<QXmppBookmarkUrl> QXmppBookmarkSet::urls() const
{
    return m_urls;
}

/// Sets the web page bookmarks in this bookmark set.
void QXmppBookmarkSet::setUrls(const QList<QXmppBookmarkUrl> &urls)
{
    m_urls = urls;
}

/// \cond
bool QXmppBookmarkSet::isBookmarkSet(const QDomElement &element)
{
    return element.tagName() == u"storage" &&
        element.namespaceURI() == ns_bookmarks;
}

void QXmppBookmarkSet::parse(const QDomElement &element)
{
    for (const auto &childElement : iterChildElements(element, u"conference")) {
        QXmppBookmarkConference conference;
        auto autojoinAttribute = childElement.attribute(u"autojoin"_s);
        conference.setAutoJoin(autojoinAttribute == u"true" || autojoinAttribute == u"1");
        conference.setJid(childElement.attribute(u"jid"_s));
        conference.setName(childElement.attribute(u"name"_s));
        conference.setNickName(firstChildElement(childElement, u"nick").text());
        m_conferences << conference;
    }

    for (const auto &childElement : iterChildElements(element, u"url")) {
        QXmppBookmarkUrl url;
        url.setName(childElement.attribute(u"name"_s));
        url.setUrl(QUrl(childElement.attribute(u"url"_s)));
        m_urls << url;
    }
}

void QXmppBookmarkSet::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QSL65("storage"));
    writer->writeDefaultNamespace(toString65(ns_bookmarks));
    for (const auto &conference : m_conferences) {
        writer->writeStartElement(QSL65("conference"));
        if (conference.autoJoin()) {
            writeOptionalXmlAttribute(writer, u"autojoin", u"true"_s);
        }
        writeOptionalXmlAttribute(writer, u"jid", conference.jid());
        writeOptionalXmlAttribute(writer, u"name", conference.name());
        if (!conference.nickName().isEmpty()) {
            writeXmlTextElement(writer, u"nick", conference.nickName());
        }
        writer->writeEndElement();
    }
    for (const auto &url : m_urls) {
        writer->writeStartElement(QSL65("url"));
        writeOptionalXmlAttribute(writer, u"name", url.name());
        writeOptionalXmlAttribute(writer, u"url", url.url().toString());
        writer->writeEndElement();
    }
    writer->writeEndElement();
}
/// \endcond
