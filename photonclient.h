#ifndef PHOTONCLIENT_H
#define PHOTONCLIENT_H

#include "LoadBalancing-cpp/inc/Client.h"

#include <thread>
#include <unordered_map>

namespace VRChad {
class PhotonClient : public ExitGames::LoadBalancing::Client, private ExitGames::LoadBalancing::Listener
{
public:
    PhotonClient(std::string_view vrchatClientVersion, std::string_view userId, std::string_view authToken, std::string_view hwid);
    ~PhotonClient();

    bool joinRoom(std::string_view roomId);
    bool leaveRoom();
private:
    void photonLoop();

    void connectToRegionMaster();

    void onCustomAuthenticationIntermediateStep(const ExitGames::Common::Dictionary<ExitGames::Common::JString, ExitGames::Common::Object>& authValues) override;

    void onStatusChanged(int statusCode) override;
    void onEvent(const ExitGames::Photon::EventData& eventData) override;
    void onPingResponse(const ExitGames::Common::JString& address, unsigned int result) override;

    // receive and print out debug out here
    void debugReturn(int debugLevel, const ExitGames::Common::JString& string) override;

    // implement your error-handling here
    void connectionErrorReturn(int errorCode) override;
    void clientErrorReturn(int errorCode) override;
    void warningReturn(int warningCode) override;
    void serverErrorReturn(int errorCode) override;

    // events, triggered by certain operations of all players in the same room
    void joinRoomEventAction(int playerNr, const ExitGames::Common::JVector<int>& playernrs, const ExitGames::LoadBalancing::Player& player) override;
    void leaveRoomEventAction(int playerNr, bool isInactive) override;

    void customEventAction(int playerNr, nByte eventCode, const ExitGames::Common::Object& eventContent) override;

    // callbacks for operations on the server
    void connectReturn(int errorCode, const ExitGames::Common::JString& errorString, const ExitGames::Common::JString& region, const ExitGames::Common::JString& cluster) override;
    void disconnectReturn() override;
    void createRoomReturn(int /*localPlayerNr*/, const ExitGames::Common::Hashtable& /*roomProperties*/, const ExitGames::Common::Hashtable& /*playerProperties*/, int /*errorCode*/, const ExitGames::Common::JString& /*errorString*/) override;
    void joinOrCreateRoomReturn(int /*localPlayerNr*/, const ExitGames::Common::Hashtable& /*roomProperties*/, const ExitGames::Common::Hashtable& /*playerProperties*/, int /*errorCode*/, const ExitGames::Common::JString& /*errorString*/) override;
    void joinRandomOrCreateRoomReturn(int /*localPlayerNr*/, const ExitGames::Common::Hashtable& /*roomProperties*/, const ExitGames::Common::Hashtable& /*playerProperties*/, int /*errorCode*/, const ExitGames::Common::JString& /*errorString*/) override;
    void joinRoomReturn(int /*localPlayerNr*/, const ExitGames::Common::Hashtable& /*roomProperties*/, const ExitGames::Common::Hashtable& /*playerProperties*/, int /*errorCode*/, const ExitGames::Common::JString& /*errorString*/) override;
    void joinRandomRoomReturn(int /*localPlayerNr*/, const ExitGames::Common::Hashtable& /*roomProperties*/, const ExitGames::Common::Hashtable& /*playerProperties*/, int /*errorCode*/, const ExitGames::Common::JString& /*errorString*/) override;
    void leaveRoomReturn(int errorCode, const ExitGames::Common::JString& errorString) override;
    void joinLobbyReturn() override;
    void leaveLobbyReturn() override;
    void onFindFriendsResponse() override;
    void onLobbyStatsResponse(const ExitGames::Common::JVector<ExitGames::LoadBalancing::LobbyStatsResponse>& /*lobbyStats*/) override;
    void webRpcReturn(int /*errorCode*/, const ExitGames::Common::JString& /*errorString*/, const ExitGames::Common::JString& /*uriPath*/, int /*resultCode*/, const ExitGames::Common::Dictionary<ExitGames::Common::Object, ExitGames::Common::Object>& /*returnData*/) override;

    // info, that certain values have been updated
    void onRoomListUpdate() override;
    void onRoomPropertiesChange(const ExitGames::Common::Hashtable& /*changes*/) override;
    void onPlayerPropertiesChange(int /*playerNr*/, const ExitGames::Common::Hashtable& /*changes*/) override;
    void onAppStatsUpdate() override;
    void onLobbyStatsUpdate(const ExitGames::Common::JVector<ExitGames::LoadBalancing::LobbyStatsResponse>& /*lobbyStats*/) override;
    void onCacheSliceChanged(int /*cacheSliceIndex*/) override;
    void onMasterClientChanged(int /*id*/, int /*oldID*/) override;

    // receive the available server regions during the connect workflow (if you have specified in the constructor, that you want to select a region)
    void onAvailableRegions(const ExitGames::Common::JVector<ExitGames::Common::JString>& /*availableRegions*/, const ExitGames::Common::JVector<ExitGames::Common::JString>& /*availableRegionServers*/) override;

    void onSecretReceival(const ExitGames::Common::JString& /*secret*/) override;

    void onDirectConnectionEstablished(int /*remoteID*/) override;
    void onDirectConnectionFailedToEstablish(int /*remoteID*/) override;
    void onDirectMessage(const ExitGames::Common::Object& /*msg*/, int /*remoteID*/, bool /*relay*/) override;

    void onCustomOperationResponse(const ExitGames::Photon::OperationResponse& /*operationResponse*/) override;

    void onGetRoomListResponse(const ExitGames::Common::JVector<ExitGames::Common::Helpers::SharedPointer<ExitGames::LoadBalancing::Room> >& /*roomList*/, const ExitGames::Common::JVector<ExitGames::Common::JString>& /*roomNameList*/) override;

    std::thread m_photonThread;
    ExitGames::LoadBalancing::AuthenticationValues m_authValues;

    std::unordered_map<std::string, std::string> m_regions;

    std::string m_userId;
    std::string m_authToken;
    std::string m_hwid;
    std::string m_authParamsStr;

    std::string m_cloudRegion;
    std::string m_photonSecret;

    enum class DisconnectIntention {
        Unknown,
        Disconnect,
        ConnectToNameServer,
        ConnectToMasterServer,
        ConnectToGameServer,
        DisconnectedByServer,
        DisconnectedByServerLogic,
        DisconnectedByServerUserLimit,
    } m_disconnectIntention;
    void SetDisconnectIntention(DisconnectIntention disconnectIntention);
    std::string_view GetDisconnectIntentionString();
    enum class ConnectionState {
        Disconnected,

        ConnectingToNameServer,
        ConnectedToNameServer,
        DisconnectingFromNameServer,

        ConnectingToMasterServer,
        ConnectedToMasterServer,
        DisconnectingFromMasterServer,

        ConnectingToGameServer,
        ConnectedToGameServer,
        DisconnectingFromGameServer,
    } m_connectionState;
    void SetConnectionState(ConnectionState connectionState);
    void SetConnectionStateAsConnected();
    void SetConnectionStateAsDisconnecting();
    bool GetConnectionStateIsConnected();
};
}

#endif // PHOTONCLIENT_H
