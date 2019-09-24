#include "accountsstore.h"
#include "utils.h"
#include "opensslaesengine.h"

#include <QSettings>
#include <QVariant>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QList>

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>



AccountsStore::AccountsStore(std::string key_)
{
    currentMRADay = Utils::currentMRATime();
    std::cout << "currentMRADay: " << currentMRADay << std::endl;
    QSettings qSettings{QSETTINGS_COMPANY_NAME, QSETTINGS_APP_NAME};
    storeLocation = qSettings.value("store_location",
                                    QVariant(DEFAULT_STORE_LOCATION)).toString();

    if (key_.size() <= 0)
        engine = new OpenSSLAESEngine{};
    else
        engine = new OpenSSLAESEngine{key_};

    loadSettings();
    settings.debugSettings();
    loadAccounts();

    shiftAccounts(currentMRADay);

    sync = new Sync{};
    createSyncCallBacks(sync);

}


AccountsStore::AccountsStore() : AccountsStore{""}
{
}


void AccountsStore::setStoreLocation(QString location)
{
    QSettings qSettings{QSETTINGS_COMPANY_NAME, QSETTINGS_APP_NAME};
    qSettings.setValue("store_location", QVariant{location});
    storeLocation = qSettings.value("store_location", QVariant(DEFAULT_STORE_LOCATION)).toString();
}


QString AccountsStore::getStoreLocation()
{
    QSettings qSettings{QSETTINGS_COMPANY_NAME, QSETTINGS_APP_NAME};
    return qSettings.value("store_location", QVariant(DEFAULT_STORE_LOCATION)).toString();
}


//std::vector<Account *>& AccountsStore::getAccounts()
QVector<Account *>& AccountsStore::getAccounts()
{
    return accounts;
}


Settings &AccountsStore::getSettings()
{
    return settings;
}


void AccountsStore::storeAccounts()
{
    saveAccounts();
}

void AccountsStore::storeSettings()
{
    saveSettings();
}


void AccountsStore::reloadSettings()
{
    loadSettings();
}


StatusDefs::Account_Status AccountsStore::addAccount(Account *account)
{
    for (Account *acct : accounts) {
        if (acct->getAccountName().toUpper() == account->getAccountName().toUpper()) {
            //if account exist but is deleted set MRA time to 0 and add new account
            if (acct->getDeleted())
                acct->setUpdateTime(0);
            else
                return StatusDefs::Account_Status::already_exists;
        }
    }

//    std::string encryptedPassword = engine->encryptPassword(account->getPassword().toStdString());
//    qDebug() << "Encrypted pword: " << QString::fromStdString(enc);

//    if (encryptedPassword == "") {
//        return StatusDefs::Account_Status::encryption_error;
//    }

//    account->setPassword(QString::fromStdString(encryptedPassword));
//    account->setOldPassword(QString::fromStdString(encryptedPassword));

    accounts.push_back(account);
    saveAccounts();
    numberOfAccounts++;
    sortAccounts();
    return StatusDefs::Account_Status::success;
}


StatusDefs::Account_Status AccountsStore::deleteAccount(Account *account)
{
    int position = -1;
    for (int i=0; i<accounts.size(); i++) {
        if (accounts[i]->getAccountName().toUpper() == account->getAccountName().toUpper() &&
                !accounts[i]->getDeleted()) {
            position = i;
            break;
        }
    }

    if (position == -1) {
        return StatusDefs::Account_Status::not_found;
    }

    //Update to remove deleted account from list
    if (settings.database.purge && settings.database.numberOfDaysBeforePurge <= 0) {
        accounts.erase(accounts.begin()+position);
        delete account;
    } else {
        accounts[position]->setUpdateTime(Utils::currentTimeMilli());
        accounts[position]->setDeleted(true);
    }

    //new version
//    account->setUpdateTime(Utils::currentTimeMilli());
//    account->setDeleted(true);
//    accounts.erase(accounts.begin()+position);

//    if (settings.database.purge && settings.database.numberOfDaysBeforePurge <= 0)
//        delete account;

    saveAccounts();
    numberOfAccounts--;

    return StatusDefs::Account_Status::success;
}


StatusDefs::Account_Status AccountsStore::updateAccount(Account *account)
{
    // should never get a pointer to an account that is not alread in the list, but check anyways
    int position = -1;
    for (int i=0; i<accounts.size(); i++) {
        if (accounts[i]->getAccountName().toUpper() == account->getAccountName().toUpper()) {
            position = i;
            break;
        }
    }

    if (position == -1) {
        return StatusDefs::Account_Status::not_found;
    }

    //these should always be the same
    accounts[position] = account;
    saveAccounts();
    return StatusDefs::Account_Status::success;
}


/*
 * Add thrwos later if error decrypting - done
 */
std::string AccountsStore::getPassword(Account *account)
{
    std::string password = account->getPassword().toStdString();
    account->getMap()[0]++;
    account->setMraTime(currentMRADay);

    if (settings.general.sortMRU)
        sortAccounts();

    if (password == "")
        return password;

    return engine->decryptPassword(password);
}

/*
 * Add thrwos later if error decrypting - dene
 */
std::string AccountsStore::getOldPassword(Account *account)
{
    std::string password = account->getOldPassword().toStdString();
    account->getMap()[0]++;
    account->setMraTime(currentMRADay);

    if (settings.general.sortMRU)
        sortAccounts();

    if (password == "")
        return password;

    return engine->decryptPassword(password);
}


int AccountsStore::getNumberOfAccounts() const
{
    return numberOfAccounts;
}


void AccountsStore::sortAccounts()
{
    std::sort(accounts.begin(), accounts.end(), [&] (Account * a, Account * b) -> bool {
        if (a->getDeleted())
            return false;
        if (b->getDeleted())
            return true;

        if (settings.general.sortMRU) {
            int a_cnt{0};
            int b_cnt{0};

            for (int i=0; i<Account::MRA_DAYS; i++) {
                a_cnt += a->getMap()[i];
                b_cnt += b->getMap()[i];

                // divide up by 7 day chunks
                if ((i+1) % 7 == 0) {
                    if (a_cnt > b_cnt)
                        return true;
                    else if (b_cnt > a_cnt)
                        return false;
                }
            }

            // if maps are the same then do alphabetic
            return a->getAccountName().toUpper() < b->getAccountName().toUpper();
        } else {
            return a->getAccountName().toUpper() < b->getAccountName().toUpper();
        }
    });

    //after sorting verify number of accounts not deleted
    numberOfAccounts = 0;
    for (Account *val : accounts)
        if (!val->getDeleted())
            numberOfAccounts++;
}


OpenSSLAESEngine * AccountsStore::getEncryptionEngine()
{
    return engine;
}


void AccountsStore::setEncryptionKey(std::string key)
{
    engine->setKey(key);

    for (Account *val : accounts) {
        bool correctKey{true};

        try {
            engine->decryptPassword(val->getPassword().toStdString());
        } catch (...) {
            correctKey = false;
        }

        val->setCorrectKey(correctKey);
    }
}


void AccountsStore::deleteRemoveSync(QString email, QString password, bool remove)
{
    // !remove = delete
    if (!remove) {
        OpenSSLAESEngine engine{};
        std::string decryptedPword = engine.decryptPassword(password.toStdString());
        sync->deleteSyncAccount(email, QString::fromStdString(decryptedPword));
    } else {
        removeSyncServer();
        saveSettings();
    }
}


void AccountsStore::syncAccounts()
{
    QString email = settings.sync.remote.userName;
    QString password = settings.sync.remote.password;

    if (email != "" && password != "") {
        //not a good way to do this but fine for a ui app
        accountsMap.clear();
        for (Account *val : accounts) {
            accountsMap.insert(val->getAccountName(), val);
            qDebug() << "acct: " << val->getAccountName() << ", deleted: " << val->getDeleted();
        }

        OpenSSLAESEngine e{};
        std::string decryptedPword = e.decryptPassword(password.toStdString());
        sync->syncInitial(email, QString::fromStdString(decryptedPword), accounts);
    } else {
        emit readyMessage("No Sync Account Setup");
    }
}


void AccountsStore::registerSync(QString email, QString password, bool create)
{
    sync->registerSyncAccount(email, password, create);
}


void AccountsStore::shiftAccounts(long currentMRADay)
{
    for (auto val : accounts) {
        val->shiftMRAMap(currentMRADay);
    }

    saveAccounts();
}


/*
 * JSON - Stuff
 */
void AccountsStore::saveStore(QJsonDocument &doc) {
    QFile qFile{storeLocation};

    //rotate backups - hardcoded to 10
    for (int i = 9; i > 0; --i) {
        if (QFile::exists(storeLocation + "." + QString::number(i))) {
            QFile::remove(storeLocation + "." + QString::number(i+1));
            QFile::rename(storeLocation + "." + QString::number(i),
                          storeLocation + "." + QString::number(i+1));
        }
    }
    QFile::copy(storeLocation, storeLocation + "." + QString::number(1));


    if (!qFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "Error: " << strerror(errno);
        return;
    }

    QJsonObject root = doc.object();
    root["version"] = root["version"].toInt() + 1;
    doc.setObject(root);
    QTextStream out{&qFile};
    out << doc.toJson(QJsonDocument::Indented);
    out.flush();
    qFile.close();
}


QJsonDocument AccountsStore::loadStore()
{
    QFile qFile{storeLocation};

    if (!qFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Error: " << strerror(errno);
        return QJsonDocument{};
    }

    QByteArray buffer = qFile.readAll();
    QJsonParseError *error = new QJsonParseError{};
    QJsonDocument doc = QJsonDocument::fromJson(buffer, error);
/*
 * ERROR - this will create a store without settings config !!!!
 * Fixed for now with a single recursive call, but this could end up in an endless recursive call
 */
    if (error->error != QJsonParseError::NoError) {
        qDebug() << "Json parse error: " << error->errorString();
        QString bkup{storeLocation + "." + QString::number(Utils::currentTimeMilli()) + ".bad"};
        qFile.copy(bkup);
        qDebug() << "File backed up to: " << bkup;
        //return QJsonDocument{};
        delete error;
        qFile.close();
        createStoreFile();
        return loadStore();
    }

    delete error;
    qFile.close();
    return doc;
}


void AccountsStore::loadAccounts()
{
    QFile qFile{storeLocation};

    if (!qFile.exists()) {
        qDebug() << "No Store found at, creating... " << storeLocation;
        createStoreFile();
    }

    qDebug() << "Loading store at: " << storeLocation;
    QJsonDocument doc = loadStore();

    for (Account *account : accounts) {
        delete account;
    }

    accounts.clear();
//    numberOfAccounts = 0;
    long now = Utils::currentTimeMilli();
    bool resaveAccountsIfDeleted = false;

    QJsonObject root = doc.object();
    QJsonArray array = root["accounts"].toArray();
    qDebug() << "accounts size=" << array.size();
    for (QJsonValueRef ref : array) {
        QJsonObject account = ref.toObject();

        //old way - now not adding deleted accounts to list even if they arent purged
        //remove any deleted accounts if purge is set - simply don't add them and save accounts at end
        if (account["Deleted"].toBool()) {
            if (settings.database.purge) {
                long timePast = now - static_cast<long>(account["UpdateTime"].toDouble());

                if (timePast > (settings.database.numberOfDaysBeforePurge * MILLI_IN_DAY)) {
                    qDebug() << "purging deleted account: " << account["AccountName"].toString();
                    resaveAccountsIfDeleted = true;
                    continue;
                }
            }
        } else {
//            numberOfAccounts++;
        }
        //new version *****Need to create a purge routine that only runs at startup****
//        if (account["Deleted"].toBool()) {
//            continue;
//        }


        QJsonArray array = account["map"].toArray();
        std::vector<int> vec{};

        for (int i=0; i< array.size(); i++)
            vec.push_back(array[i].toInt());

        bool correctKey{true};

        try {
            engine->decryptPassword(account["Password"].toString().toStdString());
        } catch (...) {
            correctKey = false;
        }

        accounts.push_back(new Account{account["AccountName"].toString(),
                           account["UserName"].toString(),
                           account["Password"].toString(),
                           account["OldPassword"].toString(),
                           account["URL"].toString(),
                           static_cast<long>(account["UpdateTime"].toDouble()),
                           account["Deleted"].toBool(),
                           static_cast<long>(account["mraTime"].toDouble()),
                           vec, correctKey});

    }


    sortAccounts();

    if (currentMRADay < Utils::currentMRATime()) {
        currentMRADay = Utils::currentMRATime();
        shiftAccounts(currentMRADay);
    }

    // if deleted accounts need to be purged, resave new account list
    if (resaveAccountsIfDeleted)
        saveAccounts();
}


void AccountsStore::saveAccounts()
{
    QJsonArray array{};

    for(Account *val : accounts) {
        QJsonObject account{};
        account["AccountName"] = val->getAccountName();
        account["UserName"] = val->getUserName();
        account["Password"] = val->getPassword();
        account["OldPassword"] = val->getOldPassword();
        account["URL"] = val->getUrl();
        account["Deleted"] = val->getDeleted();
        account["UpdateTime"] = static_cast<double>(val->getUpdateTime());
        account["mraTime"] = static_cast<double>(val->getMraTime());
        QJsonArray map_array{};

        for (size_t i=0; i<val->getMap().size(); i++)
            map_array.push_back(val->getMap()[i]);

        account["map"] = map_array;
        array.push_back(account);
    }

    QJsonDocument doc = loadStore();
    QJsonObject root = doc.object();
    root["accounts"] = array;
    doc.setObject(root);
    saveStore(doc);
}


void AccountsStore::loadSettings()
{
    QFile qFile{storeLocation};

    if (!qFile.exists())
        return;

    QJsonDocument doc = loadStore();
    QJsonObject setObj = doc.object()["settings"].toObject();
    settings.general.key = setObj["general"].toObject()["key"].toString();
    settings.general.saveKey = setObj["general"].toObject()["saveKey"].toBool();
    settings.general.sortMRU = setObj["general"].toObject()["sortMRU"].toBool();
    settings.general.accountUUID = setObj["general"].toObject()["accountUUID"].toString();

//    settings.generator.overRide = setObj["generator"].toObject()["overRide"].toBool();
    settings.generator.properties.length =
            setObj["generator"].toObject()["properties"].toObject()["length"].toInt();
    QJsonArray props =
            setObj["generator"].toObject()["properties"].toObject()["allowedCharacter"].toArray();
    settings.generator.properties.allowedCharacter.clear();
    for (QJsonValue v : props)
        settings.generator.properties.allowedCharacter.emplace(static_cast<char>(v.toInt()));

    settings.database.purge = setObj["database"].toObject()["purge"].toBool();
    settings.database.numberOfDaysBeforePurge =
            setObj["database"].toObject()["numberOfDaysBeforePurge"].toInt();

    settings.sync.local.server = setObj["sync"].toObject()["local"].toObject()["server"].toString();
    settings.sync.local.protocol = setObj["sync"].toObject()["local"].toObject()["protocol"].toString();
    settings.sync.local.port = setObj["sync"].toObject()["local"].toObject()["port"].toInt();
    settings.sync.local.db = setObj["sync"].toObject()["local"].toObject()["db"].toString();
    settings.sync.local.userName = setObj["sync"].toObject()["local"].toObject()["userName"].toString();
    settings.sync.local.password = setObj["sync"].toObject()["local"].toObject()["password"].toString();

    settings.sync.remote.server = setObj["sync"].toObject()["remote"].toObject()["server"].toString();
    settings.sync.remote.protocol = setObj["sync"].toObject()["remote"].toObject()["protocol"].toString();
    settings.sync.remote.port = setObj["sync"].toObject()["remote"].toObject()["port"].toInt();
    settings.sync.remote.db = setObj["sync"].toObject()["remote"].toObject()["db"].toString();
    settings.sync.remote.userName = setObj["sync"].toObject()["remote"].toObject()["userName"].toString();
    settings.sync.remote.password = setObj["sync"].toObject()["remote"].toObject()["password"].toString();
}


void AccountsStore::saveSettings()
{
    QJsonObject jsonSettings{};

    QJsonObject general{};
    general["key"] = QJsonValue{settings.general.key};
    general["saveKey"] = QJsonValue{settings.general.saveKey};
    general["sortMRU"] = QJsonValue{settings.general.sortMRU};
    general["accountUUID"] = QJsonValue{settings.general.accountUUID};
    jsonSettings["general"] = general;

    QJsonObject generator{};
//    generator["overRide"] = QJsonValue{settings.generator.overRide};
    QJsonObject properties{};
    QJsonArray array{};
    for (char val : settings.generator.properties.allowedCharacter)
        array.push_back(QJsonValue{val});
    properties["allowedCharacter"] = array;
    properties["length"] = QJsonValue{settings.generator.properties.length};
    generator["properties"] = properties;
    jsonSettings["generator"] = generator;

    QJsonObject database{};
    database["purge"] = QJsonValue{settings.database.purge};
    database["numberOfDaysBeforePurge"] = QJsonValue{settings.database.numberOfDaysBeforePurge};
    jsonSettings["database"] = database;

    QJsonObject sync{};
    QJsonObject remote{};
    remote["server"] = QJsonValue{settings.sync.remote.server};
    remote["port"] = QJsonValue{settings.sync.remote.port};
    remote["protocol"] = QJsonValue{settings.sync.remote.protocol};
    remote["db"] = QJsonValue{settings.sync.remote.db};
    remote["userName"] = QJsonValue{settings.sync.remote.userName};
    remote["password"] = QJsonValue{settings.sync.remote.password};
    sync["remote"] = remote;

    QJsonObject local{};
    local["server"] = QJsonValue{settings.sync.local.server};
    local["port"] = QJsonValue{settings.sync.local.port};
    local["protocol"] = QJsonValue{settings.sync.local.protocol};
    local["db"] = QJsonValue{settings.sync.local.db};
    local["userName"] = QJsonValue{settings.sync.local.userName};
    local["password"] = QJsonValue{settings.sync.local.password};
    sync["local"] = local;
    jsonSettings["sync"] = sync;

    QJsonDocument doc = loadStore();
    QJsonObject root = doc.object();
    root["settings"] = jsonSettings;
    doc.setObject(root);
    saveStore(doc);
}


void AccountsStore::createStoreFile()
{
    QJsonDocument doc{};
    QJsonObject root{};
    root["format"] = QJsonValue{1.1};
    root["version"] = QJsonValue{1};

    root["accounts"] = QJsonArray{};
    doc.setObject(root);

//    qDebug() << "doc=" << QString{doc.toJson(QJsonDocument::Indented)};
    saveStore(doc);
    saveSettings();
}


void AccountsStore::removeSyncServer()
{
    settings.sync.remote.server = "";
    settings.sync.remote.protocol = "";
    settings.sync.remote.port = -1;
    settings.sync.remote.db = "";
    settings.sync.remote.userName = "";
    settings.sync.remote.password = "";
}


void AccountsStore::createSyncCallBacks(Sync *sync)
{
    connect(sync, &Sync::resultReady, [&](const ResponseMessage *resp) {
        if (resp->error) {
            qDebug() << "Error from Sync: " << resp->errorString;

            if (resp->type == RequestType::Delete) {
                this->removeSyncServer();
                this->saveSettings();
            }

            emit readyMessage("Sync Server Error: " + resp->errorString);
        } else if (resp->type == RequestType::Register || resp->type == RequestType::Config) {
            qDebug() << "Result from sync: \n" << resp->doc ;
            QJsonObject setObj = resp->doc.object();

            settings.sync.remote.server = setObj["server"].toString();
            settings.sync.remote.protocol = setObj["protocol"].toString();
            settings.sync.remote.port = setObj["port"].toInt();
            settings.sync.remote.db = setObj["bucket"].toString();
            settings.sync.remote.userName = setObj["userName"].toString();

            OpenSSLAESEngine engine{};
            std::string encryptedPassword = engine.encryptPassword(setObj["password"].toString().toStdString());
            settings.sync.remote.password = QString::fromStdString(encryptedPassword);
            this->saveSettings();
            emit readyMessage("Register Success");
        } else if (resp->type == RequestType::Delete) {
            qDebug() << "Result from sync: \n" << resp->doc ;
            this->removeSyncServer();
            this->saveSettings();
            emit readyMessage("Sync Account Delete Success");
        } else if (resp->type == RequestType::Sync_Initial) {
            qDebug() << "Result from sync initial: \n" << resp->doc ;
            std::vector<SyncAccount> accountsFromSync{};

            for (QJsonValueRef ref : resp->doc["accountsToSendBackToClient"].toArray()) {
                //accountsFromSync.push_back(SyncAccount{ref.toObject()});
                SyncAccount a{ref.toObject()};

                if (!accountsMap.contains(a.accountName)) {
                    // if local store does not have sent account which is delete ignore
                    if (!a.deleted)
                        accountsMap[a.accountName] = new Account{a.accountName, a.userName, a.password, a.oldPassword,
                                                             a.url, a.updateTime};
                } else {
                    //if existing sentback account is deleted remove it from the store
                    if (a.deleted) {
                        Account *to_del = accountsMap[a.accountName];
                        accountsMap.remove(a.accountName);
                       delete to_del;
                    } else {
                        Account *acct = accountsMap[a.accountName];
                        acct->setUserName(a.userName);
                        acct->setPassword(a.password);
                        acct->setOldPassword(a.oldPassword);
                        acct->setUrl(a.url);
                        acct->setDeleted(a.deleted);
                        acct->setUpdateTime(a.updateTime);
                    }
                }
            }
//implement some type of lock
            accounts.clear();
            accounts = QVector<Account*>::fromList(accountsMap.values());

            // save store at halfway point in case the next call fails
            this->sortAccounts();
            this->saveAccounts();


            // accounts to send back to server
            std::vector<SyncAccount> accountsToSync{};

            for (QJsonValueRef ref : resp->doc["sendAccountsToServerList"].toArray())
                accountsToSync.push_back(SyncAccount{accountsMap[ref.toString()]});

           this->sync2 = new Sync{};
           this->createSyncCallBacks(sync2);
            OpenSSLAESEngine e{};
            std::string decryptedPword = e.decryptPassword(settings.sync.remote.password.toStdString());

            sync2->syncFinal(settings.sync.remote.userName, QString::fromStdString(decryptedPword),
                    resp->doc["lockTime"].toDouble(), accountsToSync);

        } else if (resp->type == RequestType::Sync_Final) {
            // at this point any original accounts that were deleted can be removed and then save store again
            for (auto val : accountsMap.keys())
                if (accountsMap[val]->getDeleted())
                    accountsMap.remove(val);

            accounts.clear();
            accounts = QVector<Account*>::fromList(accountsMap.values());
            this->sortAccounts();
            this->saveAccounts();

            qDebug() << "Sync Complete: \n";
            emit readyMessage("Sync Complete...");
            delete sync2;
        }

        delete resp;
    });
}

