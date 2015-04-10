#include "Game.h"
#include "Player.h"
#include "serverNetwork.h"
#include "StaticHelper.h"

// Creates network
ServerNetwork::ServerNetwork() : m_lastPlayer(nullptr), ListenSocket(INVALID_SOCKET), ClientSocket(INVALID_SOCKET)
{
    // create WSADATA object
    WSADATA wsaData;

    // address info for the server to listen to
    struct addrinfo *result = nullptr;
    struct addrinfo hints;

    // Initialize Winsock
    iResult = InitWinsock(MAKEWORD(2, 2), &wsaData);
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
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

    if (ListenSocket == INVALID_SOCKET)
    {
        printf("Socket failed with error: %d\r\n", GetSockError());
        freeaddrinfo(result);
        CloseWinsock();
        exit(1);
    }

    // Set the mode of the socket to be nonblocking
    u_long iMode = 1;
    iResult = IoctlSocket(ListenSocket, FIONBIO, &iMode);

    if (iResult == SOCKET_ERROR)
    {
        printf("IoctlSocket failed with error: %d\r\n", GetSockError());
        shutdown(ListenSocket, SD_BOTH);
        CloseWinsock();
        exit(1);
    }

    // Setup the TCP listening socket
    iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);

    if (iResult == SOCKET_ERROR)
    {
        printf("Bind failed with error: %d\r\n", GetSockError());
        freeaddrinfo(result);
        shutdown(ListenSocket, SD_BOTH);
        CloseWinsock();
        exit(1);
    }

    // no longer need address information
    freeaddrinfo(result);

    // start listening for new clients attempting to connect
    iResult = listen(ListenSocket, SOMAXCONN);

    if (iResult == SOCKET_ERROR)
    {
        printf("Listen failed with error: %d\r\n", GetSockError());
        shutdown(ListenSocket, SD_BOTH);
        CloseWinsock();
        exit(1);
    }
}

// Removes all resources
ServerNetwork::~ServerNetwork()
{
    for (PlayerMap::const_iterator iter = m_players.begin(); iter != m_players.end(); iter++)
    {
        iter->second->GetGame()->RemovePlayer(iter->second->GetId());
        delete iter->second;
    }
}

// Accepts new connections
bool ServerNetwork::AcceptNewClient(unsigned int& id)
{
    // if client is waiting, accept the connection and save the socket
    ClientSocket = accept(ListenSocket, nullptr, nullptr);
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
        
        // insert new client into session id table
        m_players.insert(std::pair<unsigned int, Player*>(id, player));

        return true;
    }

    return false;
}

// receive incoming data
int ServerNetwork::ReceiveData(Player* player, char* recvbuf)
{
    iResult = NetworkServices::receiveMessage(player->GetSocket(), recvbuf, MAX_PACKET_SIZE);
    return iResult;
}

// Broadcasts packet to all clients
void ServerNetwork::BroadcastPacket(Packet const* packet) const
{
    for (PlayerMap::const_iterator iter = m_players.begin(); iter != m_players.end(); iter++)
        iter->second->SendPacket(packet);
}

// Sends packet to player searched by name
bool ServerNetwork::SendPacketToPlayer(std::string const& playerName, Packet const* packet) const
{
    for (PlayerMap::const_iterator iter = m_players.begin(); iter != m_players.end(); iter++)
    {
        if (StaticHelper::CompareStringCaseInsensitive(iter->second->GetName(), playerName))
        {
            iter->second->SendPacket(packet);
            return true;
        }
    }

    return false;
}

// Set last player to null if this player was the one last connected
void ServerNetwork::OnPlayerDisconnected(Player* player)
{
    if (m_lastPlayer && (player->GetId() == m_lastPlayer->GetId()))
        m_lastPlayer = nullptr;
}
