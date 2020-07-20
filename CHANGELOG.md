QXmpp 1.3.1 (Jul 20, 2020)
--------------------------

The most important change of this release is the fix of CVE-2017-5603. QXmpp is
not vulnerable to roster push attacks (CVE-2016-9928).

Fixes:
 - QXmppRosterIq: Set subscriptionType to NotSet correctly (#293, @melvo)
 - Fix `QXMPP_EXPORT` define when linking statically (#299, @leobasilio)
 - QXmppMessageReceiptManager: Ignore all error messages (#300, @lnjX)
 - QXmppCarbonManager: Fix CVE-2017-5603 (missing sender check) (#304, @lnjX)

QXmpp 1.3.0 (Apr 06, 2020)
--------------------------

QXmpp complys with the XMPP Compliance Suites 2020 (XEP-0423) for client
development in the Core, IM and Advanced Mobile suites now. For this prupose it
has been updated to RFC6120 and RFC6121. ABI compatibility was kept with this
release (apart from classes marked as 'NOT FINALIZED').

New features:
 - Port QXmppCallManager to GStreamer (#207, @olesalscheider)
 - Add XEP-0245: The /me Command (#276, @lnjX)
 - Add XEP-0357: Push Notifications: Enable/disable IQ (#271, @jbbgameich,
   @zatroxde)
 - Add XEP-0359: Unique and Stable Stanza IDs (#256, @lnjX)
 - Add XEP-0428: Fallback Indication (#253, @lnjX)
 - Update from RFC3920 to RFC6120:
   * Deprecate PaymentRequired stanza error condition as it was not adopted in
     RFC6120 (#277, @lnjX)
   * Add PolicyViolation stanza error condition added in RFC6120 (#279, @lnjX)
   * Add redirection URI from RFC6120 for Gone and Redirect conditions (#280,
     @lnjX)
   * Add 'by' attribute to QXmppStanza::Error from RFC6120 (#283, @lnjX)
 - Update from RFC3921 to RFC6121:
   * Add pre-approved presence subscriptions (#285, @lnjX):
     - Add 'approved' attribute to QXmppRosterIq
     - Add stream feature for pre-approved presence subscriptions
   * Add stream feature for roster versioning (#286, @lnjX)
 - Use QUuid by default to generate unique stanza IDs (#255, @lnjX)
 - Add roster extension for MIX-PAM (XEP-0405) (#175, @lnjX)
 - Update MAM to v0.6 (including namespace bump to `urn:xmpp:mam:2`) (#254,
   #257, @lnjX)
 - Add not-authorized stream error condition in QXmppOutgoingClient (#191,
   @henry61024)
 - Add missing static service discovery features for supported message
   extensions (#287, @lnjX)
 - Add utility constructor to QXmppDataForm and QXmppDataForm::Field to make
   creation of forms easier (#273, @lnjX)

Misc:
 - Make QXmpp work with projects using `QT_NO_KEYWORDS` (#258, @tomcucinotta)
 - Add hyperlinks to XEP references in the documentation (@lnjX)
 - Move from Travis-CI to GitHub Actions (#265, @jlaine)
 - Replace deprecated `qsrand()` and `qrand()` by QRandomGenerator (#267,
   @melvo)
 - Add tests for QXmppStanza::Error parsing (#279, @lnjX)

Deprecations:
 - `QXmppStanza::Error::Condition::PaymentRequired`: The error condition was
   unused and not adopted in RFC6120

QXmpp 1.2.1 (Apr 01, 2020)
--------------------------

This release contains some bug fixes that have been found in the last two
months. Also, the coverage has slightly improved due to new unit tests for the
bug fixes.

Fixes:
 - QXmppRegistrationManager: Fix failed and succeeded signals are both emitted
   on success (#260, @melvo)
 - QXmppMessageReceiptManager: Fix receipts are sent on error messages
   (#269, @TheBluestBird)
 - QXmppVCardManager: Fix clientVCardReceived() not emitted when IQ is from the
   bare JID of the user (#281, @melvo, @lnjX)
 - QXmppRosterManager: Fix 'ask' attribute is included when renaming item
   (#262, @melvo, @lnjX)
 - QXmppRosterIq: Add missing implementation of the copy constructor (@lnjX)

QXmpp 1.2.0 (Feb 06, 2020)
--------------------------

QXmpp now requires Qt 5.7 or later. Unfortunately ABI compatibility was not
kept this release again. Code coverage has been improved from 68.93% to 69.55%.

New features:
 - Implement XEP-0077: In-band registration:
   - Add registration manager with full unit tests (#248, @lnjX)
   - Add `registered` and `remove` to the IQ (#240, @lnjX)
 - Implement XEP-0231: Bits of Binary (#230, @lnjX)
 - Add `QXmppClient::indexOfExtension<T>()` (#241, @lnjX)
 - Add QXmppStartTlsPacket to replace fixed XML data (#236, @lnjX)
 - Move TLS code to private QXmppTlsManager (#236, @lnjX)
 - Add private QXmppInternalClientExtensions to access private part of the
   client (#243, @lnjX)
 - Add utility methods to QXmppRegisterIq to create common requests (#247, @lnjX)

Fixes:
 - QXmppMucManager: Make it possible to handle stanzas not handled by the
   manager (#226, @kollix)
 - Only send Client State Indication (CSI) states when connected (#232, @lnjX)
 - Fix no documentation is generated for QXmppStanza::Error and
   QXmppStreamFeatures (@lnjX)
 - Fix some doxygen warnings and undocumented Q_PROPERTYs (@lnjX)

Misc:
 - Replace deprecated Q_FOREACH (#210, @lnjX)
 - Replace deprecated Q_ENUMS with Q_ENUM (#227, @lnjX)
 - Replace deprecated signal/slots syntax (#237, @jbbgameich)
 - Switch to Ubuntu Bionic for Travis-CI builds (#210, @lnjX)
 - Use QSharedDataPointers for QXmppRegisterIq, QXmppPubSubIq,
   QXmppDiscoveryIq, QXmppMam{Query,Result}Iq, QXmppStreamFeatures
   (#230, #235, #252, @lnjX)
 - Refactor QXmppPubSubIq and add missing tests (#235, @lnjX)
 - Refactor QXmppPresence and add missing tests (#231, @lnjX)
 - Replace manual xmlns writing by writeDefaultNamespace() (#244, @lnjX)
 - Use QT_VERSION_CHECK to generate QXMPP_VERSION (#238, @lnjX)
 - Add clang-format file (#239, @0xd34df00d)

QXmpp 1.1.0 (Oct 23, 2019)
--------------------------

All new classes and methods in this release are marked in the documentation
with *since QXmpp 1.1*.

New features:
 - Add support for SCRAM-SHA-1 and SCRAM-SHA-256 (#183, @jlaine)
 - Order SASL mechanisms to prefer the most secure (#187, @jlaine)
 - Add XEP-0334: Message Processing Hints (v0.3.0) (#212, @lnjX, @jaragont,
   @sam-truscott)
 - Add XEP-0363: HTTP File Upload (v0.9.0) (#188, @lnjX)
 - Add XEP-0367: Message Attaching (v0.3.0) (#196, @lnjX)
 - Add XEP-0369: Mediated Information eXchange (MIX) (v0.14.2) (partially):
   * Add QXmppMixIq to manage/join channels (#174, @lnjX)
   * Add QXmppMessage and QXmppPresence extensions for MIX (#175, @lnjX)
   * Add channel info and participant PubSub/PEP items (#179, @lnjX)
 - Add XEP-0380: Explicit Message Encryption (v0.3.0) (#199, @lnjX)
 - Add XEP-0382: Spoiler messages (v0.2.0) (#195, @lnjX)

Fixes:
 - Do not accept receipts from other resources of the used account (#192, lnjX)
 - cmake: Set minimum version before creating project() and bump to 3.3 (#205, @jbbgameich)

Deprecations:
 - Deprecate QXmppClient extension getters (#214, @lnjX):
   * `QXmppClient::rosterManager()`:
     Use `QXmppClient::findExtension<QXmppRosterManager>()` instead
   * `QXmppClient::vCardManager()`:
     Use `QXmppClient::findExtension<QXmppVCardManager>()` instead
   * `QXmppClient::versionManager()`:
     Use `QXmppClient::findExtension<QXmppVersionManager>()` instead
 - Refactor data form media element, deprecate `QXmppDataForm::Media` (#222, @lnjX):
   * `QXmppDataForm::Media`:
     Use a list of the new `QXmppDataForm::MediaSource` in combination with a `QSize`
   * `QXmppDataForm::Field::media()` / `QXmppDataForm::Field::setMedia()`:
     Use `QXmppDataForm::Field::mediaSources()` and `QXmppDataForm::Field::mediaSize()`

Misc:
 - Replace deprecated `qSort()` by `std::sort()` (#206, @jbbgameich)
 - Do not use deprecated `QSslSocket::setCaCertificates()` (#206, @jbbgameich)
 - Modernize code by using `nullptr`, `override`, etc. (#204, @jbbgameich)
 - Move attributes into private d-pointer for future ABI compatibility:
   * QXmppRosterIq (#175, @lnjX)
   * QXmppStanza::Error (#203, @lnjX)
 - Use raw literals, range based loops and `auto` (#224, @jbbgameich)

QXmpp 1.0.1 (Oct 14, 2019)
--------------------------

 - Fix potential SEGFAULT on connection error (#216, @0xd34df00d)
 - Fix `SO_VERSION` to 1: ABI has changed since last minor release (#185, @tehnick)
 - Add CMake option for internal tests (`BUILD_INTERNAL_TESTS`) (#184, @jlaine)

QXmpp 1.0.0 (Jan 8, 2019)
-------------------------

New features:
 - Add XEP-0066: Out of Band Data (partially) (#167, @lnjX)
 - Add XEP-0198: Stream Management (#99, @olesalscheider)
 - Add XEP-0237: Roster Versioning (#142, @LightZam)
 - Add XEP-0280: Message Carbons (#88, @fbeutel)
 - Add XEP-0308: Last Message Correction (#170, @lnjX)
 - Add XEP-0313: Message Archive Management (#120, @olesalscheider)
 - Add XEP-0319: Last User Interaction in Presence (#171, @lnjX)
 - Add XEP-0352: Client State Indication (#159, @fbeutel, @lnjX)
 - Auto-connect to next DNS-SRV record server on connection failure
   (#105, @kollix)
 - QXmppVersionManager: Use QSysInfo to determine default OS (#168, @lnjX)
 - QXmppDiscoveryManager: Default to `phone` type on mobile platforms
   (#168, @lnjX)
 - CMake based build system (#131, @olesalscheider)
 - Add BUILD_SHARED option (#160, @LightZam)
 - Use C++11 compiler standard (@jlaine)

Fixes:
 - Do not ignore SSL errors by default (#113), if you need to deal with
   broken SSL configurations, set QXmppConfiguration::ignoreSslErrors to true.
   (@jlaine)
 - Disable tests that require QXMPP_AUTOTEST_EXPORT (fixes #149) (@jlaine)
 - Fix QXmppSslServer::incomingConnection signature (#131, @olesalscheider)
 - Add missed variables initialization in constructors of few classes
   (#122, @tehnick)

Tests:
 - travis: Test builds with clang (@0xd34df00d)
 - travis: Switch to Ubuntu Xenial (#151, @tehnick)
 - tests: Generate coverage repot (@jlaine)
 - Build examples by default

Deprecations:
 - Drop Qt4 support (#131, @olesalscheider)
 - Remove example_4 / GuiClient (#131, @olesalscheider)

QXmpp 0.9.3 (Dec 3, 2015)
-------------------------

  - Add QXmppIceConnection::gatheringState property.
  - Improve QXmppTransferManager::sendFile's handling of QIODevice ownership.
  - Fix QXmppTransferManagerFix convering filename to a QUrl.

QXmpp 0.9.2 (Sep 2, 2015)
-------------------------

  - Fix build error for debug builds.
  - Allow QXmppJingleIq to have multiple contents.

QXmpp 0.9.1 (Aug 30, 2015)
--------------------------

  - Fix build error when VPX support is enabled (issue 71).

QXmpp 0.9.0 (Aug 28, 2015)
--------------------------

  - Fix phone numbers incorrectly read from / written to vCard as "PHONE"
    element instead of "TEL" (issue 65).
  - Make QXmppClient::connectToServer(QXmppConfiguration, QXmppPresence) a
    slot (issue 63).
  - Correctly receive data immediately following a SOCKS5 message (issue 64).
  - Make QXmppStream handle end of incoming stream (issue 70).
  - Add unit tests for QXmppCallManager and QXmppTransferManager.
  - Improve ICE implementation to follow RFC 5245 more closely and hide
    implementation details from public API.

QXmpp 0.8.3 (Mar 13, 2015)
--------------------------

  - Add a QXmppClient::sslErrors signal to report SSL errors.
  - Handle broken servers which send "bad-auth" instead of "not-authorized".
  - Fix a compilation issue with Qt 5.5 due to a missing header include.
  - Do not install test cases.
  - Remove trailing comma after last item in enums.

QXmpp 0.8.2 (Jan 7, 2015)
-------------------------

  - The previous release was missing an update to the VERSION definition,
    resulting in stale pkg-config files. This release fixes this issue.
  - Refactor HTML documentation so that "make docs" works in out-of-source
    builds.
  - Add support for Opus audio codec.
  - Enable error concealment for VPX video codec.

QXmpp 0.8.1 (Dec 19, 2014)
--------------------------

  - Use QString() instead of "" for default methods arguments, to enable
    building project which use QT_NO_CAST_FROM_ASCII.
  - Add support for legacy SSL.
  - Add XEP-0333: Chat Markers attributes to QXmppMessage.
  - Add QXmppClient::socketErrorString to retrieve socket error string.
  - Add equality/inequality operators for QXmppVCardIq.
  - Add "make check" command to run tests.

QXmpp 0.8.0 (Mar 26, 2014)
--------------------------

  - Fix QXmppServer incoming connections with Qt5 (issue 175).
  - Support for QXmppMessage extensions having tag names other than 'x'.
  - Support for retrieving the source QDomElement that has been used to
    initialize a QXmppElement.
  - Add organizations info interface to QXmppVCardIq.
  - Remove deprecated QXmppPresence::Status type.

QXmpp 0.7.6 (Mar 9, 2013)
-------------------------

  - Add QXmppClient::insertExtension to insert an extension at a given index.
  - Disable Facebook / Google / Facebook specific mechanisms if we do not
    have the corresponding credentials.

QXmpp 0.7.5 (Jan 11, 2013)
--------------------------

  - Replace toAscii/fromAscii with toLatin1/fromLatin1 for Qt 5 compatibility.
  - Fix build using clang in pedantic mode.

QXmpp 0.7.4 (Oct 1, 2012)
-------------------------

  - Add XEP-0249: Direct MUC Invitations attributes to QXmppMessage.
  - Add XEP-0045: Multi-User Chat attributes to QXmppPresence.
  - Improve GuiClient, stop using deprecated APIs.
  - Improve QXmppServer:
    * Move statistics to a counter / gauge system.
    * Make it possible to call listenForClients and listenForServers
      multiple times to supported multiple IP address / ports.
  - Improve QXmppTransferManager:
    * Change third argument of QXmppTransferManager::sendFile to a description.
    * Enable file transfer using IPv6.
    * Allow StreamHost::host to contain a host name.

QXmpp 0.7.3 (Sep 7, 2012)
-------------------------

  - Fix QXmppMucRoom::name(), only consider discovery IQs from the room.

QXmpp 0.7.2 (Sep 6, 2012)
-------------------------

  - Handle Error replies in QXmppDiscoveryManager so that library users can know
    about errors.
  - If building with Qt 5, use Qt's QDnsLookup instead of our backport.
  - Improve MUC scriptability:
    * Add QXmppMucRoom::ban() to ban users.
    * Add QXmppMucRoom::name() to get the room's human-readable name.
    * Add QXmppMucRoom::participantFullJid() to lookup an occupant full JID.
  - With Qt >= 4.8, verify peer SSL certificate against domain name as specified by RFC 3920.
  - Add support for X-OAUTH2 authentication for Google Talk.
  - Add links to RFCs in generated HTML documentation.

QXmpp 0.7.1 (Sep 3, 2012)
-------------------------

  - Fix export of QXmppVCardPhone class.

QXmpp 0.7.0 (Sep 3, 2012)
-------------------------

  - New XEPs:
    * XEP-0033: Extended Stanza Addressing

  - Remove deprecated APIs:
    * QXmppRosterManager::rosterChanged()
    * QXmppConfiguration::sASLAuthMechanism()

  - Improve vCard support:
    * Add support for free-form descriptive text.
    * Make it possible to have several addresses.
    * Make it possible to have several e-mail addresses.
    * Make it possible to have several phone numbers.
  - Make it possible to set the client's extended information form (XEP-0128).
  - Make sure QXmppDiscoveryManager only emits results.
  - Fix XEP-0115 verification strings (remove duplicate features, sort form values)
  - Fix issues:
    * Issue 144: QXmppBookmarkConference autojoin parsing
  - Add support for see-other-host server change.
  - Add support for X-MESSENGER-OAUTH2 authentication for Windows Live Messenger.
  - Make it possible to disable non-SASL authentication.
  - Add QXmppClient::isAuthenticated() to query whether authentication has been
    performed.

QXmpp 0.6.3 (Jul 24, 2012)
--------------------------

  - Fix regression in X-FACEBOOK-PLATFORM authentication.

QXmpp 0.6.2 (Jul 22, 2012)
--------------------------

  - New XEPs
    * XEP-0071: XHTML-IM

  - Improve SASL code test coverage.
  - Improve QXmppMessage test coverage.
  - Add a "reason" argument to QXmppRosterManager's subscription methods.
  - Refactor QXmppPresence:
    * add availableStatusType(), priority(), statusText()
    * deprecate QXmppPresence::Status
  - Remove deprecated QXmppRosterManager::removeRosterEntry().

QXmpp 0.6.1 (Jul 20, 2012)
--------------------------

  - New XEPs
    * XEP-0221: Data Forms Media Element

  - Fix data form title/instructions XML serialization.
  - Remove confusing QXmppPresence::Status::Offline status type.
  - Deprecate QXmppConfiguration::setSASLAuthMechanism(), replaced by
    the string-based QXmppConfiguration::setSaslAuthMechanism().

  - Fix issues:
    * Issue 111: QXmppPresence::Status::getTypeStr() gives warning if type is invisible
    * Issue 126: Modularize SASL mechanisms

QXmpp 0.5.0 (Jul 18, 2012)
--------------------------

  - New XEPs
    * XEP-0059: Result Set Management

  - Build a shared library by default.
  - Advertise support for XEP-0249: Direct MUC Invitations
  - Make QXmppTransferManager fully asynchronous.
  - Remove QXmppPacket class.
  - Move utility methods to a QXmppUtils class.
  - Remove QXmppReconnectionManager, QXmppClient handles reconnections.
  - Improve QXmppArchiveManager to allow paginated navigation (Olivier Goffart).
  - Only emit QXmppVersionManager::versionReceived() for results.
  - Remove deprecated QXmppClient::discoveryIqReceived() signal.

  - Fix issues:
    * Issue 64: Compile qxmpp as shared library by default
    * Issue 79: Export classes for Visual C++ Compiler
    * Issue 140: Proper XEP-0115 ver string generation with dataforms
    * Issue 142: qxmpp does not build in Qt5

QXmpp 0.4.0 (Apr 12, 2012)
--------------------------

  - New XEPs
    * XEP-0048: Bookmarks
    * XEP-0184: Message Delivery Receipts
    * XEP-0224: Attention

  - Remove deprecated  "get*" getter accessors from:
    QXmppClient
    QXmppConfiguration
    QXmppMessage
    QXmppPresence
    QXmppIq
    QXmppStanza
	QXmppVCardIq
	QXmppRosterIq

  - Remove deprecated headers:
    * QXmppRoster.h
    * QXmppVCard.h

  - Add TURN support for VoIP calls to use a relay in double-NAT network topologies.
  - Overhaul Multi-User Chat support to make it easier and more fully featured.
  - Improve QXmppServer packet routing performance.
  - Add support for X-FACEBOOK-PLATFORM SASL method.
  - Improve XEP-0136 support to enable archive deletion.
  - Set default keep-alive timeout to 20 seconds, enables detection of broken connections.
  - Make install path configurable using the PREFIX variable instead of Qt's installation path.
  - Make it possible to build a shared library by invoking "qmake QXMPP_LIBRARY_TYPE=lib".

  - Fix issues:
    * Issue 95: Patch for several utility methods in RosterManager
    * Issue 103: Does not compile for Symbian^3 with NokiaQtSDK 1.1 Beta
    * Issue 105: Initial presence is set before the roster request
    * Issue 106: QXmppClient can't override Qt's set of trusted SSL CAs
    * Issue 109: Patch for XEP-0224 (Attention)
    * Issue 113: qxmpp.pc sets incorrect include path
    * Issue 116: sessionStarted not set for non-SASL connections
    * Issue 119: ICE negotiation time out after successful ICE check
    * Issue 120: QXmppIceComponent doesn't accept interfaces with 255.255.255.255 netmask as a local candidate
    * Issue 132: [FreeBSD]: build error
    * Issue 135: qxmpp won't reconnect when disconnected

QXmpp 0.3.0 (Mar 05, 2011)
------------------------
  - New XEPs
    * XEP-0153: vCard-Based Avatars
    * XEP-0202: Entity Time

  - New Classes
    * QXmppClientExtension: base class for QXmppClient extensions (managers)
    * QXmppServer: base class for building XMPP servers
    * QXmppServerExtension: base class for QXmppServer extensions
    * QXmppDiscoveryManager: manager class for XEP-0030: Service Discovery
    * QXmppVersionManager: manager class for XEP-0092: Software Version
    * QXmppIceConnection: class representing an Interactive Connectivity
      Establishment (ICE) over UDP "connection"
    * QXmppRtpChannel: class representing an RTP audio channel for VoIP calls
  
  - Refactor QXmppVCardManager to use QXmppClientExtension
    
  - New Examples
    * example_9_vCard: vCard handling example
    * GuiClient: Graphical chat client, test bench for QXmpp functionalities

  - Deprecation
    * QXmppVCard class name changed to QXmppVCardIq
    * Signal QXmppClient::discoveryIqReceived in favour of 
      QXmppDiscoveryManager::infoReceived and QXmppDiscoveryManager::itemsReceived
  
  - Removal
    Extensions QXmppArchiveManager, QXmppMucManager, QXmppCallManager, QXmppTransferManager
    will not load by default. Therefore following functions to provide the reference 
    have been removed.
    QXmppClient::transferManager()
    QXmppClient::archiveManager()
    QXmppClient::callManager()
    QXmppClient::mucManager()
    Note: Once should use QXmppClient::addExtension() and QXmppClient::findExtension()
          to load or enable these extensions.
  
  - Add support for DNS SRV lookups, meaning you can connect to nearly all
    servers using only a JID and a password.
  - Improve support for SASL authentication, with a verification of the second
    challenge message sent by the server.
  - Add support for the birthday and URL attributes in vCards.
  - Improve STUN support for VoIP calls by detecting server-reflexive address.
  - Add QXMPP_VERSION and QXmppVersion() for compile and run time version checks. 
  - Improve code documentation coverage and quality.
  - Remove dependency on QtGui, making it easier to write console applications.
  - Fix MSVC 2005 and 2008 build issues.
  - Fix Symbian build issues, add DNS SRV support for Symbian devices.
  
QXmpp 0.2.0 (Aug 22, 2010)
--------------------------
  - New XEPs
    * XEP-0030: Service Discovery
    * XEP-0045: Multi-User Chat 
    * XEP-0047: In-Band Bytestreams
    * XEP-0054: vcard-temp
    * XEP-0065: SOCKS5 Bytestreams
    * XEP-0078: Non-SASL Authentication
    * XEP-0082: XMPP Date and Time Profiles
    * XEP-0085: Chat State Notifications
    * XEP-0091: Legacy Delayed Delivery
    * XEP-0092: Software Version
    * XEP-0095: Stream Initiation
    * XEP-0096: SI File Transfer
    * XEP-0115: Entity Capabilities
    * XEP-0128: Service Discovery Extensions
    * XEP-0166: Jingle
    * XEP-0167: Jingle RTP Sessions
    * XEP-0199: XMPP Ping
    * XEP-0203: Delayed Delivery 
    * XEP-0009: Jabber-RPC
    * XEP-0004: Data Forms
    
  - New XEPs (Initial Support) 
    * XEP-0136: Message Archiving
    * XEP-0176: Jingle ICE-UDP Transport Method [Experimental]

  - New authentication schemes
      * DIGEST-MD5
      * SASL
      * NonSASL
      * Anonymous

  - Add doxygen documentation
  - Add targets in *.pro file for packaging, installing and generating documentation
  - Use QXmlStreamWriter while creating stanzas to be sent to the server 
  - Clean up getter accessors from "getFoo" to "foo"
  - Add proper file transfer management
  - Add support for keep-alive pings
  - Report authentication errors
  - Automatic reconnection mechanism
  - Test suite for stanza parsing/serialisation
  - Refactor the logging code
  - Add proxy support
  - Fixed compile time warning messages
  - New examples
  - Support for attaching an extension element to messages and presences (QXmppElement)
  - Move parsing to the stanzas itself QXmppStanza::parse()
  - QXMPP_NO_GUI define to remove dependency on QtGui
  - Change QXmppRoster to QXmppRosterManager to have a consistent API
  
QXmpp 0.1 (Jun 14, 2009)
------------------------
  - First public release
