#include "photonclient.h"

#include <thread>

#include "fmt/core.h"
#include "fmt/xchar.h"

std::string_view VRCHAT_APPID = "bf0942f7-9935-4192-b359-f092fa85bef1";
std::string_view PHOTON_MAIN_NS = "62.169.222.103:5058";

namespace PhotonLB = ExitGames::LoadBalancing;

inline std::string_view utf8_encode(const ExitGames::Common::JString& wstr)
{
    auto utf8 = wstr.UTF8Representation();
    return std::string_view(utf8.cstr(), utf8.size());
}
inline std::string utf8_encode_copy(const ExitGames::Common::JString& wstr)
{
    return std::string(utf8_encode(wstr));
}
inline ExitGames::Common::JString utf8_decode(const std::string& str)
{
    return ExitGames::Common::JString(str.data());
}

template <typename T>
inline T DecodeJType(const ExitGames::Common::Object* object) {
    return ExitGames::Common::ValueObject<T>(object).getDataAddress();
}
template <typename T>
inline std::optional<T> DecodeObject(const ExitGames::Common::Object* object) {
    if (object != nullptr) {
        if constexpr (std::is_same<T, ExitGames::Common::JString>::value) {
            return ExitGames::Common::ValueObject<ExitGames::Common::JString>(object).getDataCopy();
        }
        else if constexpr (std::is_same<T, std::string>::value) {
            ExitGames::Common::ValueObject<ExitGames::Common::JString> valueObj(object);
            return utf8_encode_copy(*valueObj.getDataAddress());
        }
    }

    return std::nullopt;
}

VRChad::PhotonClient::PhotonClient(std::string_view vrchatClientVersion, std::string_view userId, std::string_view authToken, std::string_view hwid)
    : PhotonLB::Client(*this, VRCHAT_APPID.data(), vrchatClientVersion.data(), ExitGames::Photon::ConnectionProtocol::UDP, false, PhotonLB::RegionSelectionMode::SELECT, false)
    , PhotonLB::Listener()
    , m_photonThread()
    , m_authValues()
    , m_regions()
    , m_userId(userId)
    , m_authToken(authToken)
    , m_hwid(hwid)
    , m_authParamsStr()
    , m_cloudRegion()
    , m_photonSecret()
    , m_disconnectIntention(DisconnectIntention::Unknown)
    , m_connectionState(ConnectionState::Disconnected)
{
    fmt::print("[Photon] Starting photon thread...\n");
    m_photonThread = std::thread(&VRChad::PhotonClient::photonLoop, this);

    fmt::print("[Photon] Setting up connection options...\n");
    ExitGames::LoadBalancing::ConnectOptions connectOptions;
    connectOptions.setAuthenticationValues(m_authValues);
    connectOptions.setServerAddress(PHOTON_MAIN_NS.data());
    connectOptions.setServerType(PhotonLB::ServerType::NAME_SERVER);

    m_authParamsStr = fmt::format("token={}&user={}&hwid={}&platform=android", m_authToken, m_userId, m_hwid);

    m_authValues.setType(PhotonLB::CustomAuthenticationType::CUSTOM);
    m_authValues.setParameters(m_authParamsStr.c_str());

    connectOptions.setAuthenticationValues(m_authValues);

    connect(connectOptions);
    SetConnectionState(ConnectionState::ConnectingToNameServer);
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

void VRChad::PhotonClient::connectToRegionMaster() {
    if (GetConnectionStateIsConnected()) {
        SetDisconnectIntention(DisconnectIntention::ConnectToMasterServer);
        SetConnectionStateAsDisconnecting();
        disconnect();
    } else {
        SetConnectionState(ConnectionState::ConnectingToMasterServer);

        std::string_view addr = m_regions[m_cloudRegion];
        m_authParamsStr = fmt::format("token={}&user={}&hwid={}&platform=standalonewindows", m_authToken, m_userId, m_hwid);

        fmt::print("[Photon] RegionMaster: {} BestPing: {}\n", utf8_encode(getMasterserverAddress()), utf8_encode(getRegionWithBestPing()));
        fmt::print("[Photon] Connecting to {} with params: {}\n\n", addr, m_authParamsStr);

        m_authValues.setType(PhotonLB::CustomAuthenticationType::CUSTOM);
        m_authValues.setParameters(m_authParamsStr.c_str());

        ExitGames::LoadBalancing::ConnectOptions connectOptions;
        connectOptions.setAuthenticationValues(m_authValues);
        connectOptions.setServerAddress(addr.data());
        connectOptions.setServerType(PhotonLB::ServerType::MASTER_SERVER);

        connect(connectOptions);
    }
}

void VRChad::PhotonClient::onCustomAuthenticationIntermediateStep(const ExitGames::Common::Dictionary<ExitGames::Common::JString, ExitGames::Common::Object>& authValues)
{

}

void VRChad::PhotonClient::onStatusChanged(int statusCode)
{
    Client::onStatusChanged(statusCode); // Call base

    switch (statusCode) {
    case ExitGames::Photon::StatusCode::EXCEPTION_ON_CONNECT:
        fmt::print("[Photon] encountered an exception while opening the incoming connection to the server. The server could be down / not running\n");
        break;
    case ExitGames::Photon::StatusCode::CONNECT:
        SetConnectionStateAsConnected();
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
        SetConnectionState(ConnectionState::Disconnected);
        break;
    case ExitGames::Photon::StatusCode::DISCONNECT_BY_SERVER:
        fmt::print("[Photon] Disconnect by server due to timeout (received a disconnect command, cause server misses ACKs of client)\n");
        SetDisconnectIntention(DisconnectIntention::DisconnectedByServer);
        SetConnectionState(ConnectionState::Disconnected);
        break;
    case ExitGames::Photon::StatusCode::DISCONNECT_BY_SERVER_LOGIC:
        fmt::print("[Photon] Disconnect by server due to server's logic (received a disconnect command)\n");
        SetDisconnectIntention(DisconnectIntention::DisconnectedByServerLogic);
        SetConnectionState(ConnectionState::Disconnected);
        break;
    case ExitGames::Photon::StatusCode::DISCONNECT_BY_SERVER_USER_LIMIT:
        fmt::print("[Photon] Disconnect by server due to concurrent user limit reached (received a disconnect command)\n");
        SetDisconnectIntention(DisconnectIntention::DisconnectedByServerUserLimit);
        SetConnectionState(ConnectionState::Disconnected);
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

    fmt::print("VRChad::VrcPhotonClient::onEvent() {}\n", eventData.getCode());

    if (eventData.getCode() == 255) {
        fmt::print("User joined!\n");
    }
}

void VRChad::PhotonClient::onPingResponse(const ExitGames::Common::JString& address, unsigned int result)
{
    Client::onPingResponse(address, result); // Call base

    fmt::print("VRChad::VrcPhotonClient::onPingResponse() {} {}\n", utf8_encode(address), result);
}

void VRChad::PhotonClient::debugReturn(int debugLevel, const ExitGames::Common::JString& jstring)
{
    fmt::print("VRChad::VrcPhotonClient::debugReturn() {} {}\n", debugLevel, utf8_encode(jstring));
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

void VRChad::PhotonClient::joinRoomEventAction(int playerNr, const ExitGames::Common::JVector<int>& playernrs, const PhotonLB::Player& player)
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
    SetConnectionState(ConnectionState::Disconnected);

    if (m_disconnectIntention == DisconnectIntention::ConnectToMasterServer) {
        connectToRegionMaster();
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

void VRChad::PhotonClient::onLobbyStatsResponse(const ExitGames::Common::JVector<PhotonLB::LobbyStatsResponse>&)
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

void VRChad::PhotonClient::onLobbyStatsUpdate(const ExitGames::Common::JVector<PhotonLB::LobbyStatsResponse>&)
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
    fmt::print("[Photon] Got available regions:\n");

    for (std::uint32_t i = 0; i < names.getSize(); i++) {
        std::string ip = utf8_encode_copy(ips[i]);
        std::string name = utf8_encode_copy(names[i]);

        fmt::print("\t{}:\t{}\n", name, ip);

        m_regions[name] = ip;
    }

    if (m_regions.contains("eu")) {
        m_cloudRegion = "eu";
    } else if (m_regions.contains("us")) {
        m_cloudRegion = "us";
    } else if (m_regions.contains("usw")) {
        m_cloudRegion = "usw";
    } else {
        m_cloudRegion = m_regions.begin()->first;
    }

    // Connect to the selected region
    selectRegion(m_cloudRegion.data());
}

void VRChad::PhotonClient::onSecretReceival(const ExitGames::Common::JString& secret)
{
    m_photonSecret = utf8_encode_copy(secret);

    fmt::print("\n[Photon] Got secret: {}\n", m_photonSecret);

    if (m_connectionState == ConnectionState::ConnectedToNameServer) {
        connectToRegionMaster();
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

void VRChad::PhotonClient::onGetRoomListResponse(const ExitGames::Common::JVector<ExitGames::Common::Helpers::SharedPointer<PhotonLB::Room>>&, const ExitGames::Common::JVector<ExitGames::Common::JString>&)
{
    fmt::print("VRChad::VrcPhotonClient::onGetRoomListResponse()\n");
}

void VRChad::PhotonClient::SetDisconnectIntention(DisconnectIntention disconnectIntention)
{
    if (m_disconnectIntention != disconnectIntention) {
        m_disconnectIntention = disconnectIntention;
        switch (disconnectIntention) {
        case DisconnectIntention::DisconnectedByServer:
        case DisconnectIntention::DisconnectedByServerLogic:
        case DisconnectIntention::DisconnectedByServerUserLimit:
            SetConnectionState(ConnectionState::Disconnected);
            break;
        default:
            break;
        }
    }
}

std::string_view VRChad::PhotonClient::GetDisconnectIntentionString()
{
    using namespace std::literals;
    switch (m_disconnectIntention) {
    case DisconnectIntention::Disconnect:
        return "Disconnect"sv;
        break;
    case DisconnectIntention::ConnectToNameServer:
        return "Connect to nameserver"sv;
        break;
    case DisconnectIntention::ConnectToMasterServer:
        return "Connect to masterserver"sv;
        break;
    case DisconnectIntention::ConnectToGameServer:
        return "Connect to gameserver"sv;
        break;
    case DisconnectIntention::DisconnectedByServer:
        return "Disconnected by server"sv;
        break;
    case DisconnectIntention::DisconnectedByServerLogic:
        return "Disconnected by server logic"sv;
        break;
    case DisconnectIntention::DisconnectedByServerUserLimit:
        return "Disconnected by server due to user limit"sv;
        break;
    default:
        m_disconnectIntention = DisconnectIntention::Unknown;
    case DisconnectIntention::Unknown:
        return "Unknown"sv;
    }
}

void VRChad::PhotonClient::SetConnectionState(ConnectionState connectionState)
{
    if (m_connectionState != connectionState) {
        m_connectionState = connectionState;
        switch (connectionState) {
        case ConnectionState::Disconnected:
            fmt::print("[Photon] Disconnected, reason: {}\n", GetDisconnectIntentionString());
            break;
        case ConnectionState::ConnectingToNameServer:
            SetDisconnectIntention(DisconnectIntention::Unknown);
            fmt::print("[Photon] Connecting to nameserver\n");
            break;
        case ConnectionState::ConnectedToNameServer:
            fmt::print("[Photon] Connected to nameserver\n");
            break;
        case ConnectionState::DisconnectingFromNameServer:
            fmt::print("[Photon] Disconnecting from nameserver\n");
            break;
        case ConnectionState::ConnectingToMasterServer:
            SetDisconnectIntention(DisconnectIntention::Unknown);
            fmt::print("[Photon] Connecting to master server\n");
            break;
        case ConnectionState::ConnectedToMasterServer:
            fmt::print("[Photon] Connected to master server\n");
            break;
        case ConnectionState::DisconnectingFromMasterServer:
            fmt::print("[Photon] Disconnecting from master server\n");
            break;
        case ConnectionState::ConnectingToGameServer:
            SetDisconnectIntention(DisconnectIntention::Unknown);
            fmt::print("[Photon] Connecting to game server\n");
            break;
        case ConnectionState::ConnectedToGameServer:
            fmt::print("[Photon] Connected to game server\n");
            break;
        case ConnectionState::DisconnectingFromGameServer:
            fmt::print("[Photon] Disconnecting from game server\n");
            break;
        default:
            break;
        }
    }
}

void VRChad::PhotonClient::SetConnectionStateAsConnected()
{
    switch (m_connectionState) {
    case ConnectionState::ConnectingToNameServer:
        SetConnectionState(ConnectionState::ConnectedToNameServer);
        break;
    case ConnectionState::ConnectingToMasterServer:
        SetConnectionState(ConnectionState::ConnectedToMasterServer);
        break;
    case ConnectionState::ConnectingToGameServer:
        SetConnectionState(ConnectionState::ConnectedToNameServer);
        break;
    default:
        break;
    }
}

void VRChad::PhotonClient::SetConnectionStateAsDisconnecting()
{
    switch (m_connectionState) {
    case ConnectionState::ConnectedToNameServer:
        SetConnectionState(ConnectionState::DisconnectingFromNameServer);
        break;
    case ConnectionState::ConnectedToMasterServer:
        SetConnectionState(ConnectionState::DisconnectingFromMasterServer);
        break;
    case ConnectionState::ConnectedToGameServer:
        SetConnectionState(ConnectionState::DisconnectingFromNameServer);
        break;
    default:
        break;
    }
}

bool VRChad::PhotonClient::GetConnectionStateIsConnected()
{
    switch (m_connectionState) {
    case ConnectionState::ConnectedToNameServer:
    case ConnectionState::ConnectedToMasterServer:
    case ConnectionState::ConnectedToGameServer:
        return true;
    default:
        return false;
    }
}
