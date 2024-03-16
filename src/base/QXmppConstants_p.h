// SPDX-FileCopyrightText: 2016 Manjeet Dahiya <manjeetdahiya@gmail.com>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPCONSTANTS_H
#define QXMPPCONSTANTS_H

#include <QStringView>

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QXmpp API.  It exists for the convenience
// of QXmpp's own classes.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

namespace QXmpp::Private {

constexpr int XMPP_DEFAULT_PORT = 5222;

}

inline constexpr QStringView ns_stream = u"http://etherx.jabber.org/streams";
inline constexpr QStringView ns_client = u"jabber:client";
inline constexpr QStringView ns_server = u"jabber:server";
inline constexpr QStringView ns_roster = u"jabber:iq:roster";
inline constexpr QStringView ns_tls = u"urn:ietf:params:xml:ns:xmpp-tls";
inline constexpr QStringView ns_sasl = u"urn:ietf:params:xml:ns:xmpp-sasl";
inline constexpr QStringView ns_bind = u"urn:ietf:params:xml:ns:xmpp-bind";
inline constexpr QStringView ns_stream_error = u"urn:ietf:params:xml:ns:xmpp-streams";
inline constexpr QStringView ns_session = u"urn:ietf:params:xml:ns:xmpp-session";
inline constexpr QStringView ns_stanza = u"urn:ietf:params:xml:ns:xmpp-stanzas";
inline constexpr QStringView ns_pre_approval = u"urn:xmpp:features:pre-approval";
inline constexpr QStringView ns_rosterver = u"urn:xmpp:features:rosterver";
// XEP-0009: Jabber-RPC
inline constexpr QStringView ns_rpc = u"jabber:iq:rpc";
// XEP-0020: Feature Negotiation
inline constexpr QStringView ns_feature_negotiation = u"http://jabber.org/protocol/feature-neg";
// XEP-0027: Current Jabber OpenPGP Usage
inline constexpr QStringView ns_legacy_openpgp = u"jabber:x:encrypted";
// XEP-0030: Service Discovery
inline constexpr QStringView ns_disco_info = u"http://jabber.org/protocol/disco#info";
inline constexpr QStringView ns_disco_items = u"http://jabber.org/protocol/disco#items";
// XEP-0033: Extended Stanza Addressing
inline constexpr QStringView ns_extended_addressing = u"http://jabber.org/protocol/address";
// XEP-0045: Multi-User Chat
inline constexpr QStringView ns_muc = u"http://jabber.org/protocol/muc";
inline constexpr QStringView ns_muc_admin = u"http://jabber.org/protocol/muc#admin";
inline constexpr QStringView ns_muc_owner = u"http://jabber.org/protocol/muc#owner";
inline constexpr QStringView ns_muc_user = u"http://jabber.org/protocol/muc#user";
// XEP-0047: In-Band Bytestreams
inline constexpr QStringView ns_ibb = u"http://jabber.org/protocol/ibb";
// XEP-0048: Bookmarks
inline constexpr QStringView ns_bookmarks = u"storage:bookmarks";
// XEP-0049: Private XML Storage
inline constexpr QStringView ns_private = u"jabber:iq:private";
// XEP-0054: vcard-temp
inline constexpr QStringView ns_vcard = u"vcard-temp";
// XEP-0059: Result Set Management
inline constexpr QStringView ns_rsm = u"http://jabber.org/protocol/rsm";
// XEP-0060: Publish-Subscribe
inline constexpr QStringView ns_pubsub = u"http://jabber.org/protocol/pubsub";
inline constexpr QStringView ns_pubsub_auto_create = u"http://jabber.org/protocol/pubsub#auto-create";
inline constexpr QStringView ns_pubsub_config_node = u"http://jabber.org/protocol/pubsub#config-node";
inline constexpr QStringView ns_pubsub_config_node_max = u"http://jabber.org/protocol/pubsub#config-node-max";
inline constexpr QStringView ns_pubsub_create_and_configure = u"http://jabber.org/protocol/pubsub#create-and-configure";
inline constexpr QStringView ns_pubsub_create_nodes = u"http://jabber.org/protocol/pubsub#create-nodes";
inline constexpr QStringView ns_pubsub_errors = u"http://jabber.org/protocol/pubsub#errors";
inline constexpr QStringView ns_pubsub_event = u"http://jabber.org/protocol/pubsub#event";
inline constexpr QStringView ns_pubsub_multi_items = u"http://jabber.org/protocol/pubsub#multi-items";
inline constexpr QStringView ns_pubsub_node_config = u"http://jabber.org/protocol/pubsub#node_config";
inline constexpr QStringView ns_pubsub_owner = u"http://jabber.org/protocol/pubsub#owner";
inline constexpr QStringView ns_pubsub_publish = u"http://jabber.org/protocol/pubsub#publish";
inline constexpr QStringView ns_pubsub_publish_options = u"http://jabber.org/protocol/pubsub#publish-options";
inline constexpr QStringView ns_pubsub_rsm = u"http://jabber.org/protocol/pubsub#rsm";
// XEP-0065: SOCKS5 Bytestreams
inline constexpr QStringView ns_bytestreams = u"http://jabber.org/protocol/bytestreams";
// XEP-0066: Out of Band Data
inline constexpr QStringView ns_oob = u"jabber:x:oob";
// XEP-0071: XHTML-IM
inline constexpr QStringView ns_xhtml = u"http://www.w3.org/1999/xhtml";
inline constexpr QStringView ns_xhtml_im = u"http://jabber.org/protocol/xhtml-im";
// XEP-0077: In-Band Registration
inline constexpr QStringView ns_register = u"jabber:iq:register";
inline constexpr QStringView ns_register_feature = u"http://jabber.org/features/iq-register";
// XEP-0078: Non-SASL Authentication
inline constexpr QStringView ns_auth = u"jabber:iq:auth";
inline constexpr QStringView ns_authFeature = u"http://jabber.org/features/iq-auth";
// XEP-0080: User Location
inline constexpr QStringView ns_geoloc = u"http://jabber.org/protocol/geoloc";
inline constexpr QStringView ns_geoloc_notify = u"http://jabber.org/protocol/geoloc+notify";
// XEP-0085: Chat State Notifications
inline constexpr QStringView ns_chat_states = u"http://jabber.org/protocol/chatstates";
// XEP-0091: Legacy Delayed Delivery
inline constexpr QStringView ns_legacy_delayed_delivery = u"jabber:x:delay";
// XEP-0092: Software Version
inline constexpr QStringView ns_version = u"jabber:iq:version";
inline constexpr QStringView ns_data = u"jabber:x:data";
// XEP-0095: Stream Initiation
inline constexpr QStringView ns_stream_initiation = u"http://jabber.org/protocol/si";
inline constexpr QStringView ns_stream_initiation_file_transfer = u"http://jabber.org/protocol/si/profile/file-transfer";
// XEP-0103: URL Address Information
inline constexpr QStringView ns_url_data = u"http://jabber.org/protocol/url-data";
// XEP-0108: User Activity
inline constexpr QStringView ns_activity = u"http://jabber.org/protocol/activity";
// XEP-0115: Entity Capabilities
inline constexpr QStringView ns_capabilities = u"http://jabber.org/protocol/caps";
// XEP-0118: User Tune
inline constexpr QStringView ns_tune = u"http://jabber.org/protocol/tune";
inline constexpr QStringView ns_tune_notify = u"http://jabber.org/protocol/tune+notify";
// XEP-0136: Message Archiving
inline constexpr QStringView ns_archive = u"urn:xmpp:archive";
// XEP-0138: Stream Compression
inline constexpr QStringView ns_compress = u"http://jabber.org/protocol/compress";
inline constexpr QStringView ns_compressFeature = u"http://jabber.org/features/compress";
// XEP-0145: Annotations
inline constexpr QStringView ns_rosternotes = u"storage:rosternotes";
// XEP-0153: vCard-Based Avatars
inline constexpr QStringView ns_vcard_update = u"vcard-temp:x:update";
// XEP-0158: CAPTCHA Forms
inline constexpr QStringView ns_captcha = u"urn:xmpp:captcha";
// XEP-0166: Jingle
inline constexpr QStringView ns_jingle = u"urn:xmpp:jingle:1";
inline constexpr QStringView ns_jingle_raw_udp = u"urn:xmpp:jingle:transports:raw-udp:1";
inline constexpr QStringView ns_jingle_ice_udp = u"urn:xmpp:jingle:transports:ice-udp:1";
// XEP-0167: Jingle RTP Sessions
inline constexpr QStringView ns_jingle_rtp = u"urn:xmpp:jingle:apps:rtp:1";
inline constexpr QStringView ns_jingle_rtp_audio = u"urn:xmpp:jingle:apps:rtp:audio";
inline constexpr QStringView ns_jingle_rtp_video = u"urn:xmpp:jingle:apps:rtp:video";
inline constexpr QStringView ns_jingle_rtp_info = u"urn:xmpp:jingle:apps:rtp:info:1";
inline constexpr QStringView ns_jingle_rtp_errors = u"urn:xmpp:jingle:apps:rtp:errors:1";
// XEP-0184: Message Receipts
inline constexpr QStringView ns_message_receipts = u"urn:xmpp:receipts";
// XEP-0198: Stream Management
inline constexpr QStringView ns_stream_management = u"urn:xmpp:sm:3";
// XEP-0199: XMPP Ping
inline constexpr QStringView ns_ping = u"urn:xmpp:ping";
// XEP-0202: Entity Time
inline constexpr QStringView ns_entity_time = u"urn:xmpp:time";
// XEP-0203: Delayed Delivery
inline constexpr QStringView ns_delayed_delivery = u"urn:xmpp:delay";
// XEP-0215: External Service Discovery
inline constexpr QStringView ns_external_service_discovery = u"urn:xmpp:extdisco:2";
// XEP-0220: Server Dialback
inline constexpr QStringView ns_server_dialback = u"jabber:server:dialback";
// XEP-0221: Data Forms Media Element
inline constexpr QStringView ns_media_element = u"urn:xmpp:media-element";
// XEP-0224: Attention
inline constexpr QStringView ns_attention = u"urn:xmpp:attention:0";
// XEP-0231: Bits of Binary
inline constexpr QStringView ns_bob = u"urn:xmpp:bob";
// XEP-0249: Direct MUC Invitations
inline constexpr QStringView ns_conference = u"jabber:x:conference";
// XEP-0264: Jingle Content Thumbnails
inline constexpr QStringView ns_thumbs = u"urn:xmpp:thumbs:1";
// XEP-0272: Multiparty Jingle (Muji)
inline constexpr QStringView ns_muji = u"urn:xmpp:jingle:muji:0";
// XEP-0280: Message Carbons
inline constexpr QStringView ns_carbons = u"urn:xmpp:carbons:2";
// XEP-0293: Jingle RTP Feedback Negotiation
inline constexpr QStringView ns_jingle_rtp_feedback_negotiation = u"urn:xmpp:jingle:apps:rtp:rtcp-fb:0";
// XEP-0294: Jingle RTP Header Extensions Negotiation
inline constexpr QStringView ns_jingle_rtp_header_extensions_negotiation = u"urn:xmpp:jingle:apps:rtp:rtp-hdrext:0";
// XEP-0297: Stanza Forwarding
inline constexpr QStringView ns_forwarding = u"urn:xmpp:forward:0";
// XEP-0300: Use of Cryptographic Hash Functions in XMPP
inline constexpr QStringView ns_hashes = u"urn:xmpp:hashes:2";
// XEP-0308: Last Message Correction
inline constexpr QStringView ns_message_correct = u"urn:xmpp:message-correct:0";
// XEP-0313: Message Archive Management
inline constexpr QStringView ns_mam = u"urn:xmpp:mam:2";
// XEP-0319: Last User Interaction in Presence
inline constexpr QStringView ns_idle = u"urn:xmpp:idle:1";
// XEP-0320: Use of DTLS-SRTP in Jingle Sessions
inline constexpr QStringView ns_jingle_dtls = u"urn:xmpp:jingle:apps:dtls:0";
// XEP-0333: Chat Markers
inline constexpr QStringView ns_chat_markers = u"urn:xmpp:chat-markers:0";
// XEP-0334: Message Processing Hints
inline constexpr QStringView ns_message_processing_hints = u"urn:xmpp:hints";
// XEP-0352: Client State Indication
inline constexpr QStringView ns_csi = u"urn:xmpp:csi:0";
// XEP-0353: Jingle Message Initiation
inline constexpr QStringView ns_jingle_message_initiation = u"urn:xmpp:jingle-message:0";
// XEP-0357: Push Notifications
inline constexpr QStringView ns_push = u"urn:xmpp:push:0";
// XEP-0359: Unique and Stable Stanza IDs
inline constexpr QStringView ns_sid = u"urn:xmpp:sid:0";
// XEP-0363: HTTP File Upload
inline constexpr QStringView ns_http_upload = u"urn:xmpp:http:upload:0";
// XEP-0364: Current Off-the-Record Messaging Usage
inline constexpr QStringView ns_otr = u"urn:xmpp:otr:0";
// XEP-0367: Message Attaching
inline constexpr QStringView ns_message_attaching = u"urn:xmpp:message-attaching:1";
// XEP-0369: Mediated Information eXchange (MIX)
inline constexpr QStringView ns_mix = u"urn:xmpp:mix:core:1";
inline constexpr QStringView ns_mix_create_channel = u"urn:xmpp:mix:core:1#create-channel";
inline constexpr QStringView ns_mix_searchable = u"urn:xmpp:mix:core:1#searchable";
inline constexpr QStringView ns_mix_node_messages = u"urn:xmpp:mix:nodes:messages";
inline constexpr QStringView ns_mix_node_participants = u"urn:xmpp:mix:nodes:participants";
inline constexpr QStringView ns_mix_node_presence = u"urn:xmpp:mix:nodes:presence";
inline constexpr QStringView ns_mix_node_config = u"urn:xmpp:mix:nodes:config";
inline constexpr QStringView ns_mix_node_info = u"urn:xmpp:mix:nodes:info";
// XEP-0373: OpenPGP for XMPP
inline constexpr QStringView ns_ox = u"urn:xmpp:openpgp:0";
// XEP-0380: Explicit Message Encryption
inline constexpr QStringView ns_eme = u"urn:xmpp:eme:0";
// XEP-0382: Spoiler messages
inline constexpr QStringView ns_spoiler = u"urn:xmpp:spoiler:0";
// XEP-0384: OMEMO Encryption
inline constexpr QStringView ns_omemo = u"eu.siacs.conversations.axolotl";
inline constexpr QStringView ns_omemo_1 = u"urn:xmpp:omemo:1";
inline constexpr QStringView ns_omemo_2 = u"urn:xmpp:omemo:2";
inline constexpr QStringView ns_omemo_2_bundles = u"urn:xmpp:omemo:2:bundles";
inline constexpr QStringView ns_omemo_2_devices = u"urn:xmpp:omemo:2:devices";
// XEP-0405: Mediated Information eXchange (MIX): Participant Server Requirements
inline constexpr QStringView ns_mix_pam = u"urn:xmpp:mix:pam:1";
inline constexpr QStringView ns_mix_roster = u"urn:xmpp:mix:roster:0";
inline constexpr QStringView ns_mix_presence = u"urn:xmpp:presence:0";
// XEP-0407: Mediated Information eXchange (MIX): Miscellaneous Capabilities
inline constexpr QStringView ns_mix_misc = u"urn:xmpp:mix:misc:0";
// XEP-0428: Fallback Indication
inline constexpr QStringView ns_fallback_indication = u"urn:xmpp:fallback:0";
// XEP-0434: Trust Messages (TM)
inline constexpr QStringView ns_tm = u"urn:xmpp:tm:1";
// XEP-0444: Message Reactions
inline constexpr QStringView ns_reactions = u"urn:xmpp:reactions:0";
// XEP-0446: File metadata element
inline constexpr QStringView ns_file_metadata = u"urn:xmpp:file:metadata:0";
// XEP-0447: Stateless file sharing
inline constexpr QStringView ns_sfs = u"urn:xmpp:sfs:0";
// XEP-0448: Encryption for stateless file sharing
inline constexpr QStringView ns_esfs = u"urn:xmpp:esfs:0";
// XEP-0450: Automatic Trust Management (ATM)
inline constexpr QStringView ns_atm = u"urn:xmpp:atm:1";
// XEP-0482: Call Invites
inline constexpr QStringView ns_call_invites = u"urn:xmpp:call-invites:0";

#endif  // QXMPPCONSTANTS_H
