#ifndef _RADIUSMSG_H_
#define _RADIUSMSG_H_

#include <WiFiUdp.h>

#define RADIUS_AUTHENTICATOR_LENGTH 16
#define RADIUS_PASSWORD_BLOCK_SIZE 16
#define RADIUS_HEADER_LENGTH 20
// In actuality, the max permitted RADIUS packet size is 4096, but arduino cant handle that size
// so we artificially limit packets to 1000 octets
#define RADIUS_MAX_SIZE 1000
#define RADIUS_MAX_ATTRIBUTE_SIZE 253
typedef uint8_t RadiusAuthenticator[RADIUS_AUTHENTICATOR_LENGTH];

// RADIUS message type
typedef enum {
    RadiusCodeAccessRequest                        = 1,
    RadiusCodeAccessAccept                         = 2,
    RadiusCodeAccessReject                         = 3,
    RadiusCodeAccountingRequest                    = 4,
    RadiusCodeAccountingResponse                   = 5,
    RadiusCodeAccountingStatus                     = 6,
    RadiusCodeAccessPasswordRequest                = 7,
    RadiusCodeAccessPasswordAck                    = 8,
    RadiusCodeAccessPasswordReject                 = 9,
    RadiusCodeAccountingMessage                    = 10,
    RadiusCodeAccessChallenge                      = 11,
    RadiusCodeStatusServer                         = 12,
    RadiusCodeStatusClient                         = 13,
    RadiusCodeResourceFreeRequest                  = 21,
    RadiusCodeResourceFreeResponse                 = 22,
    RadiusCodeResourceQueryRequest                 = 23,
    RadiusCodeResourceQueryResponse                = 24,
    RadiusCodeAlternateResourceReclaimRequest      = 25,
    RadiusCodeNASRebootRequest                     = 26,
    RadiusCodeNASRebootResponse                    = 27,
    RadiusCodeAscendAccessNextCode                 = 29,
    RadiusCodeAscendAccessNewPin                   = 30,
    RadiusCodeAscendTerminateSession               = 31,
    RadiusCodeAscendPasswordExpired                = 32,
    RadiusCodeAscendAccessEventRequest             = 33,
    RadiusCodeAscendAccessEventResponse            = 34,
    RadiusCodeDisconnectRequest                    = 40,
    RadiusCodeDisconnectRequestACKed               = 41,
    RadiusCodeDisconnectRequestNAKed               = 42,
    RadiusCodeChangeFilterRequest                  = 43,
    RadiusCodeChangeFilterRequestACKed             = 44,
    RadiusCodeChangeFilterRequestNAKed             = 45,
    RadiusCodeIPAddressAllocate                    = 50,
    RadiusCodeIPAddressRelease                     = 51,
    // This is the next code after the last valid one
    RadiusCodeStartInvalidCodes                    = 52
} RadiusCode;

const unsigned RadiusAttrUserName               = 1;
const unsigned RadiusAttrUserPassword           = 2;
const unsigned RadiusAttrChapPassword           = 3;
const unsigned RadiusAttrNasIPAddress           = 4;
const unsigned RadiusAttrNASPort                = 5;
const unsigned RadiusAttrServiceType            = 6;
const unsigned RadiusAttrFramedProtocol         = 7;
const unsigned RadiusAttrFramedIPAddress        = 8;
const unsigned RadiusAttrFramedIPNetmask        = 9;
const unsigned RadiusAttrFramedRouting          = 10;
const unsigned RadiusAttrFilterId               = 11;
const unsigned RadiusAttrFramedMTU              = 12;
const unsigned RadiusAttrFramedCompression      = 13;
const unsigned RadiusAttrLoginIPHost            = 14;
const unsigned RadiusAttrLoginService           = 15;
const unsigned RadiusAttrLoginTCPPort           = 16;
const unsigned RadiusAttrOldPassword            = 17;
const unsigned RadiusAttrReplyMessage           = 18;
const unsigned RadiusAttrCallbackNumber         = 19;
const unsigned RadiusAttrCallbackId             = 20;
const unsigned RadiusAttrAscendPWExpiration     = 21;
const unsigned RadiusAttrFramedRoute            = 22;
const unsigned RadiusAttrFramedIPXNetwork       = 23;
const unsigned RadiusAttrState                  = 24;
const unsigned RadiusAttrClass                  = 25;
const unsigned RadiusAttrVendorSpecific         = 26;
const unsigned RadiusAttrSessionTimeout         = 27;
const unsigned RadiusAttrIdleTimeout            = 28;
const unsigned RadiusAttrTerminationAction      = 29;
const unsigned RadiusAttrCalledStationId        = 30;
const unsigned RadiusAttrCallingStationId       = 31;
const unsigned RadiusAttrNASIdentifier          = 32;
const unsigned RadiusAttrProxyState             = 33;
const unsigned RadiusAttrLoginLATService        = 34;
const unsigned RadiusAttrLoginLATNode           = 35;
const unsigned RadiusAttrLoginLATGroup          = 36;
const unsigned RadiusAttrFramedAppleTalkLink    = 37;
const unsigned RadiusAttrFramedAppleTalkNetwork = 38;
const unsigned RadiusAttrFramedAppleTalkZone    = 39;
const unsigned RadiusAttrAcctStatusType         = 40;
const unsigned RadiusAttrAcctDelayTime          = 41;
const unsigned RadiusAttrAcctInputOctets        = 42;
const unsigned RadiusAttrAcctOutputOctets       = 43;
const unsigned RadiusAttrAcctSessionId          = 44;
const unsigned RadiusAttrAcctAuthentic          = 45;
const unsigned RadiusAttrAcctSessionTime        = 46;
const unsigned RadiusAttrAcctInputPackets       = 47;
const unsigned RadiusAttrAcctOutputPackets      = 48;
const unsigned RadiusAttrAcctTerminateCause     = 49;
const unsigned RadiusAttrAcctMultiSessionId     = 50;
const unsigned RadiusAttrAcctLinkCount          = 51;
const unsigned RadiusAttrAcctInputGigawords     = 52;
const unsigned RadiusAttrAcctOutputGigawords    = 53;

const unsigned RadiusAttrEventTimestamp         = 55;
const unsigned RadiusAttrEgressVLANID           = 56;
const unsigned RadiusAttrIngressFilters         = 57;
const unsigned RadiusAttrEgressVLANName         = 58;
const unsigned RadiusAttrUserPriorityTable      = 59;
const unsigned RadiusAttrCHAPChallenge          = 60;
const unsigned RadiusAttrNASPortType            = 61;
const unsigned RadiusAttrPortLimit              = 62;
const unsigned RadiusAttrLoginLATPort           = 63;
const unsigned RadiusAttrTunnelType             = 64;
const unsigned RadiusAttrTunnelMediumType       = 65;
const unsigned RadiusAttrTunnelClientEndpoint   = 66;
const unsigned RadiusAttrTunnelServerEndpoint   = 67;
const unsigned RadiusAttrTunnelID               = 68;
const unsigned RadiusAttrTunnelPassword         = 69;
const unsigned RadiusAttrARAPPassword           = 70;
const unsigned RadiusAttrARAPFeatures           = 71;
const unsigned RadiusAttrARAPZoneAccess         = 72;
const unsigned RadiusAttrARAPSecurity           = 73;
const unsigned RadiusAttrARAPSecurityData       = 74;
const unsigned RadiusAttrPasswordRetry          = 75;
const unsigned RadiusAttrPrompt                 = 76;
const unsigned RadiusAttrConnectInfo            = 77;
const unsigned RadiusAttrConfigurationToken     = 78;
const unsigned RadiusAttrEAPMessage             = 79;
const unsigned RadiusAttrMessageAuthenticator   = 80;
const unsigned RadiusAttrTunnelPrivateGroupID   = 81;
const unsigned RadiusAttrTunnelAssignmentID     = 82;
const unsigned RadiusAttrTunnelPreference       = 83;
const unsigned RadiusAttrARAPChallengeResponse  = 84;
const unsigned RadiusAttrAcctInterimInterval    = 85;
const unsigned RadiusAttrAcctTunnelPacketsLost  = 86;
const unsigned RadiusAttrNASPortId              = 87;
const unsigned RadiusAttrFramedPool             = 88;
const unsigned RadiusAttrChargeableUserIdentity = 89;
const unsigned RadiusAttrTunnelClientAuthID     = 90;
const unsigned RadiusAttrTunnelServerAuthID     = 91;
const unsigned RadiusAttrNASFilterRule          = 92;
const unsigned RadiusAttrOriginatingLineInfo    = 94;
const unsigned RadiusAttrNASIPv6Address         = 95;
const unsigned RadiusAttrFramedInterfaceId      = 96;
const unsigned RadiusAttrFramedIPv6Prefix       = 97;
const unsigned RadiusAttrLoginIPv6Host          = 98;
const unsigned RadiusAttrFramedIPv6Route        = 99;
const unsigned RadiusAttrFramedIPv6Pool         = 100;
const unsigned RadiusAttrErrorCause             = 101;
const unsigned RadiusAttrEAPKeyName             = 102;
const unsigned RadiusAttrTimestamp              = 103;

const unsigned RadiusAttrDelegatedIPv6Prefix    = 123;

// Well known Radius Attribute Values
const unsigned RadiusValueAcctStatusTypeStart            = 1;
const unsigned RadiusValueAcctStatusTypeStop             = 2;
const unsigned RadiusValueAcctStatusTypeAlive            = 3;
const unsigned RadiusValueAcctStatusTypeModemStart       = 4;
const unsigned RadiusValueAcctStatusTypeModemStop        = 5;
const unsigned RadiusValueAcctStatusTypeCancel           = 6;
const unsigned RadiusValueAcctStatusTypeAccountingOn     = 7;
const unsigned RadiusValueAcctStatusTypeAccountingOff    = 8;
const unsigned RadiusValueAcctStatusTypeTunnelStart      = 9;
const unsigned RadiusValueAcctStatusTypeTunnelStop       = 10;
const unsigned RadiusValueAcctStatusTypeTunnelReject     = 11;
const unsigned RadiusValueAcctStatusTypeTunnelLinkStart  = 12;
const unsigned RadiusValueAcctStatusTypeTunnelLinkStop   = 13;
const unsigned RadiusValueAcctStatusTypeTunnelLinkReject = 14;
const unsigned RadiusValueAcctStatusTypeFailed           = 15;

// Well know RADIUS vendors
const unsigned RadiusVendorCisco                         = 9;
const unsigned RadiusVendorMicrosoft                     = 311;
const unsigned RadiusVendorBreezecom                     = 710;
const unsigned RadiusVendorNortelAptis                   = 2637;
const unsigned RadiusVendorOpenSystemConsultants         = 9048;

// Well known Cisco VSAs
const unsigned RadiusVendorCiscoAttrCiscoAvpair          = 1;

// Well known Microsoft VSAs
const unsigned RadiusVendorMicrosoftAttrMSCHAPResponse   = 1;
const unsigned RadiusVendorMicrosoftAttrMSCHAPChallenge  = 11;
const unsigned RadiusVendorMicrosoftAttrMSCHAPMPPEKeys   = 12;
const unsigned RadiusVendorMicrosoftAttrMSMPPESendKey    = 16;
const unsigned RadiusVendorMicrosoftAttrMSMPPERecvKey    = 17;
const unsigned RadiusVendorMicrosoftAttrMSCHAP2Response  = 25;


/////////////////////////////////////////////////////////////////////
/// \struct RadiusPacket
/// Helper class for mapping RADIUS request packets, including the header
/// See RFC 2138
typedef struct {
    /// The RADIUS message type code
    uint8_t code;

    /// RADIUS identifier
    uint8_t identifier;

    /// Total length of the packet, including the header
    uint16_t length;

    /// RADISU authenticator
    uint8_t authenticator[RADIUS_AUTHENTICATOR_LENGTH];

    /// All attributes in serial packed format
    uint8_t attrs[RADIUS_MAX_SIZE - RADIUS_HEADER_LENGTH];

} RadiusPacket;

/////////////////////////////////////////////////////////////////////
/// \struct RadiusAttrHeader
/// Maps a RADIUS Attribute, including the header
typedef struct {
    /// Attribute number
    uint8_t type;

    /// Total length, including header
    uint8_t length;

    /// Value of the attribute. May have some internal structure, such as for VSAs etc
    uint8_t value[RADIUS_MAX_ATTRIBUTE_SIZE];

} RadiusAttrHeader;

/////////////////////////////////////////////////////////////////////
/// \class RadiusMsg RadiusMsg.h <RadiusMsg.h>
/// \brief Class to create, format and send RADIUS requests and replies
///
/// This class is used in conjunction with UDPSocket to create, format and send RADIUS requests,
/// and to receive, authenticate and decode RADIUS replies. Works with the Arduino Ethernet shield
/// to connect to a LAN and communicate with a RDAIUS server, such as Radiator RADIUS Server
/// (http://www.airspayce.com/radiator)
///
/// Conforms broadly to RFC 2138 and 2139, with limitations:
/// \li Vendor Specific Attribautes are not supported
/// \li The only encrypted attribtute supported is User-Password
///
/// There is no RADIUS dictionary: When adding attributes to a reque or getting attriburtes
/// from a reply, you are required to use the appropriate calls according to the attribute
/// type of the attribute you are using: binary, string or integer
class RadiusMsg {
  private:
    /// The formatted RADIUS packet, including header
    RadiusPacket packet;

    /// The number of valid bytes in the packet, including the header, Min 20
    uint16_t packetLength;

    /// Number of retries for retransmission
    uint8_t retries;

    ///  retransmission timeout in seconds
    uint8_t timeout;

    /// The IP address of the peer destination address (when sent) or the sender address when received
    IPAddress peerAddress;

    /// The port number of the peer
    uint16_t peerPort;

  public:
    /// Constructor for receiving
    RadiusMsg();

    /// Constructor for sending. RADIUS message type code is initialised
    RadiusMsg(RadiusCode code) : RadiusMsg(code, 3, 5) {};
    RadiusMsg(RadiusCode code, uint8_t timeout, uint8_t retries);

    /// Return the RADIUS message type code
    /// \return RADIUS message type code
    uint8_t code();

    /// Add an attribute to the request, binary octets
    /// \param[in] type The RADIUS attribute number
    /// \param[in] vendor The vendor number of the attribue (unused, set to 0)
    /// \param[in] value Pointer to the octets of the value
    /// \param[in] length Number of octets in the value
    void addAttr(unsigned type, unsigned vendor, uint8_t* value, uint8_t length);

    /// Add a CString type attribute to the request
    /// \param[in] type The RADIUS attribute number
    /// \param[in] vendor The vendor number of the attribue (unused, set to 0)
    /// \param[in] value CString value to set. String up to (but not including) the first NUL
    /// are used to set th value
    void addAttr(unsigned type, unsigned vendor, const char* value);

    /// Add a 32 bit unsigned integer type to the request
    /// \param[in] type The RADIUS attribute number
    /// \param[in] vendor The vendor number of the attribue (unused, set to 0)
    /// \param[in] value 32 bit unsigned integer value
    void addAttr(unsigned type, unsigned vendor, uint32_t value);

    /// Get the nth attribute with matching attribute number (and optional vendor number)
    /// Skips over 'skip' attributes to get the 'skip'th matching attribute
    /// \param[in] type The RADIUS attribute number
    /// \param[in] vendor The vendor number of the attribue (unused, set to 0)
    /// \param[in] value Destination to copy the value to
    /// \param[in] length Caller sets this to the maximum permitted length available in value.
    /// if return is 1, up to length octets will be copied, and *length will be set to the actual
    /// number of octets copied.
    /// \param[in] skip Number of matching attributes to skip (defaults to 0,
    /// which means get the first matching one)
    /// \return true if a match was found and the value copied
    uint8_t getAttr(unsigned type, unsigned vendor, uint8_t* value, uint8_t* length, uint8_t skip = 0);

    /// Get the nth attribute with matching attribue number (and optional vendor number)
    /// as a 32 bit unsigned integer
    /// \param[in] type The RADIUS attribute number
    /// \param[in] vendor The vendor number of the attribue (unused, set to 0)
    /// \param[in] value Destination to copy the value to
    /// \param[in] skip Number of matching attributes to skip (defaults to 0,
    /// which means get the first matching one)
    /// \return true if a match was found and the value copied
    uint8_t getAttr(unsigned type, unsigned vendor, uint32_t* value, uint8_t skip = 0);

    /// Encrypts any parameters that require encryption, and sets the authethenticator
    /// for RADIUS codes that require it. Uses the shared secret for encryption and signing.
    /// \param[in] secret The RADIUS shared secret
    /// \param[in] secretLength Length of the secret in octets
    /// \param[in] original for RADIUS requests that are replies to an earlier request, this
    /// points to the original requerst, which is required to correctly set the authenticator in the reply.
    void sign(const char* secret, uint8_t secretLength, RadiusMsg* original = 0);

    /// Sends this RADIUS message on a UDP Socket
    /// \param[in] socket Instance of UDPSocket to use to send the message
    /// \param[in] peer IPV4Address of the destination RADIUS peer
    /// \param[in] port Port number of the destination RADIUS peer
    /// \return Returns the sent packet size for success, else -1
    uint16_t sendto(WiFiUDP* Udp, IPAddress peer, uint16_t port);

    /// Utility function for encryption passwords and other data in RADIUS RFC compliant fashion
    /// \param[in] data The data octets to encrypt
    /// \param[in] length Number of octets in data
    /// \param[in] secret The RADIUS shared secret
    /// \param[in] secretLength Length of the secret in octets
    /// \param[in] iv The intialisation vector
    void encryptPassword(uint8_t* data, uint8_t length, const char* secret, uint8_t secretLength, uint8_t* iv);

    /// Fill the packet data in the RadiusMsg with the next packet received on socket.
    /// Blocks until a packet is received. Packets that are received and which dont look
    /// vaguely like a RADIUS essage are discarded
    /// \param socket Pointer to the UDP socket to receive from
    /// \param[in] reply Pointer to a RadiusMsg which will be filled in with the reply
    /// \return The number of octets in the received message else 0 if the message was discarded
    uint16_t recv(WiFiUDP* Udp, RadiusMsg* reply);

    /// Send a message to the destiantion server, and wait for a matching reply.
    /// Implements timeouts and retries until a matching reply is received
    /// Non-matching RADIUS requests are silently discarded.
    /// Blocks until a satisfying reply is received or all retries are exhausted
    /// \param[in] socket Pointer to the UDP socket used to send and receive
    /// \param[in] server IPAddress of the destination server
    /// \param[in] port The port number of the RADIUS server at the destination
    /// \param[in] reply Pointer to a RadiusMsg which will be filled in with the reply (if any)
    /// \return true if the request was snetr and a matchin reply received
    uint8_t sendWaitReply(WiFiUDP* Udp, IPAddress server, uint16_t port, RadiusMsg* reply);

    /// Checks that the authenticator in the RadiusMsg is correct, and that therefore is
    /// verified as being from the expected peer. For RADIUS replies, requires the
    /// original request to be supplied.
    /// \param[in] secret The RADIUS shared secret
    /// \param[in] secretLength Length of the secret in octets
    /// \param[in] original When checking the authenticator of a RADIUS reply, this must point to the
    /// original request
    /// \return true if authenticator is correct.
    uint8_t checkAuthenticatorsWithOriginal(const char* secret, uint8_t secretLength, RadiusMsg* original);
};

#endif
