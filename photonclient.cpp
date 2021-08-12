#include "photonclient.h"

#include <thread>

#include "fmt/core.h"
#include "fmt/xchar.h"

std::wstring_view VRCHAT_APPID = L"bf0942f7-9935-4192-b359-f092fa85bef1";
std::wstring_view VRCHAT_VERSION = L"Release_2018_server_1121_2.5";
std::wstring_view PHOTON_MAIN_NS = L"ns.exitgames.com";

namespace PhotonLB = ExitGames::LoadBalancing;

VRChad::PhotonClient::PhotonClient(std::wstring_view userId, std::wstring_view authToken, std::wstring_view hwid)
    : PhotonLB::Client(*this, VRCHAT_APPID.data(), VRCHAT_VERSION.data(), ExitGames::Photon::ConnectionProtocol::UDP, false, PhotonLB::RegionSelectionMode::SELECT, false)
    , PhotonLB::Listener()
    , m_photonThread()
    , m_authValues()
    , m_regions()
    , m_connectionState(ClientConnectionState::Disconnected)
    , m_userId(userId)
    , m_authToken(authToken)
    , m_hwid(hwid)
{
    m_photonThread = std::thread(&VRChad::PhotonClient::photonLoop, this);

    fmt::print("[Photon] Connecting to nameserver\n");

    m_connectionState = ClientConnectionState::ConnectingToNS;
    connect(m_authValues, L"", PHOTON_MAIN_NS.data(), ExitGames::LoadBalancing::ServerType::NAME_SERVER);
}

VRChad::PhotonClient::~PhotonClient()
{
    fmt::print("VRChad::VrcPhotonClient::~VrcPhotonClient()\n");
}

bool VRChad::PhotonClient::leaveRoom()
{
    return opLeaveRoom(false);
}

void VRChad::PhotonClient::photonLoop()
{
    while (true) {
        service();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        fflush(stdout);
    }
}

void VRChad::PhotonClient::onOperationResponse(const ExitGames::Photon::OperationResponse& operationResponse)
{
    Client::onOperationResponse(operationResponse); // Call base

    nByte opCode = operationResponse.getOperationCode();

    if (opCode != 220 && opCode != 230) {
        fmt::print("OperationResponse {}\n", opCode);
    }

    // TODO Parse parameters
}

void VRChad::PhotonClient::onStatusChanged(int statusCode)
{
    Client::onStatusChanged(statusCode); // Call base

    switch (statusCode) {
    case ExitGames::Photon::StatusCode::EXCEPTION_ON_CONNECT:
        fmt::print("[Photon] encountered an exception while opening the incoming connection to the server. The server could be down / not running\n");
        break;
    case ExitGames::Photon::StatusCode::CONNECT:
        fmt::print("[Photon] connected\n");
        break;
    case ExitGames::Photon::StatusCode::EXCEPTION:
        fmt::print("[Photon] encountered an exception and will disconnect, too\n");
        break;
    case ExitGames::Photon::StatusCode::QUEUE_OUTGOING_RELIABLE_WARNING:
        fmt::print("[Photon] outgoing queue is filling up. send more often\n");
        break;
    case ExitGames::Photon::StatusCode::QUEUE_OUTGOING_UNRELIABLE_WARNING:
        fmt::print("[Photon] outgoing queue is filling up. send more often\n");
        break;
    case ExitGames::Photon::StatusCode::SEND_ERROR:
        fmt::print("[Photon] Sending command failed. Either not connected, or the requested channel is bigger than the number of initialized channels\n");
        break;
    case ExitGames::Photon::StatusCode::QUEUE_OUTGOING_ACKS_WARNING:
        fmt::print("[Photon] outgoing queue is filling up. Send more often\n");
        break;
    case ExitGames::Photon::StatusCode::QUEUE_INCOMING_RELIABLE_WARNING:
        fmt::print("[Photon] incoming reliable queue is filling up. Dispatch more often\n");
        break;
    case ExitGames::Photon::StatusCode::QUEUE_INCOMING_UNRELIABLE_WARNING:
        fmt::print("[Photon] incoming unreliable queue is filling up. Dispatch more often\n");
        break;
    case ExitGames::Photon::StatusCode::QUEUE_SENT_WARNING:
        fmt::print("[Photon] sent queue is filling up. Check, why the server does not acknowledge your sent reliable commands\n");
        break;
    case ExitGames::Photon::StatusCode::INTERNAL_RECEIVE_EXCEPTION:
        fmt::print("[Photon] Exception, if a server cannot be connected. Most likely, the server is not responding. Ask the user to try again later\n");
        break;
    case ExitGames::Photon::StatusCode::TIMEOUT_DISCONNECT:
        fmt::print("[Photon] Disconnection due to a timeout (client did no longer receive ACKs from server)\n");
        break;
    case ExitGames::Photon::StatusCode::DISCONNECT_BY_SERVER:
        fmt::print("[Photon] Disconnect by server due to timeout (received a disconnect command, cause server misses ACKs of client)\n");
        break;
    case ExitGames::Photon::StatusCode::DISCONNECT_BY_SERVER_USER_LIMIT:
        fmt::print("[Photon] Disconnect by server due to concurrent user limit reached (received a disconnect command)\n");
        break;
    case ExitGames::Photon::StatusCode::DISCONNECT_BY_SERVER_LOGIC:
        fmt::print("[Photon] Disconnect by server due to server's logic (received a disconnect command)\n");
        break;
    case ExitGames::Photon::StatusCode::ENCRYPTION_ESTABLISHED:
        fmt::print("[Photon] Encryption success\n");
        break;
    case ExitGames::Photon::StatusCode::ENCRYPTION_FAILED_TO_ESTABLISH:
        fmt::print("[Photon] Encryption failed, Check debug logs\n");
        break;
    default:
        return;
    }
}

void VRChad::PhotonClient::onEvent(const ExitGames::Photon::EventData& eventData)
{
    Client::onEvent(eventData); // Call base

    fmt::print("VRChad::VrcPhotonClient::onEvent() {} {}\n", eventData.getCode());

    if (eventData.getCode() == 255) {
        fmt::print("User joined!\n");
    }
}

void VRChad::PhotonClient::onPingResponse(const ExitGames::Common::JString& address, unsigned int result)
{
    Client::onPingResponse(address, result); // Call base
    std::wstring_view stdAddress(address.cstr(), address.length());

    fmt::print(L"VRChad::VrcPhotonClient::debugReturn() {} {}\n", stdAddress, result);
}

void VRChad::PhotonClient::debugReturn(int debugLevel, const ExitGames::Common::JString& string)
{
    std::wstring_view stdString(string.cstr(), string.length());

    fmt::print(L"VRChad::VrcPhotonClient::debugReturn() {} {}\n", debugLevel, stdString);
}


void VRChad::PhotonClient::connectionErrorReturn(int errorCode)
{
    fmt::print("VRChad::VrcPhotonClient::connectionErrorReturn() {}\n", errorCode);
}

void VRChad::PhotonClient::clientErrorReturn(int errorCode)
{
    fmt::print("VRChad::VrcPhotonClient::clientErrorReturn() {}\n", errorCode);
}

void VRChad::PhotonClient::warningReturn(int warningCode)
{
    fmt::print("VRChad::VrcPhotonClient::warningReturn() {}\n", warningCode);
}

void VRChad::PhotonClient::serverErrorReturn(int errorCode)
{
    fmt::print("VRChad::VrcPhotonClient::serverErrorReturn() {}\n", errorCode);
}

void VRChad::PhotonClient::joinRoomEventAction(int playerNr, const ExitGames::Common::JVector<int>& playernrs, const ExitGames::LoadBalancing::Player& player)
{
    fmt::print("VRChad::VrcPhotonClient::joinRoomEventAction() {}\n", playerNr);
}

void VRChad::PhotonClient::leaveRoomEventAction(int playerNr, bool isInactive)
{
    fmt::print("VRChad::VrcPhotonClient::leaveRoomEventAction() {}\n", playerNr);
}

void VRChad::PhotonClient::customEventAction(int playerNr, nByte eventCode, const ExitGames::Common::Object& eventContent)
{
    fmt::print("VRChad::VrcPhotonClient::customEventAction() {}\n", playerNr);
}

void VRChad::PhotonClient::connectReturn(int errorCode, const ExitGames::Common::JString& errorString, const ExitGames::Common::JString& region, const ExitGames::Common::JString& cluster)
{
    fmt::print("VRChad::VrcPhotonClient::connectReturn() {}\n", errorCode);
}

void VRChad::PhotonClient::disconnectReturn()
{
    fmt::print("\n[Photon] Disconnected\n");
    if (m_connectionState == ClientConnectionState::ConnectingToMaster)
    {
        std::wstring_view addr = m_regions[m_selectedRegion];
        m_authParamsStr = fmt::format(L"token={}&user={}&hwid={}&platform=standalonewindows", m_authToken, m_userId, m_hwid);

        fmt::print(L"[Photon] Connecting to {} with params: {}\n\n", addr, m_authParamsStr);

        m_authValues.setType(ExitGames::LoadBalancing::CustomAuthenticationType::CUSTOM);
        m_authValues.setParameters(m_authParamsStr.c_str());

        connect(m_authValues, L"", addr.data(), ExitGames::LoadBalancing::ServerType::MASTER_SERVER);
    }
}

void VRChad::PhotonClient::createRoomReturn(int, const ExitGames::Common::Hashtable&, const ExitGames::Common::Hashtable&, int, const ExitGames::Common::JString&)
{
    fmt::print("VRChad::VrcPhotonClient::createRoomReturn()\n");
}

void VRChad::PhotonClient::joinOrCreateRoomReturn(int, const ExitGames::Common::Hashtable&, const ExitGames::Common::Hashtable&, int, const ExitGames::Common::JString&)
{
    fmt::print("VRChad::VrcPhotonClient::joinOrCreateRoomReturn()\n");
}

void VRChad::PhotonClient::joinRandomOrCreateRoomReturn(int, const ExitGames::Common::Hashtable&, const ExitGames::Common::Hashtable&, int, const ExitGames::Common::JString&)
{
    fmt::print("VRChad::VrcPhotonClient::joinRandomOrCreateRoomReturn()\n");
}

void VRChad::PhotonClient::joinRoomReturn(int, const ExitGames::Common::Hashtable&, const ExitGames::Common::Hashtable&, int, const ExitGames::Common::JString&)
{
    fmt::print("VRChad::VrcPhotonClient::joinRoomReturn()\n");
}

void VRChad::PhotonClient::joinRandomRoomReturn(int, const ExitGames::Common::Hashtable&, const ExitGames::Common::Hashtable&, int, const ExitGames::Common::JString&)
{
    fmt::print("VRChad::VrcPhotonClient::joinRandomRoomReturn()\n");
}

void VRChad::PhotonClient::leaveRoomReturn(int errorCode, const ExitGames::Common::JString& errorString)
{
    fmt::print("VRChad::VrcPhotonClient::leaveRoomReturn()\n");
}

void VRChad::PhotonClient::joinLobbyReturn()
{
    fmt::print("VRChad::VrcPhotonClient::joinLobbyReturn()\n");
}

void VRChad::PhotonClient::leaveLobbyReturn()
{
    fmt::print("VRChad::VrcPhotonClient::leaveLobbyReturn()\n");
}

void VRChad::PhotonClient::onFindFriendsResponse()
{
    fmt::print("VRChad::VrcPhotonClient::onFindFriendsResponse()\n");
}

void VRChad::PhotonClient::onLobbyStatsResponse(const ExitGames::Common::JVector<ExitGames::LoadBalancing::LobbyStatsResponse>&)
{
    fmt::print("VRChad::VrcPhotonClient::onLobbyStatsResponse()\n");
}

void VRChad::PhotonClient::webRpcReturn(int, const ExitGames::Common::JString&, const ExitGames::Common::JString&, int, const ExitGames::Common::Dictionary<ExitGames::Common::Object, ExitGames::Common::Object>&)
{
    fmt::print("VRChad::VrcPhotonClient::webRpcReturn()\n");
}

void VRChad::PhotonClient::onRoomListUpdate()
{
    fmt::print("VRChad::VrcPhotonClient::onRoomListUpdate()\n");
}

void VRChad::PhotonClient::onRoomPropertiesChange(const ExitGames::Common::Hashtable&)
{
    fmt::print("VRChad::VrcPhotonClient::onRoomPropertiesChange()\n");
}

void VRChad::PhotonClient::onPlayerPropertiesChange(int, const ExitGames::Common::Hashtable&)
{
    fmt::print("VRChad::VrcPhotonClient::onPlayerPropertiesChange()\n");
}

void VRChad::PhotonClient::onAppStatsUpdate()
{
    fmt::print("VRChad::VrcPhotonClient::onAppStatsUpdate()\n");
}

void VRChad::PhotonClient::onLobbyStatsUpdate(const ExitGames::Common::JVector<ExitGames::LoadBalancing::LobbyStatsResponse>&)
{
    fmt::print("VRChad::VrcPhotonClient::onLobbyStatsUpdate()\n");
}

void VRChad::PhotonClient::onCacheSliceChanged(int)
{
    fmt::print("VRChad::VrcPhotonClient::onCacheSliceChanged()\n");
}

void VRChad::PhotonClient::onMasterClientChanged(int, int)
{
    fmt::print("VRChad::VrcPhotonClient::onMasterClientChanged()\n");
}

void VRChad::PhotonClient::onAvailableRegions(const ExitGames::Common::JVector<ExitGames::Common::JString>& names, const ExitGames::Common::JVector<ExitGames::Common::JString>& ips)
{
    fmt::print("\n[Photon] Got available regions:\n");

    if (m_connectionState == ClientConnectionState::ConnectingToNS) {
        m_connectionState = ClientConnectionState::ConnectedToNS;
    }

    std::wstring name;
    for (std::uint32_t i = 0; i < names.getSize(); i++) {
        name = std::wstring(names[i].cstr(), names[i].length());
        std::wstring_view ip(ips[i].cstr(), ips[i].length());

        fmt::print(L"\t{}:\t{}\n", name, ip);

        m_regions[name] = ip;
    }

    // Set the current region to the latest one listed
    m_selectedRegion = name;

    // Connect to the selected region
    selectRegion(name.data());
}

void VRChad::PhotonClient::onSecretReceival(const ExitGames::Common::JString& secret)
{
    m_photonSecret = std::wstring(secret.cstr(), secret.length());

    fmt::print(L"\n[Photon] Got secret\n", m_photonSecret);

    if (m_connectionState == ClientConnectionState::ConnectedToNS)
    {
        fmt::print(L"[Photon] Disconnecting from nameserver to connect to master server\n\n", m_photonSecret);
        m_connectionState = ClientConnectionState::ConnectingToMaster;

        disconnect();
    }
}

void VRChad::PhotonClient::onDirectConnectionEstablished(int)
{
    fmt::print("VRChad::VrcPhotonClient::onDirectConnectionEstablished()\n");
}

void VRChad::PhotonClient::onDirectConnectionFailedToEstablish(int)
{
    fmt::print("VRChad::VrcPhotonClient::onDirectConnectionFailedToEstablish()\n");
}

void VRChad::PhotonClient::onDirectMessage(const ExitGames::Common::Object&, int, bool)
{
    fmt::print("VRChad::VrcPhotonClient::onDirectMessage()\n");
}

void VRChad::PhotonClient::onCustomOperationResponse(const ExitGames::Photon::OperationResponse&)
{
    fmt::print("VRChad::VrcPhotonClient::onCustomOperationResponse()\n");
}

void VRChad::PhotonClient::onGetRoomListResponse(const ExitGames::Common::JVector<ExitGames::Common::Helpers::SharedPointer<ExitGames::LoadBalancing::Room>>&, const ExitGames::Common::JVector<ExitGames::Common::JString>&)
{
    fmt::print("VRChad::VrcPhotonClient::onGetRoomListResponse()\n");
}
