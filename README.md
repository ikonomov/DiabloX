# DiabloX

[![Discord Channel](https://img.shields.io/discord/936748966652022834?color=%237289DA&logo=discord&logoColor=%23FFFFFF)](https://discord.gg/QddvKsenvm)
[![Downloads](https://img.shields.io/github/downloads/ikonomov/DiabloX/total.svg)](https://github.com/ikonomov/DiabloX/releases)

DiabloX is a mod for Diablo 1 and a fork of [DevilutionX](https://github.com/diasurgical/devilutionX) with modifications to its code listed below.  The purpose of the changes is to increase the challenge by rebalancing various aspects of the game.

## How to Install

Note: You'll need access to the data from the original game. If you don't have an original CD then you can [buy Diablo from GoG.com](https://www.gog.com/game/diablo).

   * Download the latest [DiabloX release](https://github.com/ikonomov/DiabloX/releases) and extract the contents to a folder of your choosing.
   * Copy `DIABDAT.MPQ` from the CD or GOG-installation to the same folder.

## Test builds

If you want to help test the latest development stage of the next version (make sure to backup your files as these may contain bugs), you can fetch the test build artifact from one of the build servers:

Note: You must be logged into GitHub to download the attachments.

[![Windows MinGW x64](https://github.com/ikonomov/DiabloX/actions/workflows/Windows_MinGW_x64.yml/badge.svg)](https://github.com/ikonomov/DiabloX/actions/workflows/Windows_MinGW_x64.yml?query=branch%3Amaster)

## Multiplayer:
   * ZeroTier does not need port forwarding or an extra application.
   * TCP/IP requires the host to expose port 6112.

## Saved Games and configuration file location:
`C:\Users\[username]\AppData\Roaming\ikonomov\DiabloX`

## [Summary of Changes in DevilutionX from Diablo](https://github.com/ikonomov/DiabloX/wiki/Summary-of-Changes-in-DevilutionX-from-Diablo)

## Changes in DiabloX from DevilutionX:
*The original values are listed in [square brackets].*
#### Characters:
   * Required clvl to create game:
      * Nightmare difficulty: 10 [20]
      * Hell difficulty: 20 [30]
   * Regenerate 1% (Sorcerer +0.01% per level) of max mana per second.
   * +50% strength modifier to damage formula when using two-handed melee weapons.
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
   * Chain Lightning creates a maximum of 3+slvl/4 number of bolts.
   * Lightning, Chain Lightning, Guardian, Elemental, Fireball and Flash do 1/2 of the original damage.
   * Fire Wall has 1/2 of the original duration.
   * Information inside the spell book has been modified with more accurate values.
   * Apocalypse only works on monsters within the line of sight.
   * Stone Curse does not affect unique monsters.
   * The casting sound (spell sound unchanged) for Phasing and Teleport has been removed.
   * Mana cost:
      * Sorcerer receives the same 25% reduction (doens't extend below the minimum) as Rogue for all spells.
      * Town Portal initial: 50 [35]; minimum: 35 [18]
      * Phasing initial: 16 [12]; minimum: 8 [4]
      * Teleport initial: 50 [35]; minimum: 30 [15]
      * Stone Curse initial: 100 [60]; minimum: 60 [40]; decrease per slvl: 6 [3]
      * Fire Wall initial: 40 [28]; minimum: 28 [16]
#### Items:
   * Adria sells scrolls only.
   * Griswold does not sell magical items.
   * Pepin does not sell potions except arena potions which can be used on cleared levels and purchased by a character that has killed Diablo on Hell difficulty.
   * Wirt sells staves in addition to his normal inventory.
   * The maximum price of items at Wirt has been reduced to 45,000 [135,000] gold.
   * Effects of drinking Spectral Elixir: +1 to one attribute [+3 to all attributes].
   * Damage versus demons affix:
      * Deadly Hunter: +50% [+200%]
      * Civerb's Cudgel: +150% [+200%]
   * Staff charges have been set to minimum values.
   * Set to maximum values:
      * Affixes with a range of values.
      * Base armor except:
         * Kite Shield: 12 [8-15]
         * Gothic Shield: 14 [14-18]
         * Tower Shield: 15 [12-20]
         * Field Plate: 40 [40-45]
         * Plate Mail: 42 [42-50]
         * Gothic Plate: 45 [50-60]
         * Full Plate Mail: 50 [60-75]
   * Burgundy affix has been renamed to russet.
   * Maximum base item ilvl for dungeon drops: +15 in Nightmare difficulty; +30 in Hell
   * Maximum affix ilvl:
      * Wirt: clvl [2·clvl]
      * dungeon drops: +1 in Normal difficulty; +9 in Nightmare; +17 in Hell
      * monster drops: +8 in Nightmare difficulty; +16 in Hell
<details>
  <summary>Qlvl (click to expand)</summary>

   * Qlvl:
      * Elixir of Strength, Magic and Dexterity: 20 [15]
      * Affix:
         * giants: 18 [17]
         * titans: 29 [23]
         * sorcery: 18 [17]
         * wizardry: 29 [23]
         * precision: 18 [17]
         * perfection: 29 [23]
         * vigor: 18 [17]
         * life: 29 [23]
         * moon: 13 [11]
         * stars: 24 [17]
         * heavens: 35 [25]
         * zodiac: 45 [30]
         * tiger: 22 [21]
         * lion: 31 [27]
         * mammoth: 38 [35]
         * whale: 45 [60]
         * drake: 22 [21]
         * dragon: 31 [27]
         * wyrm: 38 [35]
         * hydra: 45 [60]
         * godly: 46 [60]
         * steel: 8 [6]
         * silver: 13 [9]
         * gold: 18 [12]
         * platinum: 22 [16]
         * mithril: 27 [20]
         * meteoric: 32 [23]
         * weird: 38 [35]
         * strange: 46 [60]
         * champion: 35 [40]
         * king: 44 [28]
         * jagged: 1 [4]
         * deadly: 4 [6]
         * heavy: 8 [9]
         * vicious: 13 [12]
         * brutal: 18 [16]
         * massive: 22 [20]
         * savage: 27 [23]
         * ruthless: 32 [35]
         * merciless: 38 [60]
         * gore: 24 [25]
         * carnage: 32 [35]
         * slaughter: 42 [60]
         * osmosis: 30 [50]
         * thunder: 46 [60]
         * blood: 24 [19]
         * vampires: 24 [19]
         * amber: 14 [12]
         * jade: 22 [18]
         * obsidian: 32 [24]
         * emerald: 43 [31]
         * readiness: 8 [1]
         * swiftness: 17 [10]
         * speed: 24 [19]
         * haste: 32 [27]
         * harmony: 24 [20]
</details>

#### Shrines:
   * Fascinating, Ornate and Sacred reduce magic instead of mana by 2% (-1 minimum) [10%].
   * Abandoned, Creepy, Eerie and Quiet increase attributes by 1 [2].
   * Hidden increases durability of items to a maximum of 250 [255].
#### Monsters:
   * All monsters are able to walk through fire wall.
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
      * key binding for using health and mana potions from belt
      * quick cast
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
