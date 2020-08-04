/*
 * Copyright (C) 2008-2021 The QXmpp developers
 *
 * Author:
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

#ifndef PUBSUBUTIL_H
#define PUBSUBUTIL_H

#include "QXmppPubSubItem.h"

#include <QDomElement>
#include <QXmlStreamWriter>
#include <QtTest/QTest>

class TestItem : public QXmppPubSubItem
{
public:
    TestItem(const QString &id = {})
        : QXmppPubSubItem(id)
    {
    }

    void parsePayload(const QDomElement &payloadElement) override
    {
        parseCalled = true;
        QCOMPARE(payloadElement.tagName(), QStringLiteral("test-payload"));
    }

    void serializePayload(QXmlStreamWriter *writer) const override
    {
        serializeCalled = true;
        writer->writeEmptyElement("test-payload");
    }

    static bool isItem(const QDomElement &element)
    {
        isItemCalled = true;
        return QXmppPubSubItem::isItem(element, [](const QDomElement &payload) {
            return payload.tagName() == "test-payload";
        });
    }

    bool parseCalled = false;
    mutable bool serializeCalled = false;
    static bool isItemCalled;
};

bool TestItem::isItemCalled = false;

#endif  // PUBSUBUTIL_H
