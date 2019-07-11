#include "utils.h"
#include <chrono>
#include <iostream>

#include <QDesktopServices>
#include <QUrl>

long Utils::currentTimeMilli()
{
//     std::chrono::duration_cast< std::chrono::milliseconds >(
//                std::chrono::system_clock::now().time_since_epoch().count());
    return std::chrono::system_clock::now().time_since_epoch().count()/1000;
}


long Utils::currentMRATime()
{
    //                         milli   sec    min  hr   day
    return Utils::currentTimeMilli() / 1000 / 60 / 60 / 24;
}


int Utils::getSelectionAsInt(int min, int max)
{
    std::string selection;
    std::getline(std::cin, selection);
    int val;

    try {
        val = std::stoi(selection);
    } catch (...) {
        std::cout << "Inavlid Selection" << std::endl;
        return -1;
    }

    if (val >= min && val <= max)
        return val;

    std::cout << "Invalid Value" << std::endl;
    return -1;
}


int Utils::getSelectionAsInt(int min, int max, std::string message)
{
    int selection = -1;
    do {
        std::cout << message;
        selection = getSelectionAsInt(min, max);
    } while (selection == -1);

    return selection;
}

int Utils::launchBrowserForAccount(Account *account)
{
    // will need to modify to verify url is http/https/or www
    QString url{account->getUrl()};

    if (!url.contains("http://", Qt::CaseSensitivity::CaseInsensitive) &&
            !url.contains("https://", Qt::CaseSensitivity::CaseInsensitive)) {

        url.prepend("http://");
    }

    return QDesktopServices::openUrl(QUrl{url});
}


