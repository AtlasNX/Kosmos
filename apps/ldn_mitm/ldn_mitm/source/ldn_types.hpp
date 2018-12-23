#pragma once
#include <stdint.h>
#include <stdlib.h>

const size_t SsidLengthMax = 32;
const size_t AdvertiseDataSizeMax = 384;
const size_t UserNameBytesMax = 32;
const int NodeCountMax = 8;
const int StationCountMax = NodeCountMax - 1;
const size_t  PassphraseLengthMax = 64;

enum NodeStateChange { 
    NodeStateChange_None = 0, 
    NodeStateChange_Connect = 1, 
    NodeStateChange_Disconnect = 2, 
    NodeStateChange_DisconnectAndConnect = 3 
};
enum ScanFilterFlag {
    ScanFilterFlag_LocalCommunicationId = 1 << 0,
    ScanFilterFlag_SessionId = 1 << 1,
    ScanFilterFlag_NetworkType = 1 << 2,
    ScanFilterFlag_Ssid = 1 << 4,
    ScanFilterFlag_SceneId = 1 << 5,
    ScanFilterFlag_IntentId = ScanFilterFlag_LocalCommunicationId | ScanFilterFlag_SceneId,
    ScanFilterFlag_NetworkId = ScanFilterFlag_IntentId | ScanFilterFlag_SessionId
};
enum class CommState {
    None,
    Initialized,
    AccessPoint,
    AccessPointCreated,
    Station,
    StationConnected,
    Error
};

struct MacAddress {
    uint8_t raw[6];
    bool operator==(const MacAddress& b) const;
};

struct Ssid {
    uint8_t length;
    char raw[SsidLengthMax + 1];
    bool operator==(const Ssid& b) const;
    Ssid& operator=(const char* s);
};

struct CommonNetworkInfo {
    MacAddress bssid;
    Ssid ssid;
    int16_t channel;
    int8_t linkLevel;
    uint8_t networkType;
    uint32_t _unk;
};

struct NodeInfo {
    uint32_t ipv4Address;
    MacAddress macAddress;
    int8_t nodeId;
    int8_t isConnected;
    char userName[UserNameBytesMax+1];
    uint8_t _unk1;
    int16_t localCommunicationVersion;
    uint8_t _unk2[16];
};

struct LdnNetworkInfo {
    uint8_t unkRandom[16];
    uint16_t securityMode;
    uint8_t stationAcceptPolicy;
    uint8_t _unk1[3];
    uint8_t nodeCountMax;
    uint8_t nodeCount;
    NodeInfo nodes[NodeCountMax];
    uint16_t _unk2;
    uint16_t advertiseDataSize;
    uint8_t advertiseData[AdvertiseDataSizeMax];
    uint8_t _unk3[148];
};

struct IntentId {
    uint64_t localCommunicationId;
    uint8_t _unk1[2];
    uint16_t sceneId;
    uint8_t _unk2[4];
};

struct SessionId {
    uint64_t high;
    uint64_t low;
    bool operator==(const SessionId& b) const;
};

struct NetworkId {
    IntentId intentId;      // 16bytes
    SessionId sessionId;    // 16bytes
};                // 32bytes

struct NetworkInfo {
    NetworkId networkId;
    CommonNetworkInfo common;
    LdnNetworkInfo ldn;
};

struct SecurityConfig {
    uint16_t securityMode;
    uint16_t passphraseSize;
    uint8_t passphrase[PassphraseLengthMax];
};

struct UserConfig {
    char userName[UserNameBytesMax + 1];
    uint8_t _unk[15];
};

struct NetworkConfig {
    IntentId intentId;      // 16byte
    uint16_t channel;
    uint8_t nodeCountMax;
    uint8_t _unk1;
    uint16_t localCommunicationVersion;
    uint8_t _unk2[10];
};            // 32bytes

struct CreateNetworkConfig {
    SecurityConfig securityConfig;
    UserConfig userConfig;
    uint8_t _unk[4];
    NetworkConfig networkConfig;
};

struct ConnectNetworkData {
    SecurityConfig securityConfig;
    UserConfig userConfig;
    uint32_t localCommunicationVersion;
    uint32_t option;
};

struct NodeLatestUpdate {
    uint8_t stateChange;
    uint8_t _unk[7];
};

struct SecurityParameter {
    uint8_t unkRandom[16];
    SessionId sessionId;
};

struct ScanFilter {
    NetworkId networkId;
    uint32_t networkType;
    MacAddress bssid;
    Ssid ssid;
    uint8_t unk[16];
    uint32_t flag;
};

void NetworkInfo2NetworkConfig(NetworkInfo* info, NetworkConfig* out);
void NetworkInfo2SecurityParameter(NetworkInfo* info, SecurityParameter* out);
