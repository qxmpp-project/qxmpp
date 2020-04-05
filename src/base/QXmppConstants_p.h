/*
 * Copyright (C) 2008-2020 The QXmpp developers
 *
 * Author:
 *  Manjeet Dahiya
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

#ifndef QXMPPCONSTANTS_H
#define QXMPPCONSTANTS_H

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

extern const char* ns_stream;
extern const char* ns_client;
extern const char* ns_server;
extern const char* ns_roster;
extern const char* ns_tls;
extern const char* ns_sasl;
extern const char* ns_bind;
extern const char* ns_session;
extern const char* ns_stanza;
extern const char* ns_pre_approval;
extern const char* ns_rosterver;
// XEP-0009: Jabber-RPC
extern const char* ns_rpc;
// XEP-0020: Feature Negotiation
extern const char* ns_feature_negotiation;
// XEP-0027: Current Jabber OpenPGP Usage
extern const char* ns_legacy_openpgp;
// XEP-0030: Service Discovery
extern const char* ns_disco_info;
extern const char* ns_disco_items;
// XEP-0033: Extended Stanza Addressing
extern const char* ns_extended_addressing;
// XEP-0045: Multi-User Chat
extern const char* ns_muc;
extern const char* ns_muc_admin;
extern const char* ns_muc_owner;
extern const char* ns_muc_user;
// XEP-0047: In-Band Bytestreams
extern const char* ns_ibb;
// XEP-0049: Private XML Storage
extern const char* ns_private;
// XEP-0054: vcard-temp
extern const char* ns_vcard;
// XEP-0059: Result Set Management
extern const char* ns_rsm;
// XEP-0060: Publish-Subscribe
extern const char* ns_pubsub;
// XEP-0065: SOCKS5 Bytestreams
extern const char* ns_bytestreams;
// XEP-0066: Out of Band Data
extern const char* ns_oob;
// XEP-0071: XHTML-IM
extern const char* ns_xhtml;
extern const char* ns_xhtml_im;
// XEP-0077: In-Band Registration
extern const char* ns_register;
extern const char* ns_register_feature;
// XEP-0078: Non-SASL Authentication
extern const char* ns_auth;
extern const char* ns_authFeature;
// XEP-0085: Chat State Notifications
extern const char* ns_chat_states;
// XEP-0091: Legacy Delayed Delivery
extern const char* ns_legacy_delayed_delivery;
// XEP-0092: Software Version
extern const char* ns_version;
extern const char* ns_data;
// XEP-0095: Stream Initiation
extern const char* ns_stream_initiation;
extern const char* ns_stream_initiation_file_transfer;
// XEP-0108: User Activity
extern const char* ns_activity;
// XEP-0115: Entity Capabilities
extern const char* ns_capabilities;
// XEP-0136: Message Archiving
extern const char* ns_archive;
// XEP-0138: Stream Compression
extern const char* ns_compress;
extern const char* ns_compressFeature;
// XEP-0145: Annotations
extern const char* ns_rosternotes;
// XEP-0153: vCard-Based Avatars
extern const char* ns_vcard_update;
// XEP-0158: CAPTCHA Forms
extern const char* ns_captcha;
// XEP-0166: Jingle
extern const char* ns_jingle;
extern const char* ns_jingle_ice_udp;
extern const char* ns_jingle_raw_udp;
extern const char* ns_jingle_rtp;
extern const char* ns_jingle_rtp_audio;
extern const char* ns_jingle_rtp_video;
// XEP-0184: Message Receipts
extern const char* ns_message_receipts;
// XEP-0198: Stream Management
extern const char* ns_stream_management;
// XEP-0199: XMPP Ping
extern const char* ns_ping;
// XEP-0202: Entity Time
extern const char* ns_entity_time;
// XEP-0203: Delayed Delivery
extern const char* ns_delayed_delivery;
// XEP-0220: Server Dialback
extern const char* ns_server_dialback;
// XEP-0221: Data Forms Media Element
extern const char* ns_media_element;
// XEP-0224: Attention
extern const char* ns_attention;
// XEP-0231: Bits of Binary
extern const char* ns_bob;
// XEP-0249: Direct MUC Invitations
extern const char* ns_conference;
// XEP-0280: Message Carbons
extern const char* ns_carbons;
// XEP-0297: Stanza Forwarding
extern const char* ns_forwarding;
// XEP-0308: Last Message Correction
extern const char* ns_message_correct;
// XEP-0313: Message Archive Management
extern const char* ns_mam;
// XEP-0319: Last User Interaction in Presence
extern const char* ns_idle;
// XEP-0333: Char Markers
extern const char* ns_chat_markers;
// XEP-0334: Message Processing Hints:
extern const char* ns_message_processing_hints;
// XEP-0352: Client State Indication
extern const char* ns_csi;
// XEP-0357: Push Notifications
extern const char* ns_push;
// XEP-0359: Unique and Stable Stanza IDs
extern const char* ns_sid;
// XEP-0363: HTTP File Upload
extern const char* ns_http_upload;
// XEP-0364: Current Off-the-Record Messaging Usage
extern const char* ns_otr;
// XEP-0367: Message Attaching
extern const char* ns_message_attaching;
// XEP-0369: Mediated Information eXchange (MIX)
extern const char* ns_mix;
extern const char* ns_mix_create_channel;
extern const char* ns_mix_searchable;
extern const char* ns_mix_node_messages;
extern const char* ns_mix_node_participants;
extern const char* ns_mix_node_presence;
extern const char* ns_mix_node_config;
extern const char* ns_mix_node_info;
// XEP-0373: OpenPGP for XMPP
extern const char* ns_ox;
// XEP-0380: Explicit Message Encryption
extern const char* ns_eme;
// XEP-0382: Spoiler messages
extern const char* ns_spoiler;
// XEP-0384: OMEMO Encryption
extern const char* ns_omemo;
// XEP-0405: Mediated Information eXchange (MIX): Participant Server Requirements
extern const char* ns_mix_pam;
extern const char* ns_mix_roster;
extern const char* ns_mix_presence;
// XEP-0428: Fallback Indication
extern const char* ns_fallback_indication;

#endif  // QXMPPCONSTANTS_H
