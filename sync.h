#ifndef SYNC_H
#define SYNC_H

#include <string>
#include <queue>

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
//#include <QVariantMap>

#include <QSslConfiguration>
#include <QNetworkAccessManager>
#include <QNetworkRequest>

#include <QFile>
#include <QString>

#include <QThread>
#include <QWaitCondition>

#include "account.h"



// Sync Message structs
//
// Request Messages
enum RequestType { Register=1, Config=2, Delete=3, Sync_Initial=4, Sync_Final=5 };

struct BaseRequestMessage {
    RequestType type;
    QByteArray post;
    QString url;
};

struct RegisterRequestMessage : public BaseRequestMessage
{

    RegisterRequestMessage(QString email, QString password) {
        type = Register;
        url = "https://ec2-13-56-39-109.us-west-1.compute.amazonaws.com:8443/PassvaultServiceRegistration/service/registerV1/sync-server";

        QJsonObject root{};
        root.insert("email", QJsonValue{email});
        root.insert("password", QJsonValue{password});
        root.insert("version", QJsonValue{"v1.0"});
        QJsonDocument doc{root};
        post = doc.toJson();
    }
};

struct ConfigRequestMessage : public BaseRequestMessage
{
    QString email;
    QString password;

    ConfigRequestMessage(QString email, QString password) {
        type = Config;
        url = "https://ec2-13-56-39-109.us-west-1.compute.amazonaws.com:8443/PassvaultServiceRegistration/service/registerV1/sync-server";
        this->email = email;
        this->password = password;
    }
};

struct DeleteRequestMessage : public BaseRequestMessage
{
    DeleteRequestMessage(QString email, QString password) {
        type = Delete;
        url = "https://ec2-13-56-39-109.us-west-1.compute.amazonaws.com:8443/PassvaultServiceRegistration/service/deleteAccount/sync-server";
        QJsonObject root{};
        root.insert("user", QJsonValue{email});
        root.insert("password", QJsonValue{password});
        QJsonDocument doc{root};
        post = doc.toJson();
    }
};

struct SyncInitialRequestMessage : public BaseRequestMessage
{
    SyncInitialRequestMessage(QString email, QString password, const QVector<Account *> &accounts) {
        type = Sync_Initial;
        url = "https://ec2-13-56-39-109.us-west-1.compute.amazonaws.com:8443/PassvaultServiceRegistration/service/sync-accounts/sync-initial";
        QJsonObject root{};
        root["user"] = email;
        root["password"] = password;
        QJsonArray accts{};
        for (Account *val : accounts) {
            accts.append(QJsonObject{
                QPair<QString, QJsonValue>{"accountName", val->getAccountName()},
                QPair<QString, QJsonValue>{"updateTime", static_cast<double>(val->getUpdateTime())}
            });
        }
        root["accounts"] = accts;
        QJsonDocument doc{root};
        post = doc.toJson();
    }
};

struct SyncAccount
{
    QString accountName;
    QString userName;
    QString password;
    QString oldPassword;
    QString url;
    long updateTime;
    bool deleted;

    SyncAccount(QJsonObject obj)
    {
        accountName = obj["accountName"].toString();
        userName = obj["userName"].toString();
        password = obj["password"].toString();
        oldPassword = obj["oldPassword"].toString();
        url = obj["url"].toString();
        deleted = obj["deleted"].toBool();
        updateTime = static_cast<long>(obj["updateTime"].toDouble());
    }

    SyncAccount(Account *val)
    {
        accountName = val->getAccountName();
        userName = val->getUserName();
        password = val->getPassword();
        oldPassword = val->getOldPassword();
        url = val->getUrl();
        deleted = val->getDeleted();
        updateTime = val->getUpdateTime();
    }

    QJsonObject asJson() {
        QJsonObject obj{};
        obj["accountName"] = accountName;
        obj["userName"] = userName;
        obj["password"] = password;
        obj["oldPassword"] = oldPassword;
        obj["url"] = url;
        obj["deleted"] = deleted;
        obj["updateTime"] = static_cast<double>(updateTime);
        return obj;
    }
};

struct SyncFinalRequestMessage : public BaseRequestMessage
{
    SyncFinalRequestMessage(QString user, QString password, double lockTime, std::vector<SyncAccount> accounts)
    {
        type = RequestType::Sync_Final;
        url = "https://ec2-13-56-39-109.us-west-1.compute.amazonaws.com:8443/PassvaultServiceRegistration/service/sync-accounts/sync-final";
        QJsonObject root{};
        root["user"] = user;
        root["password"] = password;
        root["lockTime"] = lockTime;
        QJsonArray accts{};

        for (SyncAccount &account : accounts)
            accts.append(QJsonValue{account.asJson()});

        root["accounts"] = accts;
        QJsonDocument doc{root};
        post = doc.toJson();
    }
};



//
// Response Messages
struct ResponseMessage
{
    bool error{false};
    QString errorString{};
    RequestType type;
    QJsonDocument doc;

    ResponseMessage(QJsonDocument _doc, RequestType t) : type{t}, doc{_doc} {}
    ResponseMessage(QString err, RequestType t) : error{true}, errorString{err}, type{t} {}
};



class Sync : public QThread
{

    Q_OBJECT

public:
    Sync();
    ~Sync() override;
    void run() override;

    // sync calls
    void registerSyncAccount(const QString email, const QString password, const bool create);
    void deleteSyncAccount(const QString email, const QString password);
    void syncInitial(const QString email, const QString password, const QVector<Account *> &accounts);
    void syncFinal(const QString user, const QString password, const double lockTime, const std::vector<SyncAccount>);

signals:
    void resultReady(const ResponseMessage *);


private:
    QSslConfiguration sslConfig;

    QNetworkAccessManager *manager;
    QNetworkReply *reply;
    QByteArray *buffer;

    QMutex mutex;
    QWaitCondition cond;
    bool running;

    std::queue<BaseRequestMessage *> jobs;

    void submitJob(BaseRequestMessage *);
    QNetworkRequest * createRequest(QString url);
    bool checkForErrors();

    void initialize();
    void doPost(BaseRequestMessage *);
    void doGet(BaseRequestMessage *);

};

#endif // SYNC_H
