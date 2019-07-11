#include "settings.h"
#include <QDebug>


void Settings::debugSettings()
{
    qDebug() << "--- Settings ---";
    qDebug() << "  General:";
    qDebug() << "    key: " << general.key;
    qDebug() << "    saveKey: " << general.saveKey;
    qDebug() << "    sortMRU: " << general.sortMRU;
    qDebug() << "    accountUUID: " << general.accountUUID;
    qDebug() << "";
    qDebug() << "  Generator:";
    qDebug() << "    length: " << generator.properties.length;
    qDebug() << "    allowedCharacter: ";// << generator.properties.allowedCharacter;
    QString chars{"[ "};

    for (auto it = generator.properties.allowedCharacter.begin(); it != generator.properties.allowedCharacter.end(); it++)
        chars.append(*it).append(' ');
    chars.append(']');
    qDebug() << chars;

    qDebug() << "";
    qDebug() << "  Database:";
    qDebug() << "    purge: " << database.purge;
    qDebug() << "    numberOfDaysBeforePurge: " << database.numberOfDaysBeforePurge;
    qDebug() << "";
    qDebug() << "  Sync:";
    qDebug() << "    remote.server : " << sync.remote.server;
    qDebug() << "    remote.protocol : " << sync.remote.protocol;
    qDebug() << "    remote.port : " << sync.remote.port;
    qDebug() << "    remote.db : " << sync.remote.db;
    qDebug() << "    remote.userName : " << sync.remote.userName;
    qDebug() << "    remote.password : " << sync.remote.password;
    qDebug() << "";
    qDebug() << "";
}
