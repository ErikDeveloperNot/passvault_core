#include "sync.h"

#include <QNetworkRequest>
#include <QNetworkReply>

#include <QDebug>


Sync::Sync() : running{true}
{
}


Sync::~Sync()
{
    qDebug() << "\n\nDestroying Sync\n\n";
    //think i need to verify they are not null ?
    mutex.lock();
    running = false;

    while(!jobs.empty())
        cond.wait(&mutex);

    jobs.push(new RegisterRequestMessage{"bogus", "wont process"});

    cond.wakeOne();
    mutex.unlock();
    wait();

    if (buffer)
        delete buffer;
//    if (reply)
//        delete reply;
    if (manager)
        delete manager;

    qDebug() << "\n\nDone Destroying Sync\n\n";
}


void Sync::run()
{
    qDebug() << "Starting Sync::run";

    initialize();
    mutex.lock();

    while (running) {
        BaseRequestMessage *job = jobs.front();
        jobs.pop();

        if (job->type == RequestType::Config)
            doGet(job);
        else
            doPost(job);

qDebug() << "JOB type: " << job->type;
        delete job;

        qDebug() << "starting exec()";
//mutex.unlock();
        exec();
        qDebug() << "exec() stopped, now cond.wait()";

        while (jobs.empty())
            cond.wait(&mutex);

        qDebug() << "done with wait";
    }

    qDebug() << "Exiting Sync::run";
}


void Sync::doPost(BaseRequestMessage * job)
{
    qDebug() << "starting doPost()";
    buffer->clear();
    QNetworkRequest *request = createRequest(job->url);

    RequestType type = job->type;
    reply = manager->post(*request, job->post);

    connect(reply, &QIODevice::readyRead, [&]() {
        qDebug() << "Data Recived: " << reply->size();
        buffer->append(reply->readAll());
qDebug() << "s:\n" << *buffer;
QJsonParseError error;
QJsonDocument::fromJson(*buffer, &error);
qDebug() << error.errorString() << ", offset: " << error.offset;

    });

        connect(reply, &QNetworkReply::finished, [=]() {

            if (reply->error()) {
               qDebug() << "errorString: " << reply->errorString();
               QString err{*buffer};
               emit resultReady(new ResponseMessage{err, type});
           } else {
               qDebug() << "Calling emit";
 qDebug() << *buffer;
 QJsonParseError error;
 QJsonDocument::fromJson(*buffer, &error);
 qDebug() << error.errorString() << ", offset: " << error.offset;

               emit resultReady(new ResponseMessage{QJsonDocument::fromJson(*buffer), type});
           }
           reply->deleteLater();
           qDebug() << "done with emit, calling quit";
           quit();
        });

    delete request;
}


void Sync::doGet(BaseRequestMessage * job)
{
    /*
     * GET is only used to get config for an existing account
     */
    qDebug() << "starting doGet()";
    buffer->clear();
    QNetworkRequest *request = createRequest(job->url);

    ConfigRequestMessage *configMsg = static_cast<ConfigRequestMessage*>(job);
    QString *user = new QString{configMsg->email};
    QString *pw = new QString{configMsg->password};

    reply = manager->get(*request);

    connect(reply, &QIODevice::readyRead, [&]() {
        qDebug() << "Data Recived: " << reply->size();
        buffer->append(reply->readAll());
    });

    connect(reply, &QNetworkReply::finished, [=]() {
           if (reply->error()) {
               qDebug() << "errorString: " << reply->errorString();
               QString err{*buffer};
               emit resultReady(new ResponseMessage{err, RequestType::Register});
           } else {
               QJsonObject obj = QJsonDocument::fromJson(*buffer).object();
               obj["userName"] = *user;
               obj["password"] = *pw;
               emit resultReady(new ResponseMessage{QJsonDocument{obj}, RequestType::Register});
           }

           delete user;
           delete pw;
           reply->deleteLater();
           quit();
    });

    delete request;
}


void Sync::registerSyncAccount(QString email, QString password, bool create)
{
    qDebug() << "Running registerSyncAccount--";

    if (create)
        submitJob(new RegisterRequestMessage{email, password});
    else
        submitJob(new ConfigRequestMessage{email, password});

}


void Sync::deleteSyncAccount(QString email, QString password)
{
    qDebug() << "Running deleteSyncAccount--";
    submitJob(new DeleteRequestMessage{email, password});

}


void Sync::syncInitial(const QString email, const QString password, const QVector<Account *> &accounts)
{
    SyncInitialRequestMessage *syncMsg = new SyncInitialRequestMessage{email, password, accounts};
//    qDebug() << "Running SyncInitial 1--";
    submitJob(syncMsg);
//    qDebug() << "Running SyncInitial 2--";
}


void Sync::syncFinal(const QString user, const QString password, const double lockTime, const std::vector<SyncAccount> accountsToSync)
{
    SyncFinalRequestMessage *syncMsg = new SyncFinalRequestMessage{user, password, lockTime, accountsToSync};
    qDebug() << "Running SyncFinal--";
    submitJob(syncMsg);
}


void Sync::submitJob(BaseRequestMessage *job)
{
    QMutexLocker locker(&mutex);
qDebug() << "Got Lock";
    jobs.push(job);

    if (!isRunning())
        start();
    else
        cond.wakeOne();
}


QNetworkRequest * Sync::createRequest(QString url)
{
    QNetworkRequest *request = new QNetworkRequest{QUrl{url}};
    request->setSslConfiguration(sslConfig);
    request->setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader, "application/json");
    return request;
}


void Sync::initialize()
{
    manager = new QNetworkAccessManager{};
    buffer = new QByteArray{};

    QFile cert{":/cert.pem"};
//    QFile cert{"/Users/user1/Qt_projects/passvault/passvault_core/passvault_core/cert.pem"};
//    cert.open(QIODevice::ReadOnly);
    qDebug() << "open:" << cert.open(QIODevice::ReadOnly);
    QByteArray bytes = cert.readAll();
    auto certs = QSslCertificate::fromData(bytes, QSsl::Pem);

    sslConfig.setCaCertificates(certs);
}



