#include "apiclient.h"

#include "fillrand.h"

#include <QtEndian>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>

#include <QNetworkCookie>
#include <QNetworkCookieJar>

QString VRChad::ApiClient::genAuthCookie(const QString& username, const QString& password)
{
    return "Basic " + (QUrl::toPercentEncoding(username) + ":" + QUrl::toPercentEncoding(password)).toBase64();
}

QString VRChad::ApiClient::genHardwareId()
{
    QByteArray bytes;
    bytes.resize(42);

    VRChad::Utils::FillRandHex(bytes.data(), bytes.size());

    return bytes;
}

VRChad::ApiClient::ApiClient(QObject* parent)
    : QObject(parent)
    , m_baseUrl("https://api.vrchat.cloud")
    , m_apiPath("/api/1/")
    , m_networkManager(new QNetworkAccessManager(this))
    , m_loginStatus(LoginStatus::LoggedOut)
    , m_hwid(genHardwareId())
    , m_userAgent("VRC.Core.BestHTTP")
    , m_unityVersion("2018.4.20f1")
    , m_clientVersion("2021.3.1p1-1114--Release")
    , m_apiHealthOk(false)
    , m_apiGotConfig(false)
    , m_serverName()
    , m_buildVersionTag()
    , m_token_apiKey()
    , m_token_2fa()
    , m_token_auth()
    , m_updateRateMsMaximum()
    , m_updateRateMsMinimum()
    , m_updateRateMsNormal()
    , m_updateRateMsUdonManual()
{
}

VRChad::ApiClient::LoginStatus VRChad::ApiClient::loginStatus() const
{
    return m_loginStatus;
}

QString VRChad::ApiClient::currentHwid() const
{
    return m_hwid;
}

QString VRChad::ApiClient::currentUserId() const
{
    return m_currentUserId;
}

QString VRChad::ApiClient::photonAuthToken() const
{
    return m_photonAuthToken;
}

void VRChad::ApiClient::login(QString authCookie)
{
    if (loginStatus() == LoginStatus::LoggedOut) {
        setLoginStatus(LoginStatus::LoggingIn);
        m_pendingAuth = authCookie;

        if (!m_apiHealthOk) {
            apiGetHealth();
            return;
        }

        if (!m_apiGotConfig) {
            apiGetConfig();
            return;
        }

        apiGetLogin();
    }
}

void VRChad::ApiClient::setLoginStatus(VRChad::ApiClient::LoginStatus status)
{
    if (m_loginStatus != status) {
        m_loginStatus = status;
        emit loginStatusChanged(status);
    }
}

QNetworkRequest VRChad::ApiClient::createApiRequest(const QString& ext, HttpContentType contentType, std::uint32_t contentLength, bool addQueries)
{
    QUrl url = m_baseUrl.resolved(m_apiPath + ext);

    if (addQueries) {
        QUrlQuery query;

        query.addQueryItem("organization", "vrchat");

        if (!m_token_apiKey.isEmpty()) {
            query.addQueryItem("apiKey", m_token_apiKey);
        }

        url.setQuery(query);
    }

    QNetworkRequest req(url);

    const char* contentTypeStr;
    switch (contentType) {
    case HttpContentType::UrlEncoded:
        contentTypeStr = "application/x-www-form-urlencoded";
        break;
    case HttpContentType::Json:
        contentTypeStr = "application/json";
        break;
    }


    // Set headers
    req.setHeader(QNetworkRequest::UserAgentHeader, m_userAgent.toUtf8());
    req.setHeader(QNetworkRequest::ContentTypeHeader, contentTypeStr);
    req.setHeader(QNetworkRequest::ContentLengthHeader, contentLength);

    req.setRawHeader("Accept", "*/*");
    req.setRawHeader("Accept-Encoding", "identity");
    //req.setRawHeader("X-MacAddress", m_hwid);

    return req;
}

void VRChad::ApiClient::apiGetHealth()
{
    auto req = createApiRequest("health", HttpContentType::UrlEncoded, 0, false);

    QNetworkReply* reply = m_networkManager->get(req);

    connect(reply, &QNetworkReply::finished, [this, reply]() {
        auto data = reply->readAll();
        auto doc = QJsonDocument::fromJson(data);

        if (doc.isObject()) {
            if (doc["ok"].toBool(false)) {
                qDebug() << "API Health OK!";
                m_serverName = doc["serverName"].toString();
                m_buildVersionTag = doc["buildVersionTag"].toString();

                m_apiHealthOk = true;
                if (loginStatus() == LoginStatus::LoggingIn) {
                    if (!m_apiGotConfig) {
                        apiGetConfig();
                        return;
                    }

                    apiGetLogin();
                }
            }
            else {
                m_apiHealthOk = false;
                qDebug() << "API Health is not ok!";
            }
        }
        else {
            m_apiHealthOk = false;
            qDebug() << "Failed to parse json response!";
            qDebug() << data;
        }
    });
    connect(reply, &QNetworkReply::sslErrors, this, &VRChad::ApiClient::onSslError);
    connect(reply, &QNetworkReply::errorOccurred, this, &VRChad::ApiClient::onNetworkError);
}

void VRChad::ApiClient::apiGetConfig()
{
    auto req = createApiRequest("config", HttpContentType::UrlEncoded, 0, true);

    QNetworkReply* reply = m_networkManager->get(req);

    connect(reply, &QNetworkReply::finished, [this, reply]() {
        auto data = reply->readAll();
        auto doc = QJsonDocument::fromJson(data);

        if (doc.isObject()) {
            m_token_apiKey = doc["apiKey"].toString();
            m_updateRateMsMaximum = doc["updateRateMsMaximum"].toInt();
            m_updateRateMsMinimum = doc["updateRateMsMinimum"].toInt();
            m_updateRateMsNormal = doc["updateRateMsNormal"].toInt();
            m_updateRateMsUdonManual = doc["updateRateMsUdonManual"].toInt();
            m_releaseServerVersionStandalone = doc["releaseServerVersionStandalone"].toString();

            m_networkManager->cookieJar()->updateCookie(QNetworkCookie("apiKey", m_token_apiKey.toUtf8()));

            m_apiGotConfig = true;
            qDebug() << "Got API config!";

            if (loginStatus() == LoginStatus::LoggingIn) {
                apiGetLogin();
            }
        }
        else {
            m_apiHealthOk = false;
            qDebug() << "Failed to parse json response!";
            qDebug() << data;
        }
    });
    connect(reply, &QNetworkReply::sslErrors, this, &VRChad::ApiClient::onSslError);
    connect(reply, &QNetworkReply::errorOccurred, this, &VRChad::ApiClient::onNetworkError);
}

void VRChad::ApiClient::apiGetLogin()
{
    if (m_pendingAuth.isEmpty()) {
        return;
    }

    m_token_auth.clear();

    auto req = createApiRequest("auth/user", HttpContentType::UrlEncoded, 0, true);

    req.setRawHeader("Authorization", m_pendingAuth.toUtf8());

    QNetworkReply* reply = m_networkManager->get(req);

    connect(reply, &QNetworkReply::finished, [this, reply]() {
        auto data = reply->readAll();
        auto doc = QJsonDocument::fromJson(data);

        if (doc.isObject()) {
            auto err = doc["error"];
            if (!err.isObject()) {
                m_token_auth = m_pendingAuth;
                m_pendingAuth.clear();

                QVariant cookieVar = reply->header(QNetworkRequest::SetCookieHeader);
                if (cookieVar.isValid()) {
                    QList<QNetworkCookie> cookies = cookieVar.value<QList<QNetworkCookie>>();
                    foreach (QNetworkCookie cookie, cookies) {
                        if (cookie.name() == "auth") {
                            m_photonAuthToken = cookie.value();
                            break;
                        }
                    }
                }

                m_currentUserId = doc["id"].toString();
                m_currentAvatarId = doc["currentAvatar"].toString();

                setLoginStatus(LoginStatus::LoggedIn);

                apiGetUserInfo(m_currentUserId);
            }
            else {
                qDebug() << QString("[%1] error: %2").arg(err["status_code"].toInt()).arg(err["message"].toString());
                setLoginStatus(LoginStatus::LoggedOut);
            }
        }
        else {
            qDebug() << "Failed to parse json response!";
            qDebug() << data;
            setLoginStatus(LoginStatus::LoggedOut);
        }
    });
    connect(reply, &QNetworkReply::sslErrors, this, &VRChad::ApiClient::onSslError);
    connect(reply, &QNetworkReply::errorOccurred, this, &VRChad::ApiClient::onNetworkError);
}

void VRChad::ApiClient::apiGetUserInfo(const QString& userId)
{
    auto req = createApiRequest("users/" + userId, HttpContentType::UrlEncoded, 0, true);

    QNetworkReply* reply = m_networkManager->get(req);

    connect(reply, &QNetworkReply::finished, [this, reply, userId]() {
        auto data = reply->readAll();
        auto doc = QJsonDocument::fromJson(data);

        if (doc.isObject()) {
            qDebug() << "Got user info!";
        }
        else {
            qDebug() << "Failed to parse json response!";
            qDebug() << data;
        }
    });
    connect(reply, &QNetworkReply::sslErrors, this, &VRChad::ApiClient::onSslError);
    connect(reply, &QNetworkReply::errorOccurred, this, &VRChad::ApiClient::onNetworkError);
}

void VRChad::ApiClient::apiGetWorldMetadata(const QString& worldId)
{
    auto req = createApiRequest("worlds/" + worldId + "/metadata", HttpContentType::UrlEncoded, 0, true);

    QNetworkReply* reply = m_networkManager->get(req);

    connect(reply, &QNetworkReply::finished, [this, reply, worldId]() {
        auto data = reply->readAll();
        auto doc = QJsonDocument::fromJson(data);

        if (doc.isObject()) {
            qDebug() << "Got world metadata!";
        }
        else {
            qDebug() << "Failed to parse json response!";
            qDebug() << data;
        }
    });
    connect(reply, &QNetworkReply::sslErrors, this, &VRChad::ApiClient::onSslError);
    connect(reply, &QNetworkReply::errorOccurred, this, &VRChad::ApiClient::onNetworkError);
}

void VRChad::ApiClient::apiPutPingWorld(const QString& userId, const QString& worldId)
{
    QJsonObject obj;
    obj.insert("userId", userId);
    obj.insert("worldId", worldId);
    QByteArray json = QJsonDocument(obj).toJson(QJsonDocument::Compact);

    auto req = createApiRequest("visits", HttpContentType::Json, json.length(), true);

    QNetworkReply* reply = m_networkManager->put(req, json);

    connect(reply, &QNetworkReply::finished, [this, reply, userId]() {
        auto data = reply->readAll();
        auto doc = QJsonDocument::fromJson(data);

        if (doc.isObject()) {
            qDebug() << "World ping success!";
        }
        else {
            qDebug() << "Failed to parse json response!";
            qDebug() << data;
        }
    });
    connect(reply, &QNetworkReply::sslErrors, this, &VRChad::ApiClient::onSslError);
    connect(reply, &QNetworkReply::errorOccurred, this, &VRChad::ApiClient::onNetworkError);
}

void VRChad::ApiClient::onSslError(const QList<QSslError>& errors)
{
    for (const QSslError& err : errors)
    {
        qDebug() << "SslError:" << err;
    }
}

void VRChad::ApiClient::onNetworkError(QNetworkReply::NetworkError err)
{
    qDebug() << "NetworkError:" << err;
}
