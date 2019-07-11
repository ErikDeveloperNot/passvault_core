#ifndef STATUSDEFS_H
#define STATUSDEFS_H

#include <string>


class StatusDefs
{
public:
    StatusDefs() = delete;

    enum Account_Status {success,
                         fail,
                         already_exists,
                         not_found,
                         encryption_error};

    static std::string get_Account_Status(int s) {
        switch (s) {
        case Account_Status::success:
            return "Success";
        case Account_Status::fail:
            return "Failed to add";
        case Account_Status::already_exists:
            return "Account already exists";
        case Account_Status::not_found:
            return "Account not found";
        case Account_Status::encryption_error:
            return "There was an error encrypting or decrypting the password";
        default:
            return "Unknown Error";
        }
    }


};

#endif // STATUSDEFS_H
