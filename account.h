#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <iostream>
#include <QStandardItem>
#include <vector>

class Account : public QStandardItem
{
public:
    static constexpr int MRA_DAYS = 28;

    Account();
    Account(QString _accountName, QString _userName, QString _password,
            QString _oldPassword, QString _url, long _updateTime=-1);
    Account(QString _accountName, QString _userName, QString _password,
            QString _oldPassword, QString _url, long _updateTime, bool _deleted,
            long _mraTime, std::vector<int> _map, bool _correctKey);

    QString getAccountName() const;

    QString getUserName() const;
    void setUserName(const QString &value);

    QString getPassword();
    void setPassword(const QString &value);

    QString getOldPassword();
    void setOldPassword(const QString &value);

    QString getUrl() const;
    void setUrl(const QString &value);

    long getUpdateTime() const;
    void setUpdateTime(long value);

    long getMraTime() const;
    void setMraTime(long value);
    void shiftMRAMap(long currentMRATime);

    std::vector<int>& getMap();
    void setMap(const std::vector<int>);

    bool getDeleted() const;
    void setDeleted(bool value);

    bool getCorrectKey() const;
    void setCorrectKey(bool value);

private:
    QString accountName;
    QString userName;
    QString password;
    QString oldPassword;
    QString url;
    long updateTime;
    long mraTime;
    bool deleted;
    bool correctKey;
    std::vector<int> map;
};

#endif // ACCOUNT_H
