#ifndef UTILS_H
#define UTILS_H

#include "account.h"

#include <string>
#include <vector>

#include <QVector>

class Utils
{
public:
//    Utils() = default;

    static long currentTimeMilli();
    static long currentMRATime();
    static int getSelectionAsInt(int min, int max);
    static int getSelectionAsInt(int min, int max, std::string message);
    static int launchBrowserForAccount(Account *account);
};


#endif // UTILS_H
