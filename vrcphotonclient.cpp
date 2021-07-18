#include "vrcphotonclient.h"

#include <thread>

#include <QDebug>

namespace PhotonLB = ExitGames::LoadBalancing;

VrcPhotonClient::VrcPhotonClient(const QString& userId, const QString& authToken)
    : PhotonLB::Client(*this, "bf0942f7-9935-4192-b359-f092fa85bef1", "Release_2018_server_1113", ExitGames::Photon::ConnectionProtocol::UDP, false, PhotonLB::RegionSelectionMode::SELECT, false)
    , PhotonLB::Listener()
    , m_photonThread()
    , m_authValues()
{
    QString parametersString = QString("token=%1&user=%2").arg(authToken, userId);

    m_authValues.setType(ExitGames::LoadBalancing::CustomAuthenticationType::CUSTOM);
    m_authValues.setParameters(parametersString.toStdString().c_str());

    m_photonThread = std::thread(&VrcPhotonClient::photonLoop, this);

    connect(m_authValues, userId.toStdString().c_str(), "ns.exitgames.com", ExitGames::LoadBalancing::ServerType::NAME_SERVER);
}

VrcPhotonClient::~VrcPhotonClient()
{
    qDebug() << "VrcPhotonClient::~VrcPhotonClient()";
}

bool VrcPhotonClient::leaveRoom()
{
    return opLeaveRoom(false);
}

QStringList VrcPhotonClient::availableRegions() const
{
    return m_regions.keys();
}

void VrcPhotonClient::photonLoop()
{
    qDebug() << "photonLoop()";
    while (true) {
        service();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

void VrcPhotonClient::onOperationResponse(const ExitGames::Photon::OperationResponse& operationResponse)
{
    Client::onOperationResponse(operationResponse); // Call base
}

void VrcPhotonClient::onStatusChanged(int statusCode)
{
    Client::onStatusChanged(statusCode); // Call base

    switch (statusCode) {
    case ExitGames::Photon::StatusCode::EXCEPTION_ON_CONNECT:
        std::cout << "PhotonClient encountered an exception while opening the incoming connection to the server. The server could be down / not running" << std::endl;
        break;
    case ExitGames::Photon::StatusCode::CONNECT:
        std::cout << "PhotonClient connected" << std::endl;
        break;
    case ExitGames::Photon::StatusCode::DISCONNECT:
        std::cout << "PhotonClient disconnected" << std::endl;
        break;
    case ExitGames::Photon::StatusCode::EXCEPTION:
        std::cout << "PhotonClient encountered an exception and will disconnect, too" << std::endl;
        break;
    case ExitGames::Photon::StatusCode::QUEUE_OUTGOING_RELIABLE_WARNING:
        std::cout << "PhotonClient outgoing queue is filling up. send more often" << std::endl;
        break;
    case ExitGames::Photon::StatusCode::QUEUE_OUTGOING_UNRELIABLE_WARNING:
        std::cout << "PhotonClient outgoing queue is filling up. send more often" << std::endl;
        break;
    case ExitGames::Photon::StatusCode::SEND_ERROR:
        std::cout << "Sending command failed. Either not connected, or the requested channel is bigger than the number of initialized channels" << std::endl;
        break;
    case ExitGames::Photon::StatusCode::QUEUE_OUTGOING_ACKS_WARNING:
        std::cout << "PhotonClient outgoing queue is filling up. Send more often" << std::endl;
        break;
    case ExitGames::Photon::StatusCode::QUEUE_INCOMING_RELIABLE_WARNING:
        std::cout << "PhotonClient incoming reliable queue is filling up. Dispatch more often" << std::endl;
        break;
    case ExitGames::Photon::StatusCode::QUEUE_INCOMING_UNRELIABLE_WARNING:
        std::cout << "PhotonClient incoming unreliable queue is filling up. Dispatch more often" << std::endl;
        break;
    case ExitGames::Photon::StatusCode::QUEUE_SENT_WARNING:
        std::cout << "PhotonClient sent queue is filling up. Check, why the server does not acknowledge your sent reliable commands" << std::endl;
        break;
    case ExitGames::Photon::StatusCode::INTERNAL_RECEIVE_EXCEPTION:
        std::cout << "PhotonClient Exception, if a server cannot be connected. Most likely, the server is not responding. Ask the user to try again later" << std::endl;
        break;
    case ExitGames::Photon::StatusCode::TIMEOUT_DISCONNECT:
        std::cout << "PhotonClient Disconnection due to a timeout (client did no longer receive ACKs from server)" << std::endl;
        break;
    case ExitGames::Photon::StatusCode::DISCONNECT_BY_SERVER:
        std::cout << "PhotonClient Disconnect by server due to timeout (received a disconnect command, cause server misses ACKs of client)" << std::endl;
        break;
    case ExitGames::Photon::StatusCode::DISCONNECT_BY_SERVER_USER_LIMIT:
        std::cout << "PhotonClient Disconnect by server due to concurrent user limit reached (received a disconnect command)" << std::endl;
        break;
    case ExitGames::Photon::StatusCode::DISCONNECT_BY_SERVER_LOGIC:
        std::cout << "PhotonClient Disconnect by server due to server's logic (received a disconnect command)" << std::endl;
        break;
    case ExitGames::Photon::StatusCode::ENCRYPTION_ESTABLISHED:
        std::cout << "PhotonClient Encryption success" << std::endl;
        break;
    case ExitGames::Photon::StatusCode::ENCRYPTION_FAILED_TO_ESTABLISH:
        std::cout << "PhotonClient Encryption failed, Check debug logs" << std::endl;
        break;
    }
}

void VrcPhotonClient::onEvent(const ExitGames::Photon::EventData& eventData)
{
    Client::onEvent(eventData); // Call base

    qDebug() << "VrcPhotonClient::onEvent()" << eventData.getCode();

    if (eventData.getCode() == 255) {
        qDebug() << "User joined!";
    }
}

void VrcPhotonClient::onPingResponse(const ExitGames::Common::JString& address, unsigned int result)
{
    Client::onPingResponse(address, result); // Call base

    qDebug() << "VrcPhotonClient::onPingResponse()" << address.cstr();
}

void VrcPhotonClient::debugReturn(int debugLevel, const ExitGames::Common::JString& string)
{
    qDebug() << "VrcPhotonClient::debugReturn()" << debugLevel << QString::fromWCharArray(string.cstr());
}


void VrcPhotonClient::connectionErrorReturn(int errorCode)
{
    qDebug() << "VrcPhotonClient::connectionErrorReturn()" << errorCode;
}

void VrcPhotonClient::clientErrorReturn(int errorCode)
{
    qDebug() << "VrcPhotonClient::clientErrorReturn()" << errorCode;
}

void VrcPhotonClient::warningReturn(int warningCode)
{
    qDebug() << "VrcPhotonClient::warningReturn()" << warningCode;
}

void VrcPhotonClient::serverErrorReturn(int errorCode)
{
    qDebug() << "VrcPhotonClient::serverErrorReturn()" << errorCode;
}

void VrcPhotonClient::joinRoomEventAction(int playerNr, const ExitGames::Common::JVector<int>& playernrs, const ExitGames::LoadBalancing::Player& player)
{
    qDebug() << "VrcPhotonClient::joinRoomEventAction()" << playerNr;
}

void VrcPhotonClient::leaveRoomEventAction(int playerNr, bool isInactive)
{
    qDebug() << "VrcPhotonClient::leaveRoomEventAction()" << playerNr;
}

void VrcPhotonClient::customEventAction(int playerNr, nByte eventCode, const ExitGames::Common::Object& eventContent)
{
    qDebug() << "VrcPhotonClient::customEventAction()" << playerNr;
}

void VrcPhotonClient::connectReturn(int errorCode, const ExitGames::Common::JString& errorString, const ExitGames::Common::JString& region, const ExitGames::Common::JString& cluster)
{
    qDebug() << "VrcPhotonClient::connectReturn()" << errorCode;
}

void VrcPhotonClient::disconnectReturn()
{
    qDebug() << "VrcPhotonClient::disconnectReturn()";
}

void VrcPhotonClient::createRoomReturn(int, const ExitGames::Common::Hashtable&, const ExitGames::Common::Hashtable&, int, const ExitGames::Common::JString&)
{
    qDebug() << "VrcPhotonClient::createRoomReturn()";
}

void VrcPhotonClient::joinOrCreateRoomReturn(int, const ExitGames::Common::Hashtable&, const ExitGames::Common::Hashtable&, int, const ExitGames::Common::JString&)
{
    qDebug() << "VrcPhotonClient::joinOrCreateRoomReturn()";
}

void VrcPhotonClient::joinRandomOrCreateRoomReturn(int, const ExitGames::Common::Hashtable&, const ExitGames::Common::Hashtable&, int, const ExitGames::Common::JString&)
{
    qDebug() << "VrcPhotonClient::joinRandomOrCreateRoomReturn()";
}

void VrcPhotonClient::joinRoomReturn(int, const ExitGames::Common::Hashtable&, const ExitGames::Common::Hashtable&, int, const ExitGames::Common::JString&)
{
    qDebug() << "VrcPhotonClient::joinRoomReturn()";
}

void VrcPhotonClient::joinRandomRoomReturn(int, const ExitGames::Common::Hashtable&, const ExitGames::Common::Hashtable&, int, const ExitGames::Common::JString&)
{
    qDebug() << "VrcPhotonClient::joinRandomRoomReturn()";
}

void VrcPhotonClient::leaveRoomReturn(int errorCode, const ExitGames::Common::JString& errorString)
{
    qDebug() << "VrcPhotonClient::leaveRoomReturn()";
}

void VrcPhotonClient::joinLobbyReturn()
{
    qDebug() << "VrcPhotonClient::joinLobbyReturn()";
}

void VrcPhotonClient::leaveLobbyReturn()
{
    qDebug() << "VrcPhotonClient::leaveLobbyReturn()";
}

void VrcPhotonClient::onFindFriendsResponse()
{
    qDebug() << "VrcPhotonClient::onFindFriendsResponse()";
}

void VrcPhotonClient::onLobbyStatsResponse(const ExitGames::Common::JVector<ExitGames::LoadBalancing::LobbyStatsResponse>&)
{
    qDebug() << "VrcPhotonClient::onLobbyStatsResponse()";
}

void VrcPhotonClient::webRpcReturn(int, const ExitGames::Common::JString&, const ExitGames::Common::JString&, int, const ExitGames::Common::Dictionary<ExitGames::Common::Object, ExitGames::Common::Object>&)
{
    qDebug() << "VrcPhotonClient::webRpcReturn()";
}

void VrcPhotonClient::onRoomListUpdate()
{
    qDebug() << "VrcPhotonClient::onRoomListUpdate()";
}

void VrcPhotonClient::onRoomPropertiesChange(const ExitGames::Common::Hashtable&)
{
    qDebug() << "VrcPhotonClient::onRoomPropertiesChange()";
}

void VrcPhotonClient::onPlayerPropertiesChange(int, const ExitGames::Common::Hashtable&)
{
    qDebug() << "VrcPhotonClient::onPlayerPropertiesChange()";
}

void VrcPhotonClient::onAppStatsUpdate()
{
    qDebug() << "VrcPhotonClient::onAppStatsUpdate()";
}

void VrcPhotonClient::onLobbyStatsUpdate(const ExitGames::Common::JVector<ExitGames::LoadBalancing::LobbyStatsResponse>&)
{
    qDebug() << "VrcPhotonClient::onLobbyStatsUpdate()";
}

void VrcPhotonClient::onCacheSliceChanged(int)
{
    qDebug() << "VrcPhotonClient::onCacheSliceChanged()";
}

void VrcPhotonClient::onMasterClientChanged(int, int)
{
    qDebug() << "VrcPhotonClient::onMasterClientChanged()";
}

void VrcPhotonClient::onAvailableRegions(const ExitGames::Common::JVector<ExitGames::Common::JString>& names, const ExitGames::Common::JVector<ExitGames::Common::JString>& ips)
{
    for (std::uint32_t i = 0; i < names.getSize(); i++) {
        QString name = QString::fromWCharArray(names[i].cstr());
        QString ipAddress = QString::fromWCharArray(ips[i].cstr());

        m_regions[name] = ipAddress;
    }

    qDebug() << "Got Regions:\n" << m_regions;

    qDebug() << selectRegion("eu");
}

void VrcPhotonClient::onSecretReceival(const ExitGames::Common::JString& secret)
{
    qDebug() << "Got secret!";
}

void VrcPhotonClient::onDirectConnectionEstablished(int)
{
    qDebug() << "VrcPhotonClient::onDirectConnectionEstablished()";
}

void VrcPhotonClient::onDirectConnectionFailedToEstablish(int)
{
    qDebug() << "VrcPhotonClient::onDirectConnectionFailedToEstablish()";
}

void VrcPhotonClient::onDirectMessage(const ExitGames::Common::Object&, int, bool)
{
    qDebug() << "VrcPhotonClient::onDirectMessage()";
}

void VrcPhotonClient::onCustomOperationResponse(const ExitGames::Photon::OperationResponse&)
{
    qDebug() << "VrcPhotonClient::onCustomOperationResponse()";
}

void VrcPhotonClient::onGetRoomListResponse(const ExitGames::Common::JVector<ExitGames::Common::Helpers::SharedPointer<ExitGames::LoadBalancing::Room> >&, const ExitGames::Common::JVector<ExitGames::Common::JString>&)
{
    qDebug() << "VrcPhotonClient::onGetRoomListResponse()";
}
