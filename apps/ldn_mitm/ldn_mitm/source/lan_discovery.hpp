#pragma once
#include <stratosphere.hpp>
#include "debug.hpp"
#include <memory>
#include <thread>
#include <array>
#include <mutex>
#include <unordered_map>
#include <stdint.h>
#include <unistd.h>
#include <cstring>
#include "ldn_types.hpp"
#include "lan_protocol.hpp"

enum class NodeStatus : u8 {
    Disconnected,
    Connect,
    Connected,
};

class LANDiscovery;

class LanStation : public Pollable {
    protected:
        friend class LANDiscovery;
        NodeInfo *nodeInfo;
        NodeStatus status;
        std::unique_ptr<TcpLanSocketBase> socket;
        int nodeId;
        LANDiscovery *discovery;
    public:
        LanStation(int nodeId, LANDiscovery *discovery)
            : nodeInfo(nullptr),
            status(NodeStatus::Disconnected),
            nodeId(nodeId),
            discovery(discovery)
        {}
        NodeStatus getStatus() const {
            return this->status;
        }
        void reset() {
            this->socket.reset();
            this->status = NodeStatus::Disconnected;
        };
        void link(int fd) {
            this->socket = std::make_unique<TcpLanSocketBase>(fd);
            this->status = NodeStatus::Connect;
        };
        int getFd() override {
            if (!this->socket) {
                return -1;
            }
            return this->socket->getFd();
        };
        int onRead() override;
        void onClose() override;
        int sendPacket(LANPacketType type, const void *data, size_t size) {
            if (!this->socket) {
                return -1;
            }
            return this->socket->sendPacket(type, data, size);
        };
        void overrideInfo() {
            bool connected = this->getStatus() == NodeStatus::Connected;
            this->nodeInfo->nodeId = this->nodeId;
            if (connected) {
                this->nodeInfo->isConnected = 1;
            } else {
                this->nodeInfo->isConnected = 0;
            }
        }
};

class LDUdpSocket : public UdpLanSocketBase, public Pollable {
    protected:
        struct MacHash {
            std::size_t operator() (const MacAddress &t) const {
                return *reinterpret_cast<const u32*>(t.raw + 2);
            }
        };
        virtual u32 getBroadcast() override;
        LANDiscovery *discovery;
    public:
        std::unordered_map<MacAddress, NetworkInfo, MacHash> scanResults;
    public:
        LDUdpSocket(int fd, LANDiscovery *discovery);
        int getFd() override {
            return UdpLanSocketBase::getFd();
        }
        int onRead() override;
        void onClose() override {
            LogFormat("LDUdpSocket::onClose");
        };
};

class LDTcpSocket : public TcpLanSocketBase, public Pollable {
    protected:
        LANDiscovery *discovery;
    public:
        LDTcpSocket(int fd, LANDiscovery *discovery) : TcpLanSocketBase(fd), discovery(discovery) {};
        int getFd() override {
            return TcpLanSocketBase::getFd();
        }
        int onRead() override;
        void onClose() override;
};

class LANDiscovery {
    public:
        static const int DefaultPort = 11452;
        static const char *FakeSsid;
        typedef std::function<int(LANPacketType, const void *, size_t)> ReplyFunc;
        typedef std::function<void()> LanEventFunc;
        static const LanEventFunc EmptyFunc;
    protected:
        friend class LDUdpSocket;
        friend class LDTcpSocket;
        friend class LanStation;
        // 0: udp 1: tcp 2: client
        HosMutex pollMutex;
        std::unique_ptr<LDUdpSocket> udp;
        std::unique_ptr<LDTcpSocket> tcp;
        std::array<LanStation, StationCountMax> stations;
        std::array<NodeLatestUpdate, NodeCountMax> nodeChanges;
        std::array<u8, NodeCountMax> nodeLastStates;
        static void Worker(void* args);
        bool stop;
        bool inited;
        NetworkInfo networkInfo;
        u16 listenPort;
        HosThread workerThread;
        CommState state;
        void worker();
        int loopPoll();
        void onSyncNetwork(NetworkInfo *info);
        void onConnect(int new_fd);
        void onDisconnectFromHost();
        void onNetworkInfoChanged();

        void updateNodes();
        void resetStations();
        Result getFakeMac(MacAddress *mac);
        Result getNodeInfo(NodeInfo *node, const UserConfig *userConfig, u16 localCommunicationVersion);
        LanEventFunc lanEvent;
    public:
        Result initialize(LanEventFunc lanEvent = EmptyFunc, bool listening = true);
        Result finalize();
        Result initNetworkInfo();
        Result scan(NetworkInfo *networkInfo, u16 *count, ScanFilter filter);
        Result setAdvertiseData(const u8 *data, uint16_t size);
        Result createNetwork(const SecurityConfig *securityConfig, const UserConfig *userConfig, const NetworkConfig *networkConfig);
        Result destroyNetwork();
        Result connect(NetworkInfo *networkInfo, UserConfig *userConfig, u16 localCommunicationVersion);
        Result disconnect();
        Result getNetworkInfo(NetworkInfo *pOutNetwork);
        Result getNetworkInfo(NetworkInfo *pOutNetwork, NodeLatestUpdate *pOutUpdates, int bufferCount);
        Result openAccessPoint();
        Result closeAccessPoint();
        Result openStation();
        Result closeStation();
    public:
        LANDiscovery(u16 port = DefaultPort) :
            stations({{{1, this}, {2, this}, {3, this}, {4, this}, {5, this}, {6, this}, {7, this}}}),
            stop(false), inited(false),
            networkInfo({0}), listenPort(port),
            state(CommState::None) {
            LogFormat("LANDiscovery");
        };
        ~LANDiscovery();
        u16 getListenPort() const {
            return this->listenPort;
        }
        CommState getState() const {
            return this->state;
        };
        void setState(CommState v) {
            this->state = v;
            this->lanEvent();
        };
        int stationCount();
    protected:
        Result setSocketOpts(int fd);
        Result initTcp(bool listening);
        Result initUdp(bool listening);
        void initNodeStateChange();
        bool isNodeStateChanged();
};
