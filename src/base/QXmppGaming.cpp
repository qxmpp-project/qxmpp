#include "QXmppGaming.h"

#include "QXmppConstants.h"
#include "QXmppUtils.h"

#include <QBuffer>
#include <QDomElement>

QXmppGaming::QXmppGaming()
{
}

void QXmppGaming::parse(const QDomElement &element)
{
    QDomElement gameElement = (element.tagName() == "game") ? element : element.firstChildElement("game");
    if (gameElement.namespaceURI() == ns_user_gaming) {
        this->m_character_name = gameElement.firstChildElement("character_name").text();
        this->m_character_profile = gameElement.firstChildElement("character_profile").text();
        this->m_name = gameElement.firstChildElement("name").text();
        this->m_level = gameElement.firstChildElement("level").text();
        this->m_server_address = gameElement.firstChildElement("server_address").text();
        this->m_server_name = gameElement.firstChildElement("server_name").text();
        this->m_uri = gameElement.firstChildElement("uri").text();
    }
}


void QXmppGaming::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("game");
    writer->writeAttribute("xmlns", ns_user_gaming);

    helperToXmlAddTextElement(writer, "character_name", m_character_name);
    helperToXmlAddTextElement(writer, "character_profile", m_character_profile);
    helperToXmlAddTextElement(writer, "name", m_name);
    helperToXmlAddTextElement(writer, "level", m_level);
    helperToXmlAddTextElement(writer, "server_address", m_server_address);
    helperToXmlAddTextElement(writer, "server_name", m_server_name);
    helperToXmlAddTextElement(writer, "uri", m_uri);

    writer->writeEndElement();
}

QXmppElement QXmppGaming::toQXmppElement() const
{
    QXmppElement gamingElement;
    gamingElement.setTagName("game");
    gamingElement.setAttribute("xmlns", ns_user_gaming);

    this->writeToElement(gamingElement, "character_name", m_character_name);
    this->writeToElement(gamingElement, "character_profile", m_character_profile);
    this->writeToElement(gamingElement, "name", m_name);
    this->writeToElement(gamingElement, "level", m_level);
    this->writeToElement(gamingElement, "server_address", m_server_address);
    this->writeToElement(gamingElement, "server_name", m_server_name);
    this->writeToElement(gamingElement, "uri", m_uri);

    return gamingElement;
}

QString QXmppGaming::characterName() const
{
    return m_character_name;
}

void QXmppGaming::setCharacterName(const QString &character_name)
{
    m_character_name = character_name;
}

QString QXmppGaming::characterProfile() const
{
    return m_character_profile;
}

void QXmppGaming::setCharacterProfile(const QString &character_profile)
{
    m_character_profile = character_profile;
}

QString QXmppGaming::name() const
{
    return m_name;
}

void QXmppGaming::setName(const QString &name)
{
    m_name = name;
}

QString QXmppGaming::level() const
{
    return m_level;
}

void QXmppGaming::setLevel(const QString &level)
{
    m_level = level;
}
QString QXmppGaming::serverAddress() const
{
    return m_server_address;
}

void QXmppGaming::setServerAddress(const QString &server_address)
{
    m_server_address = server_address;
}

QString QXmppGaming::serverName() const
{
    return m_server_name;
}

void QXmppGaming::setServerName(const QString &server_name)
{
    m_server_name = server_name;
}

QString QXmppGaming::uri() const
{
    return m_uri;
}

void QXmppGaming::setUri(const QString &uri)
{
    m_uri = uri;
}

void QXmppGaming::writeToElement(QXmppElement &element, const QString &name, const QString &value) const
{
    QXmppElement item;
    item.setTagName(name);;
    item.setValue(value);
    element.appendChild(item);
}
