#ifndef SETTINGS_H
#define SETTINGS_H

#include <QString>
//#include <vector>
#include <set>

struct General {
    bool saveKey{false};
    QString key{""};
    bool sortMRU{true};
    QString accountUUID{""};
};

struct props {
    std::set<char> allowedCharacter{'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p',
                                      'q','r','s','t','u','v','w','x','y','z','A','B','C','D','E','F','G',
                                       'H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X',
                                       'Y','Z','0','1','2','3','4','5','6','7','8','9','@','_','$','&',
                                       '!','?','*','-'};
    int length{32};
};

struct Generator {
    props properties;
};

struct Database {
    bool purge{true};
    int numberOfDaysBeforePurge{30};
};

struct server {
    QString server{""};
    QString protocol{"https"};
    int port{1};
    QString db{""};
    QString userName{""};
    QString password{""};
};

struct SyncServers
{
    server remote;
    server local;
};


class Settings
{
public:
    Settings() = default;

    General general;
    Generator generator;
    Database database;
    SyncServers sync;

    void debugSettings();

};

#endif // SETTINGS_H
