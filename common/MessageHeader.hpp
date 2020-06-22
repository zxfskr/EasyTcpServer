#ifndef __MessageHeader_hpp__
#define __MessageHeader_hpp__

enum
{
    CMD_LOGIN,
    CMD_LOGIN_RESULT,
    CMD_LOGOUT,
    CMD_LOGOUT_RESULT,
    CMD_NEW_USER_JOIN,
    CMD_ERROR
};

struct DataHeader
{
    DataHeader()
    {
        dataLength = sizeof(DataHeader);
        cmd = CMD_ERROR;
    }
    short dataLength;
    short cmd;
};

struct Error : public DataHeader
{
    Error()
    {
        dataLength = sizeof(Error);
        cmd = CMD_ERROR;
    }
};

struct Login : public DataHeader
{
    Login()
    {
        dataLength = sizeof(Login);
        cmd = CMD_LOGIN;
    };

    char userName[32];
    char password[32];

    char data[960];
};

struct LoginResult : public DataHeader
{
    int result;
    LoginResult()
    {
        dataLength = sizeof(LoginResult);
        cmd = CMD_LOGIN_RESULT;
        result = 1;
    };

    char data[1024];
};

struct NewUserJoin : public DataHeader
{
    int socket;
    NewUserJoin()
    {
        dataLength = sizeof(NewUserJoin);
        cmd = CMD_NEW_USER_JOIN;
        socket = 0;
    };
};

struct Logout : public DataHeader
{
    Logout()
    {
        dataLength = sizeof(Logout);
        cmd = CMD_LOGOUT;
    };
};

struct LogoutResult : public DataHeader
{
    int result;
    LogoutResult()
    {
        dataLength = sizeof(LogoutResult);
        cmd = CMD_LOGOUT_RESULT;
        result = 0;
    };
};
#endif // !