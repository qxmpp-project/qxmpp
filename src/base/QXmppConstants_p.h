// SPDX-FileCopyrightText: 2016 Manjeet Dahiya <manjeetdahiya@gmail.com>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPCONSTANTS_H
#define QXMPPCONSTANTS_H

#include <QXmppGlobal.h>

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

QXMPP_EXPORT extern const char *ns_stream;
QXMPP_EXPORT extern const char *ns_client;
QXMPP_EXPORT extern const char *ns_server;
QXMPP_EXPORT extern const char *ns_roster;
QXMPP_EXPORT extern const char *ns_tls;
QXMPP_EXPORT extern const char *ns_sasl;
QXMPP_EXPORT extern const char *ns_bind;
QXMPP_EXPORT extern const char *ns_session;
QXMPP_EXPORT extern const char *ns_stanza;
QXMPP_EXPORT extern const char *ns_pre_approval;
QXMPP_EXPORT extern const char *ns_rosterver;
// XEP-0009: Jabber-RPC
QXMPP_EXPORT extern const char *ns_rpc;
// XEP-0020: Feature Negotiation
QXMPP_EXPORT extern const char *ns_feature_negotiation;
// XEP-0027: Current Jabber OpenPGP Usage
QXMPP_EXPORT extern const char *ns_legacy_openpgp;
// XEP-0030: Service Discovery
QXMPP_EXPORT extern const char *ns_disco_info;
QXMPP_EXPORT extern const char *ns_disco_items;
// XEP-0033: Extended Stanza Addressing
QXMPP_EXPORT extern const char *ns_extended_addressing;
// XEP-0045: Multi-User Chat
QXMPP_EXPORT extern const char *ns_muc;
QXMPP_EXPORT extern const char *ns_muc_admin;
QXMPP_EXPORT extern const char *ns_muc_owner;
QXMPP_EXPORT extern const char *ns_muc_user;
// XEP-0047: In-Band Bytestreams
QXMPP_EXPORT extern const char *ns_ibb;
// XEP-0049: Private XML Storage
QXMPP_EXPORT extern const char *ns_private;
// XEP-0054: vcard-temp
QXMPP_EXPORT extern const char *ns_vcard;
// XEP-0059: Result Set Management
QXMPP_EXPORT extern const char *ns_rsm;
// XEP-0060: Publish-Subscribe
QXMPP_EXPORT extern const char *ns_pubsub;
QXMPP_EXPORT extern const char *ns_pubsub_auto_create;
QXMPP_EXPORT extern const char *ns_pubsub_config_node;
QXMPP_EXPORT extern const char *ns_pubsub_config_node_max;
QXMPP_EXPORT extern const char *ns_pubsub_create_and_configure;
QXMPP_EXPORT extern const char *ns_pubsub_create_nodes;
QXMPP_EXPORT extern const char *ns_pubsub_errors;
QXMPP_EXPORT extern const char *ns_pubsub_event;
QXMPP_EXPORT extern const char *ns_pubsub_multi_items;
QXMPP_EXPORT extern const char *ns_pubsub_node_config;
QXMPP_EXPORT extern const char *ns_pubsub_owner;
QXMPP_EXPORT extern const char *ns_pubsub_publish;
QXMPP_EXPORT extern const char *ns_pubsub_publish_options;
QXMPP_EXPORT extern const char *ns_pubsub_rsm;
// XEP-0065: SOCKS5 Bytestreams
QXMPP_EXPORT extern const char *ns_bytestreams;
// XEP-0066: Out of Band Data
QXMPP_EXPORT extern const char *ns_oob;
// XEP-0071: XHTML-IM
QXMPP_EXPORT extern const char *ns_xhtml;
QXMPP_EXPORT extern const char *ns_xhtml_im;
// XEP-0077: In-Band Registration
QXMPP_EXPORT extern const char *ns_register;
QXMPP_EXPORT extern const char *ns_register_feature;
// XEP-0078: Non-SASL Authentication
QXMPP_EXPORT extern const char *ns_auth;
QXMPP_EXPORT extern const char *ns_authFeature;
// XEP-0080: User Location
QXMPP_EXPORT extern const char *ns_geoloc;
QXMPP_EXPORT extern const char *ns_geoloc_notify;
// XEP-0085: Chat State Notifications
QXMPP_EXPORT extern const char *ns_chat_states;
// XEP-0091: Legacy Delayed Delivery
QXMPP_EXPORT extern const char *ns_legacy_delayed_delivery;
// XEP-0092: Software Version
QXMPP_EXPORT extern const char *ns_version;
QXMPP_EXPORT extern const char *ns_data;
// XEP-0095: Stream Initiation
QXMPP_EXPORT extern const char *ns_stream_initiation;
QXMPP_EXPORT extern const char *ns_stream_initiation_file_transfer;
// XEP-0103: URL Address Information
QXMPP_EXPORT extern const char *ns_url_data;
// XEP-0108: User Activity
QXMPP_EXPORT extern const char *ns_activity;
// XEP-0115: Entity Capabilities
QXMPP_EXPORT extern const char *ns_capabilities;
// XEP-0118: User Tune
QXMPP_EXPORT extern const char *ns_tune;
QXMPP_EXPORT extern const char *ns_tune_notify;
// XEP-0136: Message Archiving
QXMPP_EXPORT extern const char *ns_archive;
// XEP-0138: Stream Compression
QXMPP_EXPORT extern const char *ns_compress;
QXMPP_EXPORT extern const char *ns_compressFeature;
// XEP-0145: Annotations
QXMPP_EXPORT extern const char *ns_rosternotes;
// XEP-0153: vCard-Based Avatars
QXMPP_EXPORT extern const char *ns_vcard_update;
// XEP-0158: CAPTCHA Forms
QXMPP_EXPORT extern const char *ns_captcha;
// XEP-0166: Jingle
QXMPP_EXPORT extern const char *ns_jingle;
QXMPP_EXPORT extern const char *ns_jingle_ice_udp;
QXMPP_EXPORT extern const char *ns_jingle_raw_udp;
// XEP-0167: Jingle RTP Sessions
QXMPP_EXPORT extern const char *ns_jingle_rtp;
QXMPP_EXPORT extern const char *ns_jingle_rtp_audio;
QXMPP_EXPORT extern const char *ns_jingle_rtp_video;
QXMPP_EXPORT extern const char *ns_jingle_rtp_info;
QXMPP_EXPORT extern const char *ns_jingle_rtp_errors;
// XEP-0184: Message Receipts
QXMPP_EXPORT extern const char *ns_message_receipts;
// XEP-0198: Stream Management
QXMPP_EXPORT extern const char *ns_stream_management;
// XEP-0199: XMPP Ping
QXMPP_EXPORT extern const char *ns_ping;
// XEP-0202: Entity Time
QXMPP_EXPORT extern const char *ns_entity_time;
// XEP-0203: Delayed Delivery
QXMPP_EXPORT extern const char *ns_delayed_delivery;
// XEP-0220: Server Dialback
QXMPP_EXPORT extern const char *ns_server_dialback;
// XEP-0221: Data Forms Media Element
QXMPP_EXPORT extern const char *ns_media_element;
// XEP-0224: Attention
QXMPP_EXPORT extern const char *ns_attention;
// XEP-0231: Bits of Binary
QXMPP_EXPORT extern const char *ns_bob;
// XEP-0249: Direct MUC Invitations
QXMPP_EXPORT extern const char *ns_conference;
// XEP-0264: Jingle Content Thumbnails
QXMPP_EXPORT extern const char *ns_thumbs;
// XEP-0272: Multiparty Jingle (Muji)
QXMPP_EXPORT extern const char *ns_muji;
// XEP-0280: Message Carbons
QXMPP_EXPORT extern const char *ns_carbons;
// XEP-0293: Jingle RTP Feedback Negotiation
QXMPP_EXPORT extern const char *ns_jingle_rtp_feedback_negotiation;
// XEP-0294: Jingle RTP Header Extensions Negotiation
QXMPP_EXPORT extern const char *ns_jingle_rtp_header_extensions_negotiation;
// XEP-0297: Stanza Forwarding
QXMPP_EXPORT extern const char *ns_forwarding;
// XEP-0300: Use of Cryptographic Hash Functions in XMPP
QXMPP_EXPORT extern const char *ns_hashes;
// XEP-0308: Last Message Correction
QXMPP_EXPORT extern const char *ns_message_correct;
// XEP-0313: Message Archive Management
QXMPP_EXPORT extern const char *ns_mam;
// XEP-0319: Last User Interaction in Presence
QXMPP_EXPORT extern const char *ns_idle;
// XEP-0320: Use of DTLS-SRTP in Jingle Sessions
QXMPP_EXPORT extern const char *ns_jingle_dtls;
// XEP-0333: Char Markers
QXMPP_EXPORT extern const char *ns_chat_markers;
// XEP-0334: Message Processing Hints:
QXMPP_EXPORT extern const char *ns_message_processing_hints;
// XEP-0352: Client State Indication
QXMPP_EXPORT extern const char *ns_csi;
// XEP-0357: Push Notifications
QXMPP_EXPORT extern const char *ns_push;
// XEP-0359: Unique and Stable Stanza IDs
QXMPP_EXPORT extern const char *ns_sid;
// XEP-0363: HTTP File Upload
QXMPP_EXPORT extern const char *ns_http_upload;
// XEP-0364: Current Off-the-Record Messaging Usage
QXMPP_EXPORT extern const char *ns_otr;
// XEP-0367: Message Attaching
QXMPP_EXPORT extern const char *ns_message_attaching;
// XEP-0369: Mediated Information eXchange (MIX)
QXMPP_EXPORT extern const char *ns_mix;
QXMPP_EXPORT extern const char *ns_mix_create_channel;
QXMPP_EXPORT extern const char *ns_mix_searchable;
QXMPP_EXPORT extern const char *ns_mix_node_messages;
QXMPP_EXPORT extern const char *ns_mix_node_participants;
QXMPP_EXPORT extern const char *ns_mix_node_presence;
QXMPP_EXPORT extern const char *ns_mix_node_config;
QXMPP_EXPORT extern const char *ns_mix_node_info;
// XEP-0373: OpenPGP for XMPP
QXMPP_EXPORT extern const char *ns_ox;
// XEP-0380: Explicit Message Encryption
QXMPP_EXPORT extern const char *ns_eme;
// XEP-0382: Spoiler messages
QXMPP_EXPORT extern const char *ns_spoiler;
// XEP-0384: OMEMO Encryption
QXMPP_EXPORT extern const char *ns_omemo;
QXMPP_EXPORT extern const char *ns_omemo_1;
QXMPP_EXPORT extern const char *ns_omemo_2;
QXMPP_EXPORT extern const char *ns_omemo_2_bundles;
QXMPP_EXPORT extern const char *ns_omemo_2_devices;
// XEP-0405: Mediated Information eXchange (MIX): Participant Server Requirements
QXMPP_EXPORT extern const char *ns_mix_pam;
QXMPP_EXPORT extern const char *ns_mix_roster;
QXMPP_EXPORT extern const char *ns_mix_presence;
// XEP-0407: Mediated Information eXchange (MIX): Miscellaneous Capabilities
QXMPP_EXPORT extern const char *ns_mix_misc;
// XEP-0428: Fallback Indication
QXMPP_EXPORT extern const char *ns_fallback_indication;
// XEP-0434: Trust Messages (TM)
QXMPP_EXPORT extern const char *ns_tm;
// XEP-0444: Message Reactions
QXMPP_EXPORT extern const char *ns_reactions;
// XEP-0446: File metadata element
QXMPP_EXPORT extern const char *ns_file_metadata;
// XEP-0447: Stateless file sharing
QXMPP_EXPORT extern const char *ns_sfs;
// XEP-0448: Encryption for stateless file sharing
QXMPP_EXPORT extern const char *ns_esfs;
// XEP-0450: Automatic Trust Management (ATM)
QXMPP_EXPORT extern const char *ns_atm;

#endif  // QXMPPCONSTANTS_H
