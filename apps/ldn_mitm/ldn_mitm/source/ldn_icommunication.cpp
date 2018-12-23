#include "ldn_icommunication.hpp"

static_assert(sizeof(NetworkInfo) == 0x480, "sizeof(NetworkInfo) should be 0x480");
static_assert(sizeof(ConnectNetworkData) == 0x7C, "sizeof(ConnectNetworkData) should be 0x7C");
static_assert(sizeof(ScanFilter) == 0x60, "sizeof(ScanFilter) should be 0x60");

// https://reswitched.github.io/SwIPC/ifaces.html#nn::ldn::detail::IUserLocalCommunicationService

Result ICommunicationInterface::Initialize(u64 unk, PidDescriptor pid) {
    Result rc = 0;

    LogFormat("ICommunicationInterface::Initialize unk: %" PRIu64 " pid: %" PRIu64, unk, pid.pid);

    if (this->state_event == nullptr) {
        this->state_event = CreateWriteOnlySystemEvent();
    }

    rc = lanDiscovery.initialize([&](){
        this->onEventFired();
    });
    if (R_FAILED(rc)) {
        return rc;
    }

    return rc;
}

Result ICommunicationInterface::Finalize() {
    if (this->state_event) {
        delete this->state_event;
        this->state_event = nullptr;
    }

    return lanDiscovery.finalize();
}

Result ICommunicationInterface::OpenAccessPoint() {
    return this->lanDiscovery.openAccessPoint();
}

Result ICommunicationInterface::CloseAccessPoint() {
    return this->lanDiscovery.closeAccessPoint();
}

Result ICommunicationInterface::DestroyNetwork() {
    return this->lanDiscovery.destroyNetwork();
}

Result ICommunicationInterface::OpenStation() {
    return this->lanDiscovery.openStation();
}

Result ICommunicationInterface::CloseStation() {
    return this->lanDiscovery.closeStation();
}

Result ICommunicationInterface::Disconnect() {
    return this->lanDiscovery.disconnect();
}

Result ICommunicationInterface::CreateNetwork(CreateNetworkConfig data) {
    return this->lanDiscovery.createNetwork(&data.securityConfig, &data.userConfig, &data.networkConfig);;
}

Result ICommunicationInterface::SetAdvertiseData(InSmartBuffer<u8> data) {
    return lanDiscovery.setAdvertiseData(data.buffer, data.num_elements);
}

Result ICommunicationInterface::SetStationAcceptPolicy(u8 policy) {
    return 0;
}

Result ICommunicationInterface::SetWirelessControllerRestriction() {
    return 0;
}

Result ICommunicationInterface::GetState(Out<u32> state) {
    state.SetValue(static_cast<u32>(this->lanDiscovery.getState()));

    return 0;
}

Result ICommunicationInterface::GetIpv4Address(Out<u32> address, Out<u32> netmask) {
    Result rc = ipinfoGetIpConfig(address.GetPointer(), netmask.GetPointer());

    LogFormat("get_ipv4_address %x %x", address.GetValue(), netmask.GetValue());

    return rc;
}

Result ICommunicationInterface::GetNetworkInfo(OutPointerWithServerSize<NetworkInfo, 1> buffer) {
    LogFormat("get_network_info %p %" PRIu64 " state: %d", buffer.pointer, buffer.num_elements, static_cast<u32>(this->lanDiscovery.getState()));

    return lanDiscovery.getNetworkInfo(buffer.pointer);
}

Result ICommunicationInterface::GetDisconnectReason(Out<u32> reason) {
    reason.SetValue(0);

    return 0;
}

Result ICommunicationInterface::GetNetworkInfoLatestUpdate(OutPointerWithServerSize<NetworkInfo, 1> buffer, OutPointerWithClientSize<NodeLatestUpdate> pUpdates) {
    LogFormat("get_network_info_latest buffer %p %" PRIu64, buffer.pointer, buffer.num_elements);
    LogFormat("get_network_info_latest pUpdates %p %" PRIu64, pUpdates.pointer, pUpdates.num_elements);

    return lanDiscovery.getNetworkInfo(buffer.pointer, pUpdates.pointer, pUpdates.num_elements);
}

Result ICommunicationInterface::GetSecurityParameter(Out<SecurityParameter> out) {
    Result rc = 0;

    SecurityParameter data;
    NetworkInfo info;
    rc = lanDiscovery.getNetworkInfo(&info);
    if (R_SUCCEEDED(rc)) {
        NetworkInfo2SecurityParameter(&info, &data);
        out.SetValue(data);
    }

    return rc;
}

Result ICommunicationInterface::GetNetworkConfig(Out<NetworkConfig> out) {
    Result rc = 0;

    NetworkConfig data;
    NetworkInfo info;
    rc = lanDiscovery.getNetworkInfo(&info);
    if (R_SUCCEEDED(rc)) {
        NetworkInfo2NetworkConfig(&info, &data);
        out.SetValue(data);
    }

    return rc;
}

Result ICommunicationInterface::AttachStateChangeEvent(Out<CopiedHandle> handle) {
    handle.SetValue(this->state_event->GetHandle());
    return 0;
}

Result ICommunicationInterface::Scan(Out<u32> outCount, OutSmartBuffer<NetworkInfo> buffer, u16 channel, ScanFilter filter) {
    Result rc = 0;
    u16 count = buffer.num_elements;

    rc = lanDiscovery.scan(buffer.buffer, &count, filter);
    outCount.SetValue(count);

    LogFormat("scan %d %d", count, rc);

    return rc;
}

Result ICommunicationInterface::Connect(ConnectNetworkData param, InPointer<NetworkInfo> data) {
    LogFormat("ICommunicationInterface::connect %" PRIu64, data.num_elements);
    LogHex(data.pointer, sizeof(NetworkInfo));
    LogHex(&param, sizeof(param));

    return lanDiscovery.connect(data.pointer, &param.userConfig, param.localCommunicationVersion);
}

void ICommunicationInterface::onEventFired() {
    if (this->state_event) {
        LogFormat("onEventFired signal_event");
        this->state_event->Signal();
    }
}
