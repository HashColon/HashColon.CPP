#include "redis-manager.hpp"
#include <algorithm>
#include <mutex>
#include <regex>
#include <string>
#include <hiredis/async.h>
#include <hiredis/hiredis.h>
#include <HashColon/CLI11.hpp>
#include <HashColon/Helper.hpp>
#include <HashColon/Log.hpp>
#include <HashColon/SingletonCLI.hpp>

using namespace std;
using namespace HashColon;

namespace Avikus
{
    /* namespace redis */
    void Redis::Initialize(const string configFilePath, const string configNamespace)
    {
        using namespace HashColon;
        CLI::App *cli = SingletonCLI::GetInstance().GetCLI(configNamespace);

        if (!configFilePath.empty())
            SingletonCLI::GetInstance().AddConfigFile(configFilePath);

        cli->add_option("--host", _cDefault.host, "REDIS connection host")
            ->envname(GetEnvName(configNamespace, "host"));
        cli->add_option("--port", _cDefault.port, "REDIS connection port number")
            ->envname(GetEnvName(configNamespace, "port"));
    }
}

/* RedisManager_Sync */
namespace Avikus
{
    RedisManager_Sync::~RedisManager_Sync()
    {
        // free & disconnect redis before destruction
        Disconnect();
    }

    Redis::_Params RedisManager_Sync::GetParams()
    {
        // return connection information from the context
        if (_context != nullptr)
            return {_context->tcp.host, _context->tcp.port};
        else
            return {"", 0};
    };

    void RedisManager_Sync::Connect(const string host, int port)
    {
        // check if _context is not empty, Disconnect
        if (_context != nullptr)
            Disconnect();
        // try connection
        _context = redisConnect(host.c_str(), port);

        // if connection is still null
        if (_context == NULL || _context == nullptr)
            throw Exception("Failed to allocate REDIS context");
        // connection failed with error msg
        else if (_context->err)
            throw Exception("Failed to connect REDIS server: " + host + ":" + to_string(port) + " : " + string(_context->errstr));
        // connection succeed
        else
        {
            GlobalLogger.Log() << "REDIS connected: " << host << ":" << port << endl;
        }
    }

    void RedisManager_Sync::Disconnect()
    {
        // Free & disconnect redis
        redisFree(_context);
        GlobalLogger.Log() << "REDIS disconnected." << endl;
    }

    const string RedisManager_Sync::GetMessage_String(const string key)
    {
        // return value
        string res = "";
        // connection msg: ex) GET nmea:own
        const string getmsg = "GET " + key;

        // connect redis and get reply
        redisReply *reply = nullptr;
        reply = (redisReply *)redisCommand(_context, getmsg.c_str());

        if (reply == NULL)
        {
            GlobalLogger.Log() << "REDIS command failed: " << getmsg << endl;
        }
        else if (reply->type == REDIS_REPLY_ERROR)
        {
            GlobalLogger.Log() << "REDIS command returned error: " << getmsg << " : " << reply->str << endl;
        }
        else if (reply->type == REDIS_REPLY_NIL)
        {
            GlobalLogger.Log() << "REDIS command returned nil: " << getmsg << endl;
        }
        else if (reply->type == REDIS_REPLY_STRING)
        {
// if debug, log msg
#ifdef DEBUG
            GlobalLogger.Log() << "REDIS command replied: " << getmsg << " : " << reply->str << endl;
#endif
            res = reply->str;
        }
        else
        {
        }

        // free reply
        freeReplyObject(reply);
        return res;
    }

    void RedisManager_Sync::SetMessage_String(const string key,
                                              const string msg)
    {
        // redis command
        string setmsg = "SET " + key + " \"" + regex_replace(msg, regex("\""), "\\\"") + "\"";
        // send command with binary-safe string (%b): converts " => \"
        redisReply *reply = (redisReply *)redisCommand(_context, "SET %b %b",
                                                       key.c_str(), key.length(),
                                                       msg.c_str(), msg.length());

        GlobalLogger.Debug() << setmsg << endl;

        // check err
        if (reply == NULL)
        {
            GlobalLogger.Log() << "REDIS command failed: " << setmsg << endl;
        }
        else if (reply->type == REDIS_REPLY_ERROR)
        {
            GlobalLogger.Log() << "REDIS command returned error: " << setmsg << " : " << reply->str << endl;
        }
        else
        {
        }

        freeReplyObject(reply);
        return;
    }

    void RedisManager_Sync::DelKeys_withPattern(const string pattern)
    {
        string delmsg = "DEL (ls " + pattern + ")";
        redisReply *reply = (redisReply *)redisCommand(_context, "DEL (ls %b)",
                                                       pattern.c_str(), pattern.length());
        GlobalLogger.Debug() << delmsg << endl;
        return;
    }
}

/* RedisManager_Async */

namespace Avikus
{
    mutex redisMutex;

    Redis::_Params &GetDefaultParams() { return Redis::_cDefault; };

    RedisManager_Async::~RedisManager_Async()
    {
        // free & disconnect redis before destruction
        Disconnect();
    }

    Redis::_Params RedisManager_Async::GetParams()
    {
        // return connection information from the context
        if (_context != nullptr)
            return {_context->c.tcp.host, _context->c.tcp.port};
        else
            return {"", 0};
    };

    void RedisManager_Async::OnConnect(const redisAsyncContext *ac, int status)
    {

        // i don't know why this statement should be done, but everybody does it.
        ((void)ac);
        if (ac == nullptr)
            throw Exception("Failed to allocate REDIS(async) context");
        else if (status != REDIS_OK)
            throw Exception("Failed to connect REDIS(async) server: " + string(ac->c.tcp.host) + ":" + to_string(ac->c.tcp.port) + " : " + string(ac->errstr));
        else
            GlobalLogger.Log() << "REDIS(async) connected: " << ac->c.tcp.host << ":" << ac->c.tcp.port << endl;
    }

    void RedisManager_Async::OnDisconnect(const redisAsyncContext *ac, int status)
    {
        if (status != REDIS_OK)
            throw Exception("REDIS(async) disconnected: " + string(ac->errstr));
        else
            GlobalLogger.Log() << "REDIS(async) disconnected" << endl;
    }

    void RedisManager_Async::Connect()
    {
        Connect(Redis::_cDefault.host, Redis::_cDefault.port);
    }

    void RedisManager_Async::Connect(const string host, const int port)
    {
        // check if _context is not empty, Disconnect
        if (_context != nullptr)
            Disconnect();
        // try connection, async lib is not thread safe
        {
            lock_guard<mutex> lg(redisMutex);
            _context = redisAsyncConnect(host.c_str(), port);
        }

        // set callbacks for connection, disconnection
        int check;

        check = redisAsyncSetConnectCallback(_context, OnConnect);
        if (check != REDIS_OK)
            throw Exception("REDIS(async): Failed to set OnConnect callback");

        check = redisAsyncSetDisconnectCallback(_context, OnDisconnect);
        if (check != REDIS_OK)
            throw Exception("REDIS(async): Failed to set OnDisconnect callback");
    }

    void RedisManager_Async::Disconnect()
    {
        // Free & disconnect redis
        redisAsyncDisconnect(_context);
        GlobalLogger.Log() << "REDIS(async) disconnected." << endl;
    }
}