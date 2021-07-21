#include <QCoreApplication>
#include "apiclient.h"
#include "photonclient.h"

int main(int argc, char** argv)
{
    QCoreApplication a(argc, argv);

    /*
    VRChad::ApiClient apiClient;
    apiClient.login(VRChad::ApiClient::genAuthCookie("username", "password"));

    std::shared_ptr<VRChad::PhotonClient> photonClient;

    QObject::connect(&apiClient, &VRChad::ApiClient::loginStatusChanged, [&](VRChad::ApiClient::LoginStatus loginStatus){
        if (loginStatus == VRChad::ApiClient::LoginStatus::LoggedIn) {
            photonClient = std::make_shared<VRChad::PhotonClient>(apiClient.currentUserId().toStdWString(), apiClient.photonAuthToken().toStdWString());

        }
    });
    */

    VRChad::PhotonClient cli(L"usr_0c5507b7-fdee-4101-a0e1-319645de64de", L"authcookie_68416684-a229-4897-9150-41f951e420cd");

    return a.exec();
}
