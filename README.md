# Bot Quiet Rolls

An [AzerothCore](https://www.azerothcore.org/) (WotLK 3.3.5a) module for
[mod-playerbots](https://github.com/mod-playerbots/mod-playerbots). Bots pass on group loot
without a line in the chat window.

Every time a group member passes on a roll, the client prints `<name> passed on: [item]`. Bots
pass on most of what drops, so a dungeon group with four bots prints up to four of those lines for
each item, and a 40-player raid prints dozens. The lines that matter (who needs, who won) scroll
away.

With this module:

- **A bot's pass prints nothing.** That covers a pass the bot chose, a pass because the roll timed
  out, and an automatic pass on an item the bot can't use.
- **Everything else still shows:** need, greed and disenchant choices, the rolled numbers, the
  winner, and "Everyone passed".
- **Real players are not touched.** Their passes show as before, to them and to the group.
- **The roll itself is not changed.** Bots vote as they did.
- **Server-side only.** No addon and no client patch.

## Patch Notes: Bot Quiet Rolls

Category: Bots

- Bots no longer fill the chat window with "passed on" lines during loot rolls. You still see who
  picked Need, Greed or Disenchant, what everyone rolled and who won.

## How it works

The server reports each vote to the group in an `SMSG_LOOT_ROLL` packet, and the client prints
the chat line when it receives one. The module checks those packets on their way out and drops
the ones that report a pass by a bot. Nothing else reads that packet: the roll window and the
winner use other packets.

## Requirements

- The [mod-playerbots](https://github.com/mod-playerbots/mod-playerbots) module on the playerbots
  core fork ([mod-playerbots/azerothcore-wotlk](https://github.com/mod-playerbots/azerothcore-wotlk),
  `Playerbot` branch) from its October 2026 core-align merge. The core needs
  `WorldSession::IsHeadless()`.
- A WoW 3.3.5a (12340) client. No SQL, no client patch.

## Installation

1. Clone into the server's `modules/` folder as `mod-bot-quiet-rolls` and rebuild. The folder name
   matters: AzerothCore derives the module's loader name from it.

   ```bash
   cd modules && git clone https://github.com/buildthehomelab/wow-mod-bot-quiet-rolls.git mod-bot-quiet-rolls
   ```

2. Copy `conf/mod_bot_quiet_rolls.conf.dist` to `mod_bot_quiet_rolls.conf` in your config folder.
   The default works as-is.

To check that it's loaded, look for this line in the worldserver log at startup:

```
mod-bot-quiet-rolls: enabled
```

## Configuration

| Option | Default | What it does |
| --- | --- | --- |
| `BotQuietRolls.Enable` | `1` | Master switch. |

## Troubleshooting

- **The `enabled` line is missing from the worldserver log:** the module isn't in the build.
  Re-run CMake so it picks up the folder, then rebuild. With Docker, rebuild the images.
- **A character still prints "passed on":** that character is a real player, or a player's own
  character running bot AI on a live client. Only sessions without a client count as bots.

## Credits

Author: [buildthehomelab](https://github.com/buildthehomelab)

## License

MIT. See [LICENSE](LICENSE).
