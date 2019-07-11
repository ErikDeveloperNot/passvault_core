#ifndef ACCOUNTSSTORE_H
#define ACCOUNTSSTORE_H

#include <vector>
#include <QString>
#include <QVector>

#include "account.h"
#include "settings.h"
#include "statusdefs.h"
#include "opensslaesengine.h"
#include "sync.h"



class AccountsStore : public QObject
{
    Q_OBJECT
public:
    AccountsStore(std::string);
    AccountsStore();

    void setStoreLocation(QString);
    QString getStoreLocation();

//    std::vector<Account*>& getAccounts();
    QVector<Account *>& getAccounts();

    Settings& getSettings();

    void storeAccounts();
    void storeSettings();
    void reloadSettings();

    StatusDefs::Account_Status addAccount(Account*);
    StatusDefs::Account_Status deleteAccount(Account*);
    StatusDefs::Account_Status updateAccount(Account*);

    std::string getPassword(Account*);
    std::string getOldPassword(Account*);

    int getNumberOfAccounts() const;

    void sortAccounts();

    OpenSSLAESEngine* getEncryptionEngine();
    void setEncryptionKey(std::string);

    void registerSync(QString email, QString password, bool create);
    void deleteRemoveSync(QString email, QString password, bool remove);  //go back and change so email/pass need not pass in
    void syncAccounts();

private:
    const QString DEFAULT_STORE_LOCATION = "./passvault_store.json";
    const QString QSETTINGS_COMPANY_NAME = "dNot";
    const QString QSETTINGS_APP_NAME = "passvault";
    const long MILLI_IN_DAY = 86400000;

    long currentMRADay;

//    std::vector<Account*> accounts;
    QVector<Account *> accounts;
    QMap<QString, Account *> accountsMap;
    int numberOfAccounts;

    Settings settings;
    QString storeLocation;
    OpenSSLAESEngine *engine;
    Sync *sync;
    Sync *sync2;

    void shiftAccounts(long currentMRADay);

    //JSON methods
    void saveStore(QJsonDocument &);
    QJsonDocument loadStore();
    void createStoreFile();

    void loadAccounts();
    void saveAccounts();

    void loadSettings();
    void saveSettings();

    void removeSyncServer();

    void createSyncCallBacks(Sync *sync);

signals:
    void readyMessage(const QString &);

};



#endif // ACCOUNTSSTORE_H
