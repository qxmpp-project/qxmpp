// SPDX-FileCopyrightText: 2012 Jeremy Lainé <jeremy.laine@m4x.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppBookmarkSet.h"

#include "QXmppUtils_p.h"

#include <QDomElement>

using namespace QXmpp::Private;

static const char *ns_bookmarks = "storage:bookmarks";

/// Constructs a new conference room bookmark.
///

QXmppBookmarkConference::QXmppBookmarkConference()
    : m_autoJoin(false)
{
}

/// Returns whether the client should automatically join the conference room
/// on login.
///

bool QXmppBookmarkConference::autoJoin() const
{
    return m_autoJoin;
}

/// Sets whether the client should automatically join the conference room
/// on login.
///
/// \param autoJoin

void QXmppBookmarkConference::setAutoJoin(bool autoJoin)
{
    m_autoJoin = autoJoin;
}

/// Returns the JID of the conference room.
///

QString QXmppBookmarkConference::jid() const
{
    return m_jid;
}

/// Sets the JID of the conference room.
///
/// \param jid

void QXmppBookmarkConference::setJid(const QString &jid)
{
    m_jid = jid;
}

/// Returns the friendly name for the bookmark.
///

QString QXmppBookmarkConference::name() const
{
    return m_name;
}

/// Sets the friendly name for the bookmark.
///
/// \param name

void QXmppBookmarkConference::setName(const QString &name)
{
    m_name = name;
}

/// Returns the preferred nickname for the conference room.
///

QString QXmppBookmarkConference::nickName() const
{
    return m_nickName;
}

/// Sets the preferred nickname for the conference room.
///
/// \param nickName

void QXmppBookmarkConference::setNickName(const QString &nickName)
{
    m_nickName = nickName;
}

/// Returns the friendly name for the bookmark.
///

QString QXmppBookmarkUrl::name() const
{
    return m_name;
}

/// Sets the friendly name for the bookmark.
///
/// \param name

void QXmppBookmarkUrl::setName(const QString &name)
{
    m_name = name;
}

/// Returns the URL for the web page.
///

QUrl QXmppBookmarkUrl::url() const
{
    return m_url;
}

/// Sets the URL for the web page.
///
/// \param url

void QXmppBookmarkUrl::setUrl(const QUrl &url)
{
    m_url = url;
}

/// Returns the conference rooms bookmarks in this bookmark set.
///

QList<QXmppBookmarkConference> QXmppBookmarkSet::conferences() const
{
    return m_conferences;
}

/// Sets the conference rooms bookmarks in this bookmark set.
///
/// \param conferences

void QXmppBookmarkSet::setConferences(const QList<QXmppBookmarkConference> &conferences)
{
    m_conferences = conferences;
}

/// Returns the web page bookmarks in this bookmark set.
///

QList<QXmppBookmarkUrl> QXmppBookmarkSet::urls() const
{
    return m_urls;
}

/// Sets the web page bookmarks in this bookmark set.
///
/// \param urls

void QXmppBookmarkSet::setUrls(const QList<QXmppBookmarkUrl> &urls)
{
    m_urls = urls;
}

/// \cond
bool QXmppBookmarkSet::isBookmarkSet(const QDomElement &element)
{
    return element.tagName() == QStringLiteral("storage") &&
        element.namespaceURI() == ns_bookmarks;
}

void QXmppBookmarkSet::parse(const QDomElement &element)
{
    QDomElement childElement = element.firstChildElement();
    while (!childElement.isNull()) {
        if (childElement.tagName() == QStringLiteral("conference")) {
            QXmppBookmarkConference conference;
            conference.setAutoJoin(childElement.attribute(QStringLiteral("autojoin")) == QStringLiteral("true") || childElement.attribute("autojoin") == "1");
            conference.setJid(childElement.attribute(QStringLiteral("jid")));
            conference.setName(childElement.attribute(QStringLiteral("name")));
            conference.setNickName(childElement.firstChildElement(QStringLiteral("nick")).text());
            m_conferences << conference;
        } else if (childElement.tagName() == QStringLiteral("url")) {
            QXmppBookmarkUrl url;
            url.setName(childElement.attribute(QStringLiteral("name")));
            url.setUrl(QUrl(childElement.attribute(QStringLiteral("url"))));
            m_urls << url;
        }
        childElement = childElement.nextSiblingElement();
    }
}

void QXmppBookmarkSet::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QStringLiteral("storage"));
    writer->writeDefaultNamespace(ns_bookmarks);
    for (const auto &conference : m_conferences) {
        writer->writeStartElement(QStringLiteral("conference"));
        if (conference.autoJoin()) {
            writeOptionalXmlAttribute(writer, QStringLiteral("autojoin"), QStringLiteral("true"));
        }
        writeOptionalXmlAttribute(writer, QStringLiteral("jid"), conference.jid());
        writeOptionalXmlAttribute(writer, QStringLiteral("name"), conference.name());
        if (!conference.nickName().isEmpty()) {
            writeXmlTextElement(writer, QStringLiteral("nick"), conference.nickName());
        }
        writer->writeEndElement();
    }
    for (const auto &url : m_urls) {
        writer->writeStartElement(QStringLiteral("url"));
        writeOptionalXmlAttribute(writer, QStringLiteral("name"), url.name());
        writeOptionalXmlAttribute(writer, QStringLiteral("url"), url.url().toString());
        writer->writeEndElement();
    }
    writer->writeEndElement();
}
/// \endcond
