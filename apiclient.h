#ifndef APICLIENT_H
#define APICLIENT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>

namespace VRChad {
class ApiClient : public QObject {
    Q_OBJECT
public:
    enum class LoginStatus
    {
        LoggedOut,
        LoggingOut,
        LoggingIn,
        LoggedIn
    };
    enum class Privacy
    {
        Public,
        FriendsPlus,
        Friends,
        InvitePlus,
        Invite
    };
    enum class Region
    {
        US,
        EU,
        JP
    };

    static QString genAuthCookie(const QString& username, const QString& password);
    static QString genHardwareId();

    ApiClient(QObject* parent = nullptr);

    LoginStatus loginStatus() const;
    QString currentHwid() const;
    QString currentUserId() const;
    QString photonAuthToken() const;

    QString clientVersion() const;
    QString photonServerName() const;

    void login(QString authCookie);
signals:
    void gotApiConfig();
    void gotPhotonConfig();
    void loginStatusChanged(VRChad::ApiClient::LoginStatus);
private:
    void setLoginStatus(LoginStatus status);

    enum HttpContentType {
        UrlEncoded,
        Json,
    };

    QNetworkRequest createApiRequest(const QString& ext, HttpContentType contentType, std::uint32_t contentLength, bool addQueries);

    void ensureConfigs(std::function<void()> needsConfigs);

    void getPhotonConfig(std::function<void()> needsConfig);
    void apiGetConfig(std::function<void()> needsConfig);

    void apiGetLogin();

    void apiGetUserInfo(const QString& userId);
    void apiGetWorldMetadata(const QString& worldId);
    void apiPutPingWorld(const QString& userId, const QString& worldId);

    void onSslError(const QList<QSslError>&);
    void onNetworkError(QNetworkReply::NetworkError);

    QUrl m_baseUrl;
    QString m_apiPath;

    QNetworkAccessManager* m_networkManager;

    LoginStatus m_loginStatus;
    QString m_pendingAuth;

    QString m_hwid;
    QString m_userAgent;
    QString m_unityVersion;
    QString m_clientVersion;
    QString m_photonServerName;

    bool m_apiGotConfig;
    bool m_photonGotConfig;
    QString m_serverName;
    QString m_buildVersionTag;

    QString m_token_apiKey;
    QString m_token_2fa;
    QString m_token_auth;

    int m_updateRateMsMaximum;
    int m_updateRateMsMinimum;
    int m_updateRateMsNormal;
    int m_updateRateMsUdonManual;
    QString m_releaseServerVersionStandalone;

    QString m_photonAuthToken;
    QString m_currentUserId;
    QString m_currentAvatarId;
};
}

#endif // APICLIENT_H
