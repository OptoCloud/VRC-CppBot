#include <QCoreApplication>
#include "vrchttpclient.h"
#include "vrcphotonclient.h"

int main(int argc, char** argv)
{
    QCoreApplication a(argc, argv);

    VrcApiClient apiClient;
    apiClient.login(VrcApiClient::genAuthCookie("username", "password"));

    std::shared_ptr<VrcPhotonClient> photonClient;

    QObject::connect(&apiClient, &VrcApiClient::loginStatusChanged, [&](VrcApiClient::LoginStatus loginStatus){
        if (loginStatus == VrcApiClient::LoginStatus::LoggedIn) {
            photonClient = std::make_shared<VrcPhotonClient>(apiClient.currentUserId(), apiClient.photonAuthToken());

        }
    });

    return a.exec();
}
