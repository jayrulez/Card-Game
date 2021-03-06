#pragma once
#include <cstdint>
#include <list>
#define MAX_CARDS_COUNT 5
#define MAX_CARDS_ON_DECK 4

enum CardType
{
    CARD_TYPE_MELEE = 1,
    CARD_TYPE_RANGED,
    CARD_TYPE_DEFENSE
};

enum CardActions
{
    CARD_ACTION_NONE = 0,
    CARD_ACTION_DEFEND,
    CARD_ACTION_BASIC_ATTACK,
    CARD_ACTION_SPELL_USE
};

enum FailReson
{
    INVALID_CARD_COUNT = 1,
    INVALID_CARD_ID
};

enum CardStats
{
    CARD_STAT_DEFENSE = 1,
    CARD_STAT_DAMAGE
};

class Spell;
class Card
{
    private:
        uint32_t m_id;
        uint8_t m_type;
        uint8_t m_price;
        Spell const* m_spell;

    protected:
        uint8_t m_hp;
        uint8_t m_damage;
        uint8_t m_mana;
        uint8_t m_defense;

    public:
        Card(uint32_t id, uint8_t type, uint8_t hp, uint8_t damage, uint8_t mana, uint8_t defense, uint8_t price, Spell const* spell);
        virtual ~Card() { }

        uint32_t const& GetId() const { return m_id; }
        uint8_t const& GetType() const { return m_type; }
        uint8_t const& GetDamage() const { return m_damage; }
        uint8_t const& GetDefense() const { return m_defense; }
        uint8_t const& GetMana() const { return m_mana; }
        uint8_t const& GetHealth() const { return m_hp; }
        uint8_t const& GetPrice() const { return m_price; }
        Spell const* GetSpell() const { return m_spell; }
};

