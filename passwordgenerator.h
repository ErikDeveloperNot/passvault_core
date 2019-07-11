#ifndef PASSWORDGENERATOR_H
#define PASSWORDGENERATOR_H

#include "settings.h"

#include <vector>
#include <set>
#include <exception>

class PasswordGenerator
{
public:
    PasswordGenerator(Generator &);

    void enableLower(bool);
    void enableUpper(bool);
    void enableDigits(bool);
    void enableSpecial(bool);

    void addChar(char);
    void removeChar(char);

    int getPasswordLength();
    void setPasswordLength(int);
    std::set<char> getAllowedCharacters();

    Generator getGeneratorOptions() { return generator_struct; }

    QString generate();

    bool lowerEnabled() { return lower; }
    bool upperEnabled() { return upper; }
    bool digitsEnabled() { return digits; }
    bool specialsEnabled() { return special; }

private:
    Generator generator_struct;
    bool lower{false};
    bool upper{false};
    bool digits{false};
    bool special{false};

    int aLower = static_cast<int>(char{'a'});
    int zLower = static_cast<int>(char{'z'});
    int aUpper = static_cast<int>(char{'A'});
    int zUpper = static_cast<int>(char{'Z'});
    int zero = static_cast<int>(char{'0'});
    int nine = static_cast<int>(char{'9'});

    std::vector<char> specials = {'@','_','$','&','!','?','*','-'};

    void modifyCharacters(bool, int, int);
};


class GeneratorException : public std::exception
{
public:
    GeneratorException(const char* _ex) : ex{_ex} {}
    ~GeneratorException() = default;

    virtual const char* what() const noexcept
    {
        return ex;
    }

private:
    const char* ex;
};


#endif // PASSWORDGENERATOR_H
