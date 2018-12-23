#include "lan_discovery.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include "ipinfo.hpp"

static const int ModuleID = 0xFD;
static const int LdnModuleId = 0xCB;

const char *LANDiscovery::FakeSsid = "12345678123456781234567812345678";
const LANDiscovery::LanEventFunc LANDiscovery::EmptyFunc = [](){};


int LanStation::onRead() {
    if (!this->socket) {
        LogFormat("Nullptr %d\n", this->nodeId);
        return -1;
    }
    return this->socket->recvPacket([&](LANPacketType type, const void *data, size_t size, ReplyFunc reply) -> int {
        if (type == LANPacketType::Connect) {
            LogFormat("on connect");
            NodeInfo *info = (decltype(info))data;
            if (size != sizeof(*info)) {
                LogFormat("NodeInfo size is wrong");
                return -1;
            }
            *this->nodeInfo = *info;
            this->status = NodeStatus::Connected;

            this->discovery->updateNodes();
        } else {
            LogFormat("unexpecting type %d", static_cast<int>(type));
        }
        return 0;
    });
}

void LanStation::onClose() {
    LogFormat("LanStation::onClose %d", this->nodeId);
    this->reset();
    this->discovery->updateNodes();
}

LDUdpSocket::LDUdpSocket(int fd, LANDiscovery *discovery)
    :   UdpLanSocketBase(fd, discovery->getListenPort()),
        discovery(discovery) {
    /* ... */
}

int LDUdpSocket::onRead() {
    LogFormat("LDUdpSocket::onRead");
    return this->recvPacket([&](LANPacketType type, const void *data, size_t size, ReplyFunc reply) -> int {
        switch (type) {
            case LANPacketType::Scan: {
                if (this->discovery->getState() == CommState::AccessPointCreated) {
                    reply(LANPacketType::ScanResp, &this->discovery->networkInfo, sizeof(NetworkInfo));
                }
                break;
            }
            case LANPacketType::ScanResp: {
                LogFormat("ScanResp");
                NetworkInfo *info = (decltype(info))data;
                if (size != sizeof(*info)) {
                    break;
                }
                this->scanResults.insert({info->common.bssid, *info});
                break;
            }
            default: {
                LogFormat("LDUdpSocket::onRead unhandle type %d", static_cast<int>(type));
                break;
            }
        }
        return 0;
    });
}

int LDTcpSocket::onRead() {
    LogFormat("LDTcpSocket::onRead");
    const auto state = this->discovery->getState();
    if (state == CommState::Station || state == CommState::StationConnected) {
        return this->recvPacket([&](LANPacketType type, const void *data, size_t size, ReplyFunc reply) -> int {
            if (type == LANPacketType::SyncNetwork) {
                LogFormat("SyncNetwork");
                NetworkInfo *info = (decltype(info))data;
                if (size != sizeof(*info)) {
                    return -1;
                }

                this->discovery->onSyncNetwork(info);
            } else {
                LogFormat("LDTcpSocket::onRead unhandle type %d", static_cast<int>(type));
                return -1;
            }

            return 0;
        });
    } else if (state == CommState::AccessPointCreated) {
        struct sockaddr_in addr;
        socklen_t addrlen = sizeof(addr);
        int new_fd = accept(this->getFd(), (struct sockaddr *)&addr, &addrlen);
        if (new_fd < 0)
        {
            LogFormat("accept failed");
            return -1;
        }
        this->discovery->onConnect(new_fd);
        return 0;
    } else {
        LogFormat("LDTcpSocket::onRead wrong state %d", static_cast<int>(state));
        return -1;
    }
}

void LDTcpSocket::onClose() {
    LogFormat("LDTcpSocket::onClose");
    this->discovery->onDisconnectFromHost();
}

u32 LDUdpSocket::getBroadcast() {
    u32 address;
    u32 netmask;
    Result rc = ipinfoGetIpConfig(&address, &netmask);
    if (R_FAILED(rc)) {
        LogFormat("Broadcast failed to get ip");
        return 0xFFFFFFFF;
    }
    u32 ret = address | ~netmask;
    return ret;
}

void LANDiscovery::onSyncNetwork(NetworkInfo *info) {
    this->networkInfo = *info;
    if (this->state == CommState::Station) {
        this->setState(CommState::StationConnected);
    }
    this->onNetworkInfoChanged();
}

void LANDiscovery::onConnect(int new_fd) {
    LogFormat("Accepted %d", new_fd);
    if (this->stationCount() >= StationCountMax) {
        LogFormat("Close new_fd. stations are full");
        close(new_fd);
        return;
    }

    bool found = false;
    for (auto &i : this->stations) {
        if (i.getStatus() == NodeStatus::Disconnected) {
            i.link(new_fd);
            found = true;
            break;
        }
    }

    if (!found) {
        LogFormat("Close new_fd. no free station found");
        close(new_fd);
    }
}

void LANDiscovery::onDisconnectFromHost() {
    LogFormat("onDisconnectFromHost state: %d", static_cast<int>(this->state));
    if (this->state == CommState::StationConnected) {
        this->setState(CommState::Station);
    }
}

void LANDiscovery::onNetworkInfoChanged() {
    if (this->isNodeStateChanged()) {
        this->lanEvent();
    }
    return;
}

Result LANDiscovery::setAdvertiseData(const u8 *data, uint16_t size) {
    if (size > AdvertiseDataSizeMax) {
        return MAKERESULT(ModuleID, 10);
    }

    if (size > 0 && data != nullptr) {
        std::memcpy(this->networkInfo.ldn.advertiseData, data, size);
    } else {
        LogFormat("LANDiscovery::setAdvertiseData data %p size %lu", data, size);
    }
    this->networkInfo.ldn.advertiseDataSize = size;

    this->updateNodes();

    return 0;
}

Result LANDiscovery::initNetworkInfo() {
    Result rc = getFakeMac(&this->networkInfo.common.bssid);
    if (R_FAILED(rc)) {
        return rc;
    }
    this->networkInfo.common.channel = 6;
    this->networkInfo.common.linkLevel = 3;
    this->networkInfo.common.networkType = 2;
    this->networkInfo.common.ssid = FakeSsid;

    auto nodes = this->networkInfo.ldn.nodes;
    for (int i = 0; i < NodeCountMax; i++) {
        nodes[i].nodeId = i;
        nodes[i].isConnected = 0;
    }

    return 0;
}

Result LANDiscovery::getFakeMac(MacAddress *mac) {
    mac->raw[0] = 0x02;
    mac->raw[1] = 0x00;

    u32 ip;
    Result rc = ipinfoGetIpConfig(&ip);
    if (R_SUCCEEDED(rc)) {
        memcpy(mac->raw + 2, &ip, sizeof(ip));
    }

    return rc;
}

Result LANDiscovery::setSocketOpts(int fd) {
    int rc;

    {
        int b = 1;
        rc = setsockopt(fd, SOL_SOCKET, SO_BROADCAST, &b, sizeof(b));
        if (rc != 0) {
            return MAKERESULT(ModuleID, 4);
        }
    }
    {
        int yes = 1;
        rc = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
        if (rc != 0) {
            // return MAKERESULT(ModuleID, 5);
            LogFormat("SO_REUSEADDR failed");
        }
    }

    return 0;
}

Result LANDiscovery::initTcp(bool listening) {
    int fd;
    int rc;
    struct sockaddr_in addr;

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        return MAKERESULT(ModuleID, 6);
    }
    auto tcpSocket = std::make_unique<LDTcpSocket>(fd, this);

    if (listening) {
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htons(INADDR_ANY);
        addr.sin_port = htons(listenPort);
        if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
            return MAKERESULT(ModuleID, 7);
        }
        if (listen(fd, 10) != 0) {
            return MAKERESULT(ModuleID, 8);
        }
    }
    rc = setSocketOpts(fd);
    if (R_FAILED(rc)) {
        return rc;
    }

    {
        std::scoped_lock<HosMutex> lock(this->pollMutex);
        this->tcp = std::move(tcpSocket);
    }

    return 0;
}

Result LANDiscovery::initUdp(bool listening) {
    int fd;
    int rc;
    struct sockaddr_in addr;

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        return MAKERESULT(ModuleID, 1);
    }
    auto udpSocket = std::make_unique<LDUdpSocket>(fd, this);

    if (listening) {
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htons(INADDR_ANY);
        addr.sin_port = htons(listenPort);
        if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
            return MAKERESULT(ModuleID, 2);
        }
    }
    rc = setSocketOpts(fd);
    if (R_FAILED(rc)) {
        return rc;
    }

    {
        std::scoped_lock<HosMutex> lock(this->pollMutex);
        this->udp = std::move(udpSocket);
    }

    return 0;
}

void LANDiscovery::initNodeStateChange() {
    for (auto &i : this->nodeChanges) {
        i.stateChange = NodeStateChange_None;
    }
    for (auto &i : this->nodeLastStates) {
        i = 0;
    }
}

bool LANDiscovery::isNodeStateChanged() {
    bool changed = false;
    const auto &nodes = this->networkInfo.ldn.nodes;
    for (int i = 0; i < NodeCountMax; i++) {
        if (nodes[i].isConnected != this->nodeLastStates[i]) {
            if (nodes[i].isConnected) {
                this->nodeChanges[i].stateChange |= NodeStateChange_Connect;
            } else {
                this->nodeChanges[i].stateChange |= NodeStateChange_Disconnect;
            }
            this->nodeLastStates[i] = nodes[i].isConnected;
            changed = true;
        }
    }
    return changed;
}

void LANDiscovery::Worker(void* args) {
    LANDiscovery* self = (LANDiscovery*)args;

    self->worker();
}

Result LANDiscovery::scan(NetworkInfo *pOutNetwork, u16 *count, ScanFilter filter) {
    this->udp->scanResults.clear();

    int len = this->udp->sendBroadcast(LANPacketType::Scan);
    if (len < 0) {
        return MAKERESULT(ModuleID, 20);
    }

    svcSleepThread(1000000000L); // 1sec

    int i = 0;
    for (auto& item : this->udp->scanResults) {
        if (i >= *count) {
            break;
        }
        auto &info = item.second;

        bool copy = true;
        // filter
        if (filter.flag & ScanFilterFlag_LocalCommunicationId) {
            copy &= filter.networkId.intentId.localCommunicationId == info.networkId.intentId.localCommunicationId;
        }
        if (filter.flag & ScanFilterFlag_SessionId) {
            copy &= filter.networkId.sessionId == info.networkId.sessionId;
        }
        if (filter.flag & ScanFilterFlag_NetworkType) {
            copy &= filter.networkType == info.common.networkType;
        }
        if (filter.flag & ScanFilterFlag_Ssid) {
            copy &= filter.ssid == info.common.ssid;
        }
        if (filter.flag & ScanFilterFlag_SceneId) {
            copy &= filter.networkId.intentId.sceneId == info.networkId.intentId.sceneId;
        }

        if (copy) {
            pOutNetwork[i++] = info;
        }
    }
    *count = i;

    return 0;
}

void LANDiscovery::resetStations() {
    for (auto &i : this->stations) {
        i.reset();
    }
}

int LANDiscovery::stationCount() {
    int count = 0;

    for (auto const &i : this->stations) {
        if (i.getStatus() != NodeStatus::Disconnected) {
            count++;
        }
    }

    return count;
}

void LANDiscovery::updateNodes() {
    int count = 0;
    for (auto &i : this->stations) {
        bool connected = i.getStatus() == NodeStatus::Connected;
        if (connected) {
            count++;
        }
        i.overrideInfo();
    }
    this->networkInfo.ldn.nodeCount = count + 1;

    for (auto &i : stations) {
        if (i.getStatus() == NodeStatus::Connected) {
            int ret = i.sendPacket(LANPacketType::SyncNetwork, &this->networkInfo, sizeof(this->networkInfo));
            if (ret < 0) {
                LogFormat("Failed to sendTcp");
            }
        }
    }

    this->onNetworkInfoChanged();
}

int LANDiscovery::loopPoll() {
    int rc;
    if (!inited) {
        return 0;
    }

    std::scoped_lock<HosMutex> lock(this->pollMutex);
    int nfds = 2 + StationCountMax;
    Pollable *fds[nfds];
    fds[0] = this->udp.get();
    fds[1] = this->tcp.get();
    for (int i = 0; i < StationCountMax; i++) {
        fds[2 + i] = this->stations.data() + i;
    }
    rc = Pollable::Poll(fds, nfds);

    return rc;
}

LANDiscovery::~LANDiscovery() {
    LogFormat("~LANDiscovery");
}

void LANDiscovery::worker() {
    this->stop = false;
    while (!this->stop) {

        int rc = loopPoll();
        if (rc < 0) {
            break;
        }
        svcSleepThread(0);
    }
    LogFormat("Worker exit");
    svcExitThread();
}

Result LANDiscovery::getNetworkInfo(NetworkInfo *pOutNetwork) {
    Result rc = 0;

    if (this->state == CommState::AccessPointCreated || this->state == CommState::StationConnected) {
        std::memcpy(pOutNetwork, &networkInfo, sizeof(networkInfo));
    } else {
        rc = MAKERESULT(LdnModuleId, 32);
    }

    return rc;
}

Result LANDiscovery::getNetworkInfo(NetworkInfo *pOutNetwork, NodeLatestUpdate *pOutUpdates, int bufferCount) {
    Result rc = 0;

    if (bufferCount < 0 || bufferCount > NodeCountMax) {
        return MAKERESULT(ModuleID, 50);
    }

    if (this->state == CommState::AccessPointCreated || this->state == CommState::StationConnected) {
        std::memcpy(pOutNetwork, &networkInfo, sizeof(networkInfo));

        char str[10] = {0};
        for (int i = 0; i < bufferCount; i++) {
            pOutUpdates[i].stateChange = nodeChanges[i].stateChange;
            nodeChanges[i].stateChange = NodeStateChange_None;
            str[i] = '0' + pOutUpdates[i].stateChange;
        }
        LogFormat("getNetworkInfo updates %s", str);
    } else {
        rc = MAKERESULT(LdnModuleId, 32);
    }

    return rc;
}

Result LANDiscovery::getNodeInfo(NodeInfo *node, const UserConfig *userConfig, u16 localCommunicationVersion) {
    u32 ipAddress;
    Result rc = ipinfoGetIpConfig(&ipAddress);
    if (R_FAILED(rc)) {
        return rc;
    }
    rc = getFakeMac(&node->macAddress);
    if (R_FAILED(rc)) {
        return rc;
    }

    node->isConnected = 1;
    strcpy(node->userName, userConfig->userName);
    node->localCommunicationVersion = localCommunicationVersion;
    node->ipv4Address = ipAddress;

    return 0;
}

Result LANDiscovery::createNetwork(const SecurityConfig *securityConfig, const UserConfig *userConfig, const NetworkConfig *networkConfig) {
    Result rc = 0;

    if (this->state != CommState::AccessPoint) {
        return MAKERESULT(LdnModuleId, 32);
    }

    rc = this->initTcp(true);
    if (R_FAILED(rc)) {
        return rc;
    }
    rc = this->initNetworkInfo();
    if (R_FAILED(rc)) {
        return rc;
    }
    this->networkInfo.ldn.nodeCountMax = networkConfig->nodeCountMax;
    this->networkInfo.ldn.securityMode = securityConfig->securityMode;

    if (networkConfig->channel == 0) {
        this->networkInfo.common.channel = 6;
    } else {
        this->networkInfo.common.channel = networkConfig->channel;
    }
    this->networkInfo.networkId.intentId = networkConfig->intentId;

    NodeInfo *node0 = &this->networkInfo.ldn.nodes[0];
    rc = this->getNodeInfo(node0, userConfig, networkConfig->localCommunicationVersion);
    if (R_FAILED(rc)) {
        return rc;
    }
    
    this->setState(CommState::AccessPointCreated);

    this->initNodeStateChange();
    node0->isConnected = 1;
    this->updateNodes();

    return rc;
}

Result LANDiscovery::destroyNetwork() {
    if (this->tcp) {
        this->tcp->close();
    }
    this->resetStations();

    this->setState(CommState::AccessPoint);

    return 0;
}

Result LANDiscovery::disconnect() {
    if (this->tcp) {
        this->tcp->close();
    }
    this->setState(CommState::Station);

    return 0;
}

Result LANDiscovery::openAccessPoint() {
    if (this->state == CommState::None) {
        return MAKERESULT(LdnModuleId, 32);
    }

    if (this->tcp) {
        this->tcp->close();
    }
    this->resetStations();

    this->setState(CommState::AccessPoint);

    return 0;
}

Result LANDiscovery::closeAccessPoint() {
    if (this->state == CommState::None) {
        return MAKERESULT(LdnModuleId, 32);
    }

    if (this->tcp) {
        this->tcp->close();
    }
    this->resetStations();

    this->setState(CommState::Initialized);

    return 0;
}

Result LANDiscovery::openStation() {
    if (this->state == CommState::None) {
        return MAKERESULT(LdnModuleId, 32);
    }

    if (this->tcp) {
        this->tcp->close();
    }
    this->resetStations();

    this->setState(CommState::Station);

    return 0;
}

Result LANDiscovery::closeStation() {
    if (this->state == CommState::None) {
        return MAKERESULT(LdnModuleId, 32);
    }

    if (this->tcp) {
        this->tcp->close();
    }
    this->resetStations();

    this->setState(CommState::Initialized);

    return 0;
}

Result LANDiscovery::connect(NetworkInfo *networkInfo, UserConfig *userConfig, u16 localCommunicationVersion) {
    if (networkInfo->ldn.nodeCount == 0) {
        return MAKERESULT(ModuleID, 30);
    }

    u32 hostIp = networkInfo->ldn.nodes[0].ipv4Address;
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(hostIp);
    addr.sin_port = htons(listenPort);
    LogFormat("connect hostIp %x", hostIp);

    Result rc = this->initTcp(false);
    if (R_FAILED(rc)) {
        return rc;
    }

    int ret = ::connect(this->tcp->getFd(), (struct sockaddr *)&addr, sizeof(addr));
    if (ret != 0) {
        LogFormat("connect failed");
        return MAKERESULT(ModuleID, 31);
    }

    NodeInfo myNode = {0};
    rc = this->getNodeInfo(&myNode, userConfig, localCommunicationVersion);
    if (R_FAILED(rc)) {
        return rc;
    }
    ret = this->tcp->sendPacket(LANPacketType::Connect, &myNode, sizeof(myNode));
    if (ret < 0) {
        LogFormat("sendPacket failed");
        return MAKERESULT(ModuleID, 32);
    }
    this->initNodeStateChange();

    svcSleepThread(1000000000L); // 1sec

    return 0;
}

Result LANDiscovery::finalize() {
    if (this->inited) {
        this->stop = true;
        this->workerThread.Join();
        this->udp.reset();
        this->tcp.reset();
        this->resetStations();
        this->inited = false;
    }

    this->setState(CommState::None);

    return 0;
}

Result LANDiscovery::initialize(LanEventFunc lanEvent, bool listening) {
    if (this->inited) {
        return 0;
    }

    for (auto &i : stations) {
        i.discovery = this;
        i.nodeInfo = &this->networkInfo.ldn.nodes[i.nodeId];
        i.reset();
    }

    this->lanEvent = lanEvent;
    Result rc = initUdp(listening);
    if (R_FAILED(rc)) {
        LogFormat("initUdp %x", rc);
        return rc;
    }

    if (R_FAILED(this->workerThread.Initialize(&Worker, this, 0x4000, 0x15, 2))) {
        LogFormat("LANDiscovery Failed to threadCreate");
        return 0xF601;
    }
    if (R_FAILED(this->workerThread.Start())) {
        LogFormat("LANDiscovery Failed to threadStart");
        return 0xF601;
    }
    this->setState(CommState::Initialized);

    this->inited = true;
    return 0;
}
