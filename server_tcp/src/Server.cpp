#include <utility>
#include <functional>
#include <algorithm>
#include "Server.h"

#define SCROLLS_USE_LOGGER_MACROS
#include <Scrolls.h>

namespace nameplate
{

Server::Server(uint16_t port, DatabaseConnection& database)
    : m_port(port), m_connectionAcceptor(m_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)),
    m_database(database), m_pollResponses()
{
}

void Server::Start()
{
    AsyncWaitForConnection();

    m_serverThread = std::thread([this]() {
        m_context.run();
    });

    INFO("[Server] server started on port " + std::to_string(m_port));
}

void Server::Stop()
{
    m_context.stop();

    if (m_serverThread.joinable())
    {
        m_serverThread.join();
    }

    INFO("[Server] server stopped");
}

void Server::AsyncWaitForConnection()
{
    m_connectionAcceptor.async_accept([this](asio::error_code error, asio::ip::tcp::socket socket) {
        if (error)
        {
            ERROR_FL("[Server] async_accept failed with erorr: " + error.message());

            AsyncWaitForConnection();
            return;
        }

        LOG_DEBUG("[Server] New connection from " + socket.remote_endpoint().address().to_string());
        
        const std::function<void(const ClientConnection&)> disconHandle = std::bind(&Server::OnDisconnect, this, std::placeholders::_1);
        
        const auto connection = std::make_shared<ClientConnection>(
            m_context, std::move(socket), ++m_numConnections, m_incomingMessageQueue, disconHandle
        );

        m_connections.push_back(connection);
        connection->Start();

        OnConnect(*connection);
        
        AsyncWaitForConnection();
    });
}

void Server::SendMessage(ClientConnection& client, const Message& message)
{
    if (!client.IsConnected())
    {
        OnDisconnect(client);
        return;
    }

    client.SendMessage(message);
}

void Server::SendMessage(const uint32_t clientId, const Message& message)
{
    const auto it = std::find_if(m_connections.begin(), m_connections.end(), [clientId](const std::shared_ptr<ClientConnection>& c) {
        return c->Id() == clientId;
    });

    if (it != m_connections.end())
    {
        SendMessage(**it, message);
    }
}

void Server::BroadcastMessage(const Message& message)
{
    for (const auto& c : m_connections)
    {
        c->SendMessage(message);
    }
}

void Server::OnConnect(ClientConnection& client)
{
    INFO("[Server] " + client.IpAddress() + " connected to server with id " + std::to_string(client.Id()));
    // Send their Id
    Message message(PacketType::SetClientId, client.Id());
    SendMessage(client, message);

    // Send default quiet mode value
    Message quietMsg(PacketType::QuiteMode, client.Id());
    QuietMode mode = QuietMode::Off;
    quietMsg.Push(&mode, sizeof(mode));
    SendMessage(client, quietMsg);
}

// TODO still an edge case where if a client disconnects before clearing their reactions
void Server::OnDisconnect(const ClientConnection& client)
{
    INFO("[Server] " + client.IpAddress() + " (connection_" + std::to_string(client.Id()) + ") disconnected from server");

    if (client.hasStudentSession)
    {
        const auto it = std::find_if(m_studentsInClass.begin(), m_studentsInClass.end(), [&](const Student& student) {
            return client.currentStudentId == student.id;
        });

        if (it != m_studentsInClass.end()) {
            m_studentsInClass.erase(it);
            LOG_DEBUG("[Server] student " + std::to_string(client.currentStudentId) + " removed because of disconnection");
        }
    }

    const auto it = std::find_if(m_connections.begin(), m_connections.end(), [&client](const std::shared_ptr<ClientConnection>& ptr) {
        return ptr.get() == &client;
    });

    if (it != m_connections.end())
    {
        m_connections.erase(it);
        return;
    }

    ERROR("[Server] Could not remove connection_" + std::to_string(client.Id()) + " (" + client.IpAddress() + ')');
}

void Server::HandleMessages()
{
    while (!m_incomingMessageQueue.empty())
    {
        Message& msg = m_incomingMessageQueue.front();
        LOG_DEBUG("[Server] message:\n" + msg.AsString());

        switch (msg.MessageType())
        {
        case PacketType::StudentId:
            HandleStudentId(msg);
            break;
        case PacketType::StudentInfo:
            HandleStudentInfo(msg);
            break;
        case PacketType::PollResponse:
            HandlePollResponse(msg);
            break;
        case PacketType::LeaveClass:
            HandleLeaveClass(msg);
            break;
        case PacketType::SetReaction:
            HandleSetReaction(msg);
            break;
        case PacketType::ClearReaction:
            HandleClearReaction(msg);
            break;
        default:
            ERROR_FL("[Server] message is an unknown type and can not be handled");
        }

        m_incomingMessageQueue.pop_front();
    }
}

void Server::SendStudentInfo(const uint32_t clientId, const uint32_t studentId)
{
    Student student = m_database.FetchStudent(studentId);
    Message resp(PacketType::StudentInfo, clientId);

    // 4 bytes of length info for the packet (2 per string)
    const uint16_t lastNameBufSz = static_cast<uint16_t>(student.lastName.length() + 1);
    const uint16_t firstNameBufSz = static_cast<uint16_t>(student.firstName.length() + 1);

    resp.Push(&student.id, sizeof(student.id));
    resp.Push(student.lastName.c_str(), lastNameBufSz);
    resp.Push(student.firstName.c_str(), firstNameBufSz);
    resp.Push(&lastNameBufSz, sizeof(lastNameBufSz));
    resp.Push(&firstNameBufSz, sizeof(firstNameBufSz));

    m_studentsInClass.push_back(student);

    for (const auto& c : m_connections)
    {
        if (c->Id() == clientId)
        {
            c->currentStudentId = studentId;
            c->hasStudentSession = true;
        }
    }

    SendMessage(clientId, resp);
}

void Server::HandleStudentId(Message& msg)
{
    // Query database
    // if user exists, send data, else, send something else (TBD)

    uint32_t studentId;
    msg.Pop(&studentId, sizeof(uint32_t), sizeof(uint32_t));
    const bool exists = m_database.DoesStudentExist(studentId);

    if (exists)
    {
        SendStudentInfo(msg.ClientId(), studentId);
        return;
    }

    // Send not found message
    const Message resp(PacketType::StudentNotFound, msg.ClientId());
    SendMessage(msg.ClientId(), resp);
}

void Server::HandleStudentInfo(Message& msg)
{
    uint16_t firstNameSize;
    msg.Pop(&firstNameSize, sizeof(firstNameSize), sizeof(firstNameSize));

    uint16_t lastNameSize;
    msg.Pop(&lastNameSize, sizeof(lastNameSize), sizeof(lastNameSize));

    // Pop strings into a vector
    std::vector<uint8_t> buf(firstNameSize + lastNameSize);
    msg.Pop(buf.data(), buf.size(), firstNameSize + lastNameSize);

    const char* lastName = reinterpret_cast<const char*>(&buf[0]);
    const char* firstName = reinterpret_cast<const char*>(&buf[lastNameSize]);

    uint32_t studentId;
    msg.Pop(&studentId, sizeof(uint32_t), sizeof(studentId));

    m_database.CreateStudent(studentId, lastName, firstName);

    SendStudentInfo(msg.ClientId(), studentId);
}

void Server::HandlePollResponse(Message& msg)
{
    int pollResponse;
    msg.Pop(&pollResponse, sizeof(pollResponse), sizeof(pollResponse));

    // Check for existing response from a particular nameplate
    const auto it = std::find_if(m_pollResponses.begin(), m_pollResponses.end(), [msg](const PollResponse& resp) {
        return msg.ClientId() == resp.clientOwner;
    });

    if (it != m_pollResponses.end())
    {
        (*it).response = pollResponse;
        LOG_DEBUG("[Server] poll response updated to " + std::to_string(pollResponse));
        return;
    }

    m_pollResponses.emplace_back(msg.ClientId(), pollResponse);
    LOG_DEBUG("[Server] poll response " + std::to_string(pollResponse));
}

void Server::HandleLeaveClass(Message& msg)
{
    uint32_t studentId;
    msg.Pop(&studentId, sizeof(studentId), sizeof(studentId));

    const auto it = std::find_if(m_studentsInClass.begin(), m_studentsInClass.end(), [msg, studentId](const Student& student) {
        return studentId == student.id;
    });

    if (it != m_studentsInClass.end()) {
        m_studentsInClass.erase(it);
        LOG_DEBUG("[Server] student " + std::to_string(studentId) + " signed out");
    }
}

void Server::HandleSetReaction(Message& msg)
{
    Reaction r;
    msg.Pop(&r, sizeof(r), sizeof(r));
    ++m_reactions[r];
}

void Server::HandleClearReaction(Message& msg)
{
    Reaction r;
    msg.Pop(&r, sizeof(r), sizeof(r));
    --m_reactions[r];
}

}
