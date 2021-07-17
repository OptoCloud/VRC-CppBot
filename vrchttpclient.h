#ifndef VRCHTTPCLIENT_H
#define VRCHTTPCLIENT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class VrcApiClient : public QObject {
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
    static QString genWorldLink(const QString& worldId, std::uint32_t lobbyId, Privacy privacy, Region region, const QString& userId);
    static QByteArray genHardwareId();

    VrcApiClient(QObject* parent = nullptr);

    LoginStatus loginStatus() const;
    QString currentUserId() const;
    QString photonAuthToken() const;

    void login(QString authCookie);
signals:
    void gotApiHealthOk();
    void gotApiConfig();
    void loginStatusChanged(LoginStatus);
private:
    void setLoginStatus(LoginStatus status);

    enum HttpContentType {
        UrlEncoded,
        Json,
    };

    QNetworkRequest createApiRequest(const QString& ext, HttpContentType contentType, std::uint32_t contentLength, bool addQueries);

    void apiGetHealth();
    void apiGetConfig();
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

    QByteArray m_hwid;
    QString m_userAgent;
    QString m_unityVersion;
    QString m_clientVersion;

    bool m_apiHealthOk;
    bool m_apiGotConfig;
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

#endif // VRCHTTPCLIENT_H
