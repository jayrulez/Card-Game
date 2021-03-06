#include "Game.h"
#include "Player.h"
#include "ServerNetwork.h"
#include "StaticHelper.h"
#include "../Shared/SharedDefines.h"

// Creates network
ServerNetwork::ServerNetwork() : m_lastPlayer(nullptr), m_listenSocket(INVALID_SOCKET), m_shuttingDown(false)
{
    // create WSADATA object
    WSADATA wsaData;

    // address info for the server to listen to
    struct addrinfo *result = nullptr;
    struct addrinfo hints;

    // Initialize Winsock
    int iResult = InitWinsock(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0)
    {
        printf("WSAStartup failed with error: %d\r\n", iResult);
        exit(1);
    }

    // set address information
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;    // TCP connection!!!
    hints.ai_flags = AI_PASSIVE;
    // Resolve the server address and port
    iResult = getaddrinfo(nullptr, DEFAULT_PORT, &hints, &result);

    if (iResult != 0)
    {
        printf("Getaddrinfo failed with error: %d\r\n", iResult);
        CloseWinsock();
        exit(1);
    }

    // Create a SOCKET for connecting to server
    m_listenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

    if (m_listenSocket == INVALID_SOCKET)
    {
        printf("Socket failed with error: %d\r\n", GetSockError());
        freeaddrinfo(result);
        CloseWinsock();
        exit(1);
    }

    // Set the mode of the socket to be blocking
    u_long iMode = 0;
    iResult = IoctlSocket(m_listenSocket, FIONBIO, &iMode);

    if (iResult == SOCKET_ERROR)
    {
        printf("IoctlSocket failed with error: %d\r\n", GetSockError());
        shutdown(m_listenSocket, SD_BOTH);
        CloseWinsock();
        exit(1);
    }

    // Setup the TCP listening socket
    iResult = bind(m_listenSocket, result->ai_addr, (int)result->ai_addrlen);

    if (iResult == SOCKET_ERROR)
    {
        printf("Bind failed with error: %d\r\n", GetSockError());
        freeaddrinfo(result);
        shutdown(m_listenSocket, SD_BOTH);
        CloseWinsock();
        exit(1);
    }

    // no longer need address information
    freeaddrinfo(result);

    // start listening for new clients attempting to connect
    iResult = listen(m_listenSocket, SOMAXCONN);

    if (iResult == SOCKET_ERROR)
    {
        printf("Listen failed with error: %d\r\n", GetSockError());
        shutdown(m_listenSocket, SD_BOTH);
        CloseWinsock();
        exit(1);
    }
}

// Removes all resources
ServerNetwork::~ServerNetwork()
{
    DEBUG_LOG("Destroying server network\r\n");

    GetLocker().lock();
    m_shuttingDown = true;
    GetLocker().unlock();

    for (PlayerMap::iterator iter = m_players.begin(); iter != m_players.end(); iter++)
    {
        iter->second.first->Disconnect();
        iter->second.second->join();
        delete iter->second.second;
        iter->second.first->GetGame()->RemovePlayer(iter->second.first->GetId());
        delete iter->second.first;
    }

    DEBUG_LOG("Server network cleared\r\n");
}

// Accepts new connections
bool ServerNetwork::AcceptNewClient(unsigned int& id)
{
    // if client is waiting, accept the connection and save the socket
    SOCKET ClientSocket = accept(m_listenSocket, nullptr, nullptr);
    if (ClientSocket != INVALID_SOCKET)
    {
        Game* game = nullptr;
        if (m_lastPlayer)
        {
            game = m_lastPlayer->GetGame();
            if (game->IsFull())
                game = new Game();
        }
        else
            game = new Game();

        Player* player = new Player(id, ClientSocket, game, this);
        game->AddPlayer(player);
        m_lastPlayer = player;

        std::thread* t = new std::thread(&ServerNetwork::handlePlayerNetwork, player);

        // insert new client into session id table
        m_players.insert(std::make_pair(id, std::make_pair(player, t)));

        return true;
    }

    return false;
}

void ServerNetwork::handlePlayerNetwork(Player* player)
{
    ServerNetwork* network = player->GetNetwork();
    int playerId = player->GetId();

    while (!network->IsShuttingDown())
    {
        char networkData[MAX_PACKET_SIZE];

        // get data for that client
        int dataLength = network->ReceiveData(player, networkData);
        if (!dataLength)
        {
            
            if (!network->IsShuttingDown())
            {
                std::mutex& locker = network->GetLocker();
                locker.lock();

                try
                {
                    if (!network->IsShuttingDown())
                    {
                        player->Disconnect();
                        PlayerMap::iterator iter = network->GetPlayers().find(playerId);
                        if (iter != network->GetPlayers().end())
                        {
                        iter->second.second->detach();
                        delete iter->second.second;

                        network->GetPlayers().erase(iter);
                        }

                        if (player->GetGame()->IsEmpty())
                            delete player->GetGame();
                    }
                }
                catch(...)
                {
                    // Prevent deadlock
                }

                locker.unlock();
            }
            break;
        }

        // invalid packet sended
        if (dataLength < 2)
            continue;

        player->ReceivePacket(dataLength, networkData);
    }

    DEBUG_LOG("Thread of player %d has ended.\r\n", playerId);
}

// receive incoming data
int ServerNetwork::ReceiveData(Player const* player, char* recvbuf) const
{
    return NetworkServices::ReceiveMessage(player->GetSocket(), recvbuf, MAX_PACKET_SIZE);
}

// Broadcasts packet to all clients
void ServerNetwork::BroadcastPacket(Packet const* packet) const
{
    for (PlayerMap::const_iterator iter = m_players.begin(); iter != m_players.end(); iter++)
        iter->second.first->SendPacket(packet);
}

// Sends packet to player searched by name
bool ServerNetwork::SendPacketToPlayer(std::string const& playerName, Packet const* packet) const
{
    for (PlayerMap::const_iterator iter = m_players.begin(); iter != m_players.end(); iter++)
    {
        if (StaticHelper::CompareStringCaseInsensitive(iter->second.first->GetName(), playerName))
        {
            iter->second.first->SendPacket(packet);
            return true;
        }
    }

    return false;
}

// Set last player to null if this player was the one last connected
void ServerNetwork::OnPlayerDisconnected(Player const* player)
{
    if (m_lastPlayer && (player->GetId() == m_lastPlayer->GetId()))
        m_lastPlayer = nullptr;
}
