#pragma once
#include <map>
#include <string>
#include <vector>
#include "Game.h"
#include "../Multiplatform/NetworkCommunication.h"

class Card;
class Packet;
class ServerNetwork;
class PlayableCard;

typedef std::map<uint64_t, PlayableCard*> CardsMap;

class Player
{
    private:
        std::vector<PlayableCard*> m_cardOrder;
        std::vector<PlayableCard*> m_currentCards;
        bool m_isPrepared;
        bool m_isDisconnected;
        uint32_t m_id;
        CardsMap m_cards;
        uint8_t m_currentCardIndex;
        Game* m_game;
        ServerNetwork* m_network;
        SOCKET m_socket;
        std::string m_name;
        std::string m_AesKey;

        void destroyCard(uint64_t const& cardGuid);
        void endTurn();

    public:
        Player(uint32_t id, SOCKET socket, Game* game, ServerNetwork* network);
        ~Player();

        void SetEncryptionKey(std::string const& key) { m_AesKey = key; }
        void SetName(std::string const& name) { m_name = name; }
        void SendInitResponse() const;
        void SendAvailableCards() const;
        void SendChatWhisperResponse(std::string const& message, std::string const& receiver, bool success) const;
        void SendSelectCardsFailed(uint8_t const& failReason) const;
        void SendPlayerDisconnected() const;
        void SendAttackResult(uint8_t const& result, uint64_t const& cardGuid, uint8_t const& damage) const;
        void SendEndGame() const;
        void SendCardStatChanged(uint64_t const& cardGuid, uint8_t const& cardStat, int8_t const& value) const;
        void Attack(uint64_t const& victimCardGuid, uint8_t const& attackType);
        void DefendSelf();
        void Prepare();
        
        void ClearCards() { m_cards.clear(); }
        void CreateCard(Card const* cardTemplate);
        void ReceivePacket(uint32_t const& length, char const* packetData);
        void SendPacket(Packet const* packet) const;
        void Disconnect();
        void HandleDeckCards(bool addCard);
        
        Player* GetOpponent() const { return m_game->GetOpponent(this); }
        PlayableCard* GetCurrentCard();
        CardsMap const& GetCards() { return m_cards; }
        std::vector<PlayableCard*> const& GetCurrentCards() { return m_currentCards; }
        PlayableCard* GetCard(uint64_t cardGuid);
        SOCKET GetSocket() const { return m_socket; }
        Game* GetGame() const { return m_game; }
        uint32_t GetId() const { return m_id; }
        std::string const& GetName() const { return m_name; }
        std::string const& GetEncryptionKey() const { return m_AesKey; }
        ServerNetwork const* GetNetwork() const { return m_network; }
        bool IsPrepared() const { return m_isPrepared; }
        bool IsDisconnected() const { return m_isDisconnected; }
        bool IsActive() const { return m_game->GetActivePlayerId() == m_id; }
};
