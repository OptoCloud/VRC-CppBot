#include <QCoreApplication>
#include "apiclient.h"
#include "photonclient.h"

#include <QDebug>

int main(int argc, char** argv)
{
    QCoreApplication a(argc, argv);
/*
    auto apiClient = std::make_shared<VRChad::ApiClient>();
    apiClient->login(VRChad::ApiClient::genAuthCookie("eirik.boee64@gmail.com", "Roboslayer9865"));

    std::shared_ptr<VRChad::PhotonClient> photonClient;

    QObject::connect(apiClient.get(), &VRChad::ApiClient::loginStatusChanged, [&](VRChad::ApiClient::LoginStatus loginStatus){
        if (loginStatus == VRChad::ApiClient::LoginStatus::LoggedIn) {
            photonClient = std::make_shared<VRChad::PhotonClient>(apiClient->clientVersion().toStdString(), apiClient->currentUserId().toStdString(), apiClient->photonAuthToken().toStdString(), apiClient->currentHwid().toStdString());

        }
    });
*/
    VRChad::PhotonClient cli("2021.4.2p2-1160--Release", "usr_34da7bcd-271a-4c41-b04a-c4f53b5738ba", "authcookie_8210ad58-d9ab-45be-aebf-368100db35de", "BB4725700AF1396D7C2263D53CF7F5947CB9CE9CDA");

    return a.exec();
}
