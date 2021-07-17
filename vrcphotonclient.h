#ifndef VRCPHOTONCLIENT_H
#define VRCPHOTONCLIENT_H

#include "LoadBalancing-cpp/inc/Client.h"

#include <QString>

#include <thread>

class VrcPhotonClient : public ExitGames::LoadBalancing::Client, private ExitGames::LoadBalancing::Listener
{
public:
    VrcPhotonClient(const QString& userId, const QString& authToken);
    ~VrcPhotonClient();
private:
    void photonLoop();

    // receive and print out debug out here
    void debugReturn(int debugLevel, const ExitGames::Common::JString& string) override;

    void onEvent(const ExitGames::Photon::EventData& eventData) override;

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
    void disconnectReturn(void) override;
    void leaveRoomReturn(int errorCode, const ExitGames::Common::JString& errorString) override;

    std::thread m_photonThread;
    ExitGames::LoadBalancing::AuthenticationValues m_authValues;
};

#endif // VRCPHOTONCLIENT_H
