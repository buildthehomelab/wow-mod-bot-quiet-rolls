/*
 * mod-bot-quiet-rolls
 *
 * Bots pass on group loot without a line in the chat window. Every pass makes the client print
 * "<name> passed on: [item]", so a dungeon group with four bots prints up to four lines for each
 * item, and a raid prints dozens. This module doesn't send a bot's pass to the players.
 *
 * How: each vote goes out to the group as an SMSG_LOOT_ROLL packet, and the client prints the
 * line when it gets one. The module drops that packet when it reports a pass by a bot. Need,
 * greed and disenchant choices, the rolled numbers, the winner and "Everyone passed" still show,
 * and so does a pass by a real player.
 *
 * The roll itself is not changed. Needs an AzerothCore with WorldSession::IsHeadless() (the
 * playerbots core fork from its 2026-10 core-align on).
 *
 * Released under the MIT License.
 */

#include "ByteBuffer.h"
#include "Config.h"
#include "Log.h"
#include "LootMgr.h"
#include "ObjectAccessor.h"
#include "ObjectGuid.h"
#include "Opcodes.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "WorldPacket.h"
#include "WorldSession.h"

namespace
{
    bool enabled = true;

    // SMSG_LOOT_ROLL: uint64 item GUID, uint32 loot slot, uint64 voter GUID, uint32 item entry,
    // uint32 random suffix, uint32 random property, uint8 roll number, uint8 roll type,
    // uint8 automatic pass.
    constexpr std::size_t VOTER_POS = 8 + 4;
    constexpr std::size_t ROLL_NUMBER_POS = VOTER_POS + 8 + 4 + 4 + 4;
    constexpr std::size_t ROLL_TYPE_POS = ROLL_NUMBER_POS + 1;
    constexpr std::size_t LOOT_ROLL_SIZE = ROLL_TYPE_POS + 1 + 1;

    // The roll number of a packet that reports a choice and not a rolled number. A "selected
    // Need" packet has roll type 0 as well, but roll number 0.
    constexpr uint8 NO_ROLL_NUMBER = 128;

    bool IsBotPass(WorldPacket const& packet)
    {
        if (packet.size() < LOOT_ROLL_SIZE)
            return false;

        if (packet.read<uint8>(ROLL_NUMBER_POS) != NO_ROLL_NUMBER || packet.read<uint8>(ROLL_TYPE_POS) != ROLL_PASS)
            return false;

        // Bots are headless sessions (no socket).
        Player* voter = ObjectAccessor::FindConnectedPlayer(ObjectGuid(packet.read<uint64>(VOTER_POS)));
        return voter && voter->GetSession()->IsHeadless();
    }
}

class BotQuietRollsWorldScript : public WorldScript
{
public:
    BotQuietRollsWorldScript() : WorldScript("BotQuietRollsWorldScript") { }

    void OnAfterConfigLoad(bool /*reload*/) override
    {
        enabled = sConfigMgr->GetOption<bool>("BotQuietRolls.Enable", true);

        LOG_INFO("server.loading", "mod-bot-quiet-rolls: {}", enabled ? "enabled" : "disabled");
    }
};

class BotQuietRollsServerScript : public ServerScript
{
public:
    BotQuietRollsServerScript() : ServerScript("BotQuietRollsServerScript", { SERVERHOOK_CAN_PACKET_SEND }) { }

    // Runs for every packet sent to every client, so bail out on the opcode first. It doesn't
    // run for bots: they have no socket.
    bool CanPacketSend(WorldSession* /*session*/, WorldPacket const& packet) override
    {
        if (!enabled || packet.GetOpcode() != SMSG_LOOT_ROLL)
            return true;

        return !IsBotPass(packet);
    }
};

void AddBotQuietRollsScripts()
{
    new BotQuietRollsWorldScript();
    new BotQuietRollsServerScript();
}
