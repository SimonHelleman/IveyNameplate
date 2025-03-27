#pragma once
#include <stdint.h>
#include <memory>
#include <vector>
#include <deque>
#include <unordered_map>
#include <thread>

#define ASIO_STANDALONE
#include <asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>

// ASIO includes windows API which is annoying
#ifdef SendMessage
#undef SendMessage
#endif

#include "Reaction.h"
#include "QuietMode.h"
#include "DatabaseConnection.h"
#include "ClientConnection.h"

namespace nameplate
{
class Server
{
public:
    struct PollResponse
    {
        PollResponse() = default;
        PollResponse(uint32_t clientOwner, int response)
            : clientOwner(clientOwner), response(response)
        {
        }

        uint32_t clientOwner;
        int response;
    };

public:
    Server(uint16_t port, DatabaseConnection& database);

    Server(const Server&) = delete;

    Server& operator=(const Server&) = delete;
public:

    void Start();
    void Stop();

    void AsyncWaitForConnection();

    void SendMessage(ClientConnection& client, const Message& message);
    void SendMessage(const uint32_t clientId, const Message& message);
    void BroadcastMessage(const Message& message);

    void OnConnect(ClientConnection& client);
    void OnDisconnect(const ClientConnection& client);

    void HandleMessages();

    const std::vector<PollResponse>& PollResponseData() const
    {
        return m_pollResponses;
    }

    const std::vector<Student>& StundentsInClass()
    {
        return m_studentsInClass;
    }

    const int ReactionCount(const Reaction r)
    {
        return m_reactions[r];
    }

private:
    void SendStudentInfo(const uint32_t clientId, const uint32_t studentId);

    void HandleStudentId(Message& msg);
    void HandleStudentInfo(Message& msg);
    void HandlePollResponse(Message& msg);
    void HandleLeaveClass(Message& msg);
    void HandleSetReaction(Message& msg);
    void HandleClearReaction(Message& msg);

private:
    uint16_t m_port;
    unsigned int m_numConnections = 0;

    std::vector<std::shared_ptr<ClientConnection>> m_connections;
    
    std::deque<Message> m_incomingMessageQueue;

    asio::io_context m_context;
    asio::ip::tcp::acceptor m_connectionAcceptor;
    std::thread m_serverThread;

    DatabaseConnection& m_database;

    std::vector<PollResponse> m_pollResponses;
    std::vector<Student> m_studentsInClass;

    std::unordered_map<Reaction, int> m_reactions;
};

}