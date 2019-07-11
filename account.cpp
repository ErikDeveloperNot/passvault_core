#include "account.h"
#include "utils.h"
#include <iostream>

Account::Account()
{

}

/*
 * called for creating a new account
 */
Account::Account(QString _accountName, QString _userName, QString _password,
                 QString _oldPassword, QString _url, long _updateTime) :
    QStandardItem(_accountName), accountName{_accountName}, userName{_userName},
    password{_password}, oldPassword{_oldPassword}, url{_url}, updateTime{_updateTime},
    correctKey{true}
{

    for (size_t i=0; i<MRA_DAYS; i++)
        map.push_back(0);

    mraTime = Utils::currentMRATime();

    if (updateTime == -1)
        updateTime = Utils::currentTimeMilli();

    deleted = false;
}

/*
 * called for loading an account from database
 */
Account::Account(QString _accountName, QString _userName, QString _password,
                 QString _oldPassword, QString _url, long _updateTime, bool _deleted,
                 long _mraTime, std::vector<int> _map, bool _correctKey) :
    QStandardItem(_accountName), accountName{_accountName}, userName{_userName},
    password{_password}, oldPassword{_oldPassword}, url{_url}, updateTime{_updateTime},
    mraTime{_mraTime}, deleted{_deleted}, correctKey{_correctKey}, map{_map}
{

}

QString Account::getAccountName() const
{
    return accountName;
}

QString Account::getUserName() const
{
    return userName;
}

void Account::setUserName(const QString &value)
{
    userName = value;
    updateTime = Utils::currentTimeMilli();
}

QString Account::getPassword()
{
    return password;
}

void Account::setPassword(const QString &value)
{
    oldPassword = password;
    password = value;
    updateTime = Utils::currentTimeMilli();
}

QString Account::getOldPassword()
{
    return oldPassword;
}

void Account::setOldPassword(const QString &value)
{
    oldPassword = value;
}

QString Account::getUrl() const
{
    return url;
}

void Account::setUrl(const QString &value)
{
    url = value;
    updateTime = Utils::currentTimeMilli();
}

long Account::getUpdateTime() const
{
    return updateTime;
}

void Account::setUpdateTime(long value)
{
    updateTime = value;
}

long Account::getMraTime() const
{
    return mraTime;
}

void Account::setMraTime(long value)
{
    mraTime = value;
}


void Account::shiftMRAMap(long currentMRATime)
{
    long daysToShift = currentMRATime - mraTime;

    if (daysToShift > 0) {
        if (daysToShift >= MRA_DAYS) {
            map.clear();

            for (size_t i=0; i<MRA_DAYS; i++)
                map.push_back(0);
        } else {
            int insert = MRA_DAYS - 1;

            for (int i=MRA_DAYS-daysToShift-1; i>=0; i--) {
                map[insert--] = map[i];
                map[i] = 0;
            }
        }
    }
}

std::vector<int>& Account::getMap()
{
    return map;
}

void Account::setMap(const std::vector<int> value)
{
    map = value;
}

bool Account::getDeleted() const
{
    return deleted;
}

void Account::setDeleted(bool value)
{
    deleted = value;
}

bool Account::getCorrectKey() const
{
    return correctKey;
}

void Account::setCorrectKey(bool value)
{
    correctKey = value;
}

