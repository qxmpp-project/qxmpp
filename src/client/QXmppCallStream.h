// SPDX-FileCopyrightText: 2019 Niels Ole Salscheider <niels_ole@salscheider-online.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPCALLSTREAM_H
#define QXMPPCALLSTREAM_H

#include <QXmppGlobal.h>

#include <functional>

#include <QObject>

typedef struct _GstPad GstPad;
typedef struct _GstElement GstElement;

class QXmppCallStreamPrivate;
class QXmppIceConnection;
class QXmppCall;
class QXmppCallPrivate;

class QXMPP_EXPORT QXmppCallStream : public QObject
{
    Q_OBJECT

public:
    QString creator() const;
    QString media() const;
    QString name() const;
    int id() const;
    void setReceivePadCallback(std::function<void(GstPad *)> cb);
    void setSendPadCallback(std::function<void(GstPad *)> cb);

private:
    QXmppCallStream(GstElement *pipeline, GstElement *rtpbin,
                    QString media, QString creator, QString name, int id);

    QXmppCallStreamPrivate *d;

    friend class QXmppCall;
    friend class QXmppCallPrivate;
};

#endif
