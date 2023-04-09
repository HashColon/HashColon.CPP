#ifndef _HG_HASHCOLON_WEBSOCKETSTREAM
#define _HG_HASHCOLON_WEBSOCKETSTREAM

#include <HashColon/header>

#include <set>
#include <memory>

#include <websocketpp/config/asio.hpp>
#include <websocketpp/server.hpp>

#include <HashColon/exception.hpp>

namespace HashColon
{
    using WebsocketConnectionSet =
        std::set<websocketpp::connection_hdl, std::owner_less<websocketpp::connection_hdl> >;
    using WebsocketServer = websocketpp::server<websocketpp::config::asio_tls>;

    class StreamServer
    {
        HASHCOLON_CLASS_EXCEPTION_DEFINITION(StreamServer);

        enum class _EventType
        {
            ConnectionOpen,
            ConnectionClose,
            ReceivedMessage,
            SendMessage
        };

        struct _Event
        {
            _Event(_EventType t, websocketpp::connection_hdl hdl)
                : type(t), connectionHandle(hdl)
            {
                if (t != _EventType::ConnectionOpen && t != _EventType::ConnectionClose)
                {
                    throw StreamServerException("StreamServer event type do not match with the constructor.");
                }
            };
            _Event(_EventType t, websocketpp::connection_hdl hdl, std::string msg)
                : type(t), connectionHandle(hdl){};
            _Event(_EventType t, std::string msg)
                : type(t), message(msg){};

            _EventType type;
            websocketpp::connection_hdl connectionHandle;
            std::string message;
        };

        WebsocketServer _server;
        WebsocketConnectionSet _connections;
        std::queue <

    }

    class WebsocketStreamBuf : public std::streambuf
    {
        /**
         * @brief Override function for std::streambuf::xsputn
         * @details Writes characters from the array pointed to by s into the controlled output sequence,
         *          until either n characters have been written or the end of the output sequence is reached.
         *
         * @param s const char string
         * @param n length of the string
         * @return std::streamsize return the size of the printed string
         */
        virtual std::streamsize xsputn(const char *s, std::streamsize n) final override;

        /**
         * @brief
         * @details
         * @return int_type
         */
        virtual int_type underflow() final override;

        /**
         * @brief Override function for std::streambuf::overflow
         * @details Virtual function called by other member functions to put a character into the controlled output sequence without changing the current position.
         *
         * @param c Character to be put
         * @return int_type The character put
         */
        virtual int_type overflow(int_type c = traits_type::eof()) final override;

        /**
         * @brief Override function for std::streambuf::sync
         *
         * @return int 0 if success, -1 if failed.
         */
        virtual int sync() final override;
    }

    class WebsocketOstream : public std::ostream
    {

        websocketpp::server<websocketpp::config::asio_tls> _server;
    }
}

#endif