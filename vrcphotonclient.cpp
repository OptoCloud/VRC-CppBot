#include "vrcphotonclient.h"

#include <thread>

#include <QDebug>

namespace PhotonLB = ExitGames::LoadBalancing;

VrcPhotonClient::VrcPhotonClient(const QString& userId, const QString& authToken)
    : PhotonLB::Client(*this, "bf0942f7-9935-4192-b359-f092fa85bef1", "Release_2018_server_1113")
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

void VrcPhotonClient::photonLoop()
{
    qDebug() << "photonLoop()";
    while (true) {
        service();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

void VrcPhotonClient::debugReturn(int debugLevel, const ExitGames::Common::JString& string)
{
    qDebug() << "VrcPhotonClient::debugReturn()" << debugLevel << string;
}

void VrcPhotonClient::onEvent(const ExitGames::Photon::EventData& eventData)
{
    Client::onEvent(eventData); // Call base

    if (eventData.getCode() == 255) {
        qDebug() << "User joined!";
    }
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

}

void VrcPhotonClient::connectReturn(int errorCode, const ExitGames::Common::JString& errorString, const ExitGames::Common::JString& region, const ExitGames::Common::JString& cluster)
{
    qDebug() << "VrcPhotonClient::connectReturn()" << errorCode;
}

void VrcPhotonClient::disconnectReturn()
{
    qDebug() << "VrcPhotonClient::disconnectReturn()";
}

void VrcPhotonClient::leaveRoomReturn(int errorCode, const ExitGames::Common::JString& errorString)
{

}
