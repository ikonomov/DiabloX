# DiabloX

[![Discord](https://img.shields.io/discord/936748966652022834?color=%237289DA&logo=discord&logoColor=%23FFFFFF)](https://discord.gg/TeUFFnarUV)
[![Downloads](https://img.shields.io/github/downloads/ikonomov/DiabloX/total.svg)](https://github.com/ikonomov/DiabloX/releases)

DiabloX is a mod for Diablo 1 and a fork of [DevilutionX](https://github.com/diasurgical/devilutionX) with modifications to its code listed below.  The purpose of the changes is to increase the challenge by rebalancing various aspects of the game.

## How to Install

Note: You'll need access to the data from the original game. If you don't have an original CD then you can [buy Diablo from GoG.com](https://www.gog.com/game/diablo).

   * Download the latest [DiabloX release](https://github.com/ikonomov/DiabloX/releases) and extract the contents to a folder of your choosing.
   * Copy `DIABDAT.MPQ` from the CD or GOG-installation to the same folder.

## Multiplayer:
   * ZeroTier does not need port forwarding or an extra application.
   * TCP/IP requires the host to expose port 6112.
   * [Discord server](https://discord.gg/TeUFFnarUV)

## Saved Games and configuration file location:
`%AppData%\devxmods\diablox`

## [Summary of Changes in DevilutionX from Diablo](https://github.com/ikonomov/DiabloX/wiki/Summary-of-Changes-in-DevilutionX-from-Diablo)

## Changes in DiabloX from DevilutionX:
*The original values are listed in [square brackets].*
#### Characters:
   * Required clvl to create game:
      * Nightmare difficulty: 10 [20]
      * Hell difficulty: 20 [30]
   * Regenerate 1% (Sorcerer +0.01% per level) of max mana per second.
   * Sorcerer receives the same 25% mana cost reduction (doesn't extend below the minimum) as Rogue for all spells.
   * +50% dexterity modifier to armor class formula when using melee weapons and no shield.
   * +50% character damage when using two-handed melee weapons.
   * Block bonus:
      * Warrior: 10 [30]
      * Rogue: 0 [20]
      * Sorcerer: 0 [10]
   * Maximum stats:
      * Warrior's strength: 150 [250]
      * Rogue's dexterity: 150 [250]
      * Sorcerer's magic: 150 [250]
   * The 1/2 spell damage modifier in player versus player is applied to all missiles.
#### Spells:
   * Chain Lightning creates a maximum of 3+slvl/5 number of bolts.
   * Guardian, Elemental, Fireball and Flash do 1/2 of the original damage.
   * Fire Wall has 1/2 of the original duration.
   * Information inside the spell book has been modified with more accurate values.
   * Apocalypse only works on monsters within the line of sight.
   * Stone Curse does not affect unique monsters.
   * Golems receive no damage from players in friendly mode.
   * The casting sound for Phasing and Teleport can be heard by other players after landing.
   * Mana cost:
      * Town Portal initial: 50 [35]; minimum: 35 [18]
      * Phasing initial: 16 [12]; minimum: 8 [4]
      * Teleport initial: 50 [35]; minimum: 30 [15]
#### Items:
   * Effects of drinking Spectral Elixir: +1 to one attribute [+3 to all attributes].
   * Damage versus demons affix [+200%]:
      * Deadly Hunter: +50%
      * Civerb's Cudgel: +150%
   * Damage is applied 7 times for Lightning and 9 times for Fire affixes.
   * Set to minimum values:
      * Staff charges.
      * Armor class for shields and armor.
   * Set to maximum values:
      * Armor class for helms.
      * Affixes with a range of values.
   * Burgundy affix has been renamed to Russet.
   * All unique items are findable.
   * Slain Hero drops:
      * Warrior: Full Plate Mail in Nightmare and Hell difficulty
      * Rogue: Long War Bow in Nightmare and Hell difficulty
      * Sorcerer: Book of Fireball in Nightmare and Book of Chain Lightning in Hell difficulty
   * Town portal scrolls dropped as special items (from chests and barrels, replacing Mana or Healing potions) can be found on levels 2-8 [2-15].
   * Maximum base item ilvl for dungeon drops (not from monsters): +15 in Nightmare difficulty; +30 in Hell
   * Maximum affix ilvl:
      * dungeon drops (not from monsters): -1 in Normal difficulty; +9 in Nightmare; +19 in Hell
      * monster drops: +10 in Nightmare difficulty; +20 in Hell
<details>
  <summary>Value in gold (click to expand)</summary>

   * Value in gold:
      * Potion:
         * Healing: 3,000 [50]
         * Mana: 3,000 [50]
         * Full Healing: 9,000 [150]
         * Full Mana: 9,000 [150]
         * Rejuvenation: 7,200 [120]
         * Full Rejuvenation: 36,000 [600]
      * Elixirs: 10,000 [5,000]
      * Scroll:
         * Healing: 1,000 [50]
         * Resurrect: 5,000 [250]
         * Identify: 500 [100]
         * Town Portal: 2,000 [200]
         * Infravision: 1,200 [600]
         * Mana Shield: 2,400 [1,200]
      * Book:
         * Healing: 10,000 [1,000]
         * Heal Other: 10,000 [1,000]
         * Town Portal: 15,000 [3,000]
      * Staff multiplier:
         * Healing: 100 [10]
         * Heal Other: 100 [10]
         * Resurrect: 500 [50]
         * Town Portal: 200 [40]
</details>

<details>
  <summary>Qlvl (click to expand)</summary>

   * Qlvl:
      * Elixir of Strength, Magic and Dexterity: 20 [15]
      * [Affix](https://github.com/ikonomov/DiabloX/wiki/Affixes-(DiabloX)):
         * Giants: 18 [17]
         * Titans: 29 [23]
         * Sorcery: 18 [17]
         * Wizardry: 29 [23]
         * Precision: 18 [17]
         * Perfection: 29 [23]
         * Vigor: 18 [17]
         * Life: 29 [23]
         * Trouble: 15 [12]
         * Moon: 14 [11]
         * Stars: 25 [17]
         * Heavens: 36 [25]
         * Zodiac: 50 [30]
         * Tiger: 22 [21]
         * Lion: 31 [27]
         * Mammoth: 39 [35]
         * Whale: 48 [60]
         * Drake: 22 [21]
         * Dragon: 31 [27]
         * Wyrm: 39 [35]
         * Hydra: 48 [60]
         * Awesome: 30 [28]
         * Holy: 38 [35]
         * Godly: 49 [60]
         * Steel: 8 [6]
         * Silver: 13 [9]
         * Gold: 19 [12]
         * Platinum: 23 [16]
         * Mithril: 27 [20]
         * Meteoric: 32 [23]
         * Weird: 40 [35]
         * Strange: 47 [60]
         * Champion: 35 [40]
         * King: 45 [28]
         * Jagged: 1 [4]
         * Deadly: 4 [6]
         * Heavy: 8 [9]
         * Vicious: 13 [12]
         * Brutal: 19 [16]
         * Massive: 23 [20]
         * Savage: 27 [23]
         * Ruthless: 32 [35]
         * Merciless: 40 [60]
         * Gore: 24 [25]
         * Carnage: 33 [35]
         * Slaughter: 42 [60]
         * Ages: 35 [25]
         * Osmosis: 30 [50]
         * Thunder: 44 [60]
         * Blood: 25 [19]
         * Vampires: 25 [19]
         * Amber: 14 [12]
         * Jade: 22 [18]
         * Obsidian: 32 [24]
         * Emerald: 43 [31]
         * Readiness: 8 [1]
         * Swiftness: 17 [10]
         * Speed: 25 [19]
         * Haste: 32 [27]
         * Harmony: 26 [20]
         * Bear: 19 [5]
         * Blocking: 17 [5]

</details>

#### Stores:
   * Adria and Pepin do not sell items in unlimited quantities except Arena Potions which can be used on cleared levels.
   * Cain charges 500 [100] gold to identify items.
   * Wirt charges 50 extra gold for every 5 clvls to show his item for sale.
   * Wirt's sale price increase has been changed to 100% [50%].
   * The maximum store price has been changed to 200,000 [140,000/135,000] gold.
   * Maximum affix qlvl:
      * Adria: 2×ilvl+1 [2×ilvl]
      * Griswold: 32 [30]
      * Wirt: (45×clvl/(clvl+40))×2+1 [2×clvl]
#### Shrines:
   * Fascinating, Ornate and Sacred reduce magic by -2% (-1 minimum) [-10% mana].
   * Abandoned, Creepy, Eerie and Quiet increase attributes by 1 [2].
   * Hidden increases durability of items to a maximum of 250 [255].
#### Monsters:
   * All monsters are able to walk through Fire Wall.
   * Black Deaths have been changed to reduce vitality instead of life by one point on successful hit.
   * Diablo has 100% more health, fastest hit recovery and resistance to Holy Bolt.
#### Engine:
   * Removed:
      * single player mode
      * support for Spawned Diablo and Diablo: Hellfire
      * multi-language support
      * arenas
      * speed settings
      * run in town
      * friendly damage from spells and ranged attacks to other players
      * Adria refilling mana
      * option to disable randomized quests
      * option to disable harmful shrines
      * town automap
   * Disabled automation options:
      * pickup except for gold (configurable for gold)
      * equip except for weapons and shields (configurable for weapons and shields)
      * refilling belt
      * quick cast
   * Half the gold not dropped after being killed is lost (no gold will be lost on level 16).
   * Restart In Town has been renamed to Resurrect In Town.
   * The lock-on target functionality has been replaced by rapid fire when playing with a mouse.
   * Gold pile size has been increased to 20,000 [5,000].
   * The game name and application ID for Discord have been changed to DiabloX.
   * GameID, Stash and character files have been assigned unique names and identifiers to prevent issues with different builds.
   * The game difficulty info has been moved from the automap to the version info pop-up message (hotkey `V`).
   * Help menu has been replaced by a list of shrine descriptions available after reaching clvl 17.
   * [Keyboard controls](https://github.com/ikonomov/DiabloX/wiki/Keyboard-Controls):
      * Shrine List: D [Help: F1]
      * Quick Spells: F1-F8 [F5-F8]

## [Credits](https://github.com/ikonomov/DiabloX/graphs/contributors)

## Legal

DiabloX is made publicly available and released under the Sustainable Use License (see [LICENSE](LICENSE.md))

The source code in this repository is for non-commercial use only. If you use the source code you may not charge others for access to it or any derivative work thereof.

Diablo® - Copyright © 1996 Blizzard Entertainment, Inc. All rights reserved. Diablo and Blizzard Entertainment are trademarks or registered trademarks of Blizzard Entertainment, Inc. in the U.S. and/or other countries.

DiabloX and any of its maintainers are in no way associated with or endorsed by Blizzard Entertainment®.
