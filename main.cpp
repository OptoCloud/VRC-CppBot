#include <QCoreApplication>
#include "apiclient.h"
#include "photonclient.h"

int main(int argc, char** argv)
{
    QCoreApplication a(argc, argv);

    /*
    auto apiClient = std::make_shared<VRChad::ApiClient>();
    apiClient->login(VRChad::ApiClient::genAuthCookie("username", "password"));

    std::shared_ptr<VRChad::PhotonClient> photonClient;

    QObject::connect(apiClient.get(), &VRChad::ApiClient::loginStatusChanged, [&](VRChad::ApiClient::LoginStatus loginStatus){
        if (loginStatus == VRChad::ApiClient::LoginStatus::LoggedIn) {
            photonClient = std::make_shared<VRChad::PhotonClient>(apiClient->currentUserId().toStdWString(), apiClient->photonAuthToken().toStdWString(), apiClient->currentHwid().toStdWString());

        }
    });
    */

    VRChad::PhotonClient cli(L"usr_0c5507b7-fdee-4101-a0e1-319645de64de", L"authcookie_0e0744a2-878d-4de0-9664-c186a908ebd9", L"440B2A8E26703A65A04A5318C487D83230938853BF");

    return a.exec();
}
