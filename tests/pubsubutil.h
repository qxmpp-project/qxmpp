// SPDX-FileCopyrightText: 2020 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef PUBSUBUTIL_H
#define PUBSUBUTIL_H

#include "QXmppPubSubBaseItem.h"

#include <QDomElement>
#include <QXmlStreamWriter>
#include <QtTest/QTest>

class TestItem : public QXmppPubSubBaseItem
{
public:
    TestItem(const QString &id = {})
        : QXmppPubSubBaseItem(id)
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
        return QXmppPubSubBaseItem::isItem(element, [](const QDomElement &payload) {
            return payload.tagName() == "test-payload";
        });
    }

    bool parseCalled = false;
    mutable bool serializeCalled = false;
    static bool isItemCalled;
};

bool TestItem::isItemCalled = false;

#endif  // PUBSUBUTIL_H
