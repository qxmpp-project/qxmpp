// SPDX-FileCopyrightText: 2025 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include <QXmppCallManager.h>
#include <QXmppClient.h>
#include <QXmppRosterManager.h>

#include <chrono>
#include <gst/gst.h>

#include <QCoreApplication>
#include <QTimer>
#ifdef Q_OS_UNIX
#include <csignal>
#endif

using namespace std::chrono_literals;

#ifdef Q_OS_UNIX
void handleSignal(int signal)
{
    if (signal == SIGINT) {
        // print newline
        qDebug() << "";
        if (QCoreApplication::instance()) {
            QCoreApplication::instance()->quit();
        }
    }
}
#endif

void setupCallStream(QXmppCall *call)
{
    auto *gstPipeline = call->pipeline();
    auto *stream = call->audioStream();

    qDebug() << "[Call] Setup call stream" << stream->media();
    if (stream->media() == u"audio") {
        // output receiving audio
        stream->setReceivePadCallback([gstPipeline](GstPad *receivePad) {
            GstElement *output = gst_parse_bin_from_description("audioresample ! audioconvert ! autoaudiosink", true, nullptr);
            if (!gst_bin_add(GST_BIN(gstPipeline), output)) {
                qFatal("Failed to add input to pipeline");
                return;
            }

            gst_pad_link(receivePad, gst_element_get_static_pad(output, "sink"));
            gst_element_sync_state_with_parent(output);

            qDebug() << "[Call] receive pad set";
        });

        // record and send microphone
        stream->setSendPadCallback([gstPipeline](GstPad *sendPad) {
            GstElement *output = gst_parse_bin_from_description("autoaudiosrc ! audioconvert ! audioresample ! queue max-size-time=1000000", true, nullptr);
            if (!gst_bin_add(GST_BIN(gstPipeline), output)) {
                qFatal("Failed to add input to pipeline");
                return;
            }

            gst_pad_link(gst_element_get_static_pad(output, "src"), sendPad);
            gst_element_sync_state_with_parent(output);

            qDebug() << "[Call] send pad set";
        });
    }
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

#ifdef Q_OS_UNIX
    // set signal handler for SIGINT (CTRL+C)
    std::signal(SIGINT, handleSignal);
#endif

    QXmppClient client;
    auto *rosterManager = client.findExtension<QXmppRosterManager>();
    auto *callManager = client.addNewExtension<QXmppCallManager>();
    client.logger()->setLoggingType(QXmppLogger::StdoutLogging);
    client.logger()->setMessageTypes(QXmppLogger::MessageType::AnyMessage);

    // client config
    QXmppConfiguration config;
    config.setJid(qEnvironmentVariable("QXMPP_JID"));
    config.setPassword(qEnvironmentVariable("QXMPP_PASSWORD"));
    config.setResourcePrefix("Call");

    // call manager config
    callManager->setStunServer(QHostAddress(QStringLiteral("stun.nextcloud.com")), 443);
    // callManager->setTurnServer();
    // callManager->setTurnUser(client.configuration().jid());
    // callManager->setTurnUser(client.configuration().password());

    client.connectToServer(config);

    auto setupCall = [&app, callManager](QXmppCall *call) {
        if (call->audioStream()) {
            setupCallStream(call);
        }

        QObject::connect(call, &QXmppCall::streamCreated, call, [call](QXmppCallStream *stream) {
            setupCallStream(call);
        });

        QObject::connect(call, &QXmppCall::connected, &app, [=]() {
            qDebug() << "[Call] Call to" << call->jid() << "connected!";
        });
        QObject::connect(call, &QXmppCall::ringing, [=]() {
            qDebug() << "[Call] Ringing" << call->jid() << "...";
        });
        QObject::connect(call, &QXmppCall::finished, [=]() {
            qDebug() << "[Call] Call with" << call->jid() << "ended. (Deleting)";
            call->deleteLater();
        });
    };

    // on connect
    QObject::connect(&client, &QXmppClient::connected, &app, [=, &config] {
        // wait 1 second for presence of other clients to arrive
        QTimer::singleShot(1s, [=, &config] {
            // other resources of our account
            auto otherResources = rosterManager->getResources(config.jidBare());
            otherResources.removeOne(config.resource());
            if (otherResources.isEmpty()) {
                qDebug() << "[Call] No other clients to call on this account.";
                return;
            }

            // call first JID
            auto *call = callManager->call(config.jidBare() + u'/' + otherResources.first());
            Q_ASSERT(call != nullptr);

            setupCall(call);
        });
    });

    // on incoming call
    QObject::connect(callManager, &QXmppCallManager::callReceived, &app, [=](QXmppCall *call) {
        qDebug() << "[Call] Received incoming call from" << call->jid() << "-" << "Accepting.";
        call->accept();

        setupCall(call);
    });

    // disconnect from server to avoid having multiple open dead sessions when testing
    QObject::connect(&app, &QCoreApplication::aboutToQuit, &app, [&client]() {
        qDebug() << "Closing connection...";
        client.disconnectFromServer();
    });

    return app.exec();
}
