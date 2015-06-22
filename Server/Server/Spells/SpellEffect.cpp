#include "SpellEffect.h"
#include "../Cards/PlayableCard.h"
#include "../PacketHandlers/Packet.h"
#include "../Player.h"

SpellEffectFunc const SpellEffect::m_spellEffects[] =
{
    handleDirectDamage, // SPELL_EFFECT_DIRECT_DAMAGE
    handleApplyAura,    // SPELL_EFFECT_APPLY_AURA
    handleHeal          // SPELL_EFFECT_HEAL
};

bool SpellEffect::handleDirectDamage(Player* attacker, Player* victim, uint64_t targetGuid, SpellEffectValues const* effectValues)
{
    std::list<PlayableCard*> targets = SpellTargetSelector::GetTargets(effectValues->Target, attacker, victim, targetGuid);
    if (targets.empty())
        return false;

    attacker->SpellAttack(targets, effectValues->Value1);
    return true;
}

bool SpellEffect::handleApplyAura(Player* attacker, Player* victim, uint64_t targetGuid, SpellEffectValues const* effectValues)
{
    std::list<PlayableCard*> targets = SpellTargetSelector::GetTargets(effectValues->Target, attacker, victim, targetGuid);
    if (targets.empty())
        return false;

    for (std::list<PlayableCard*>::const_iterator iter = targets.begin(); iter != targets.end(); ++iter)
    {
        SpellAuraEffect auraEffect(*iter, effectValues->SpellId, effectValues->Value1, effectValues->Value2, effectValues->Value3, effectValues->Value4);
        (*iter)->ApplyAura(auraEffect);
    }

    return true;
}

bool SpellEffect::handleHeal(Player* attacker, Player* victim, uint64_t targetGuid, SpellEffectValues const* effectValues)
{
    std::list<PlayableCard*> targets = SpellTargetSelector::GetTargets(effectValues->Target, attacker, victim, targetGuid);
    if (targets.empty())
        return false;

    for (std::list<PlayableCard*>::iterator iter = targets.begin(); iter != targets.end(); ++iter)
        (*iter)->Heal(effectValues->Value1);

    return true;
}

SpellEffectFunc SpellEffect::GetSpellEffectFunc(uint8_t const& spellEffectId)
{
    return spellEffectId < MAX_SPELL_EFFECT_VALUE ? m_spellEffects[spellEffectId] : nullptr;
}
