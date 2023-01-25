# DiabloX

[![Discord Channel](https://img.shields.io/discord/936748966652022834?color=%237289DA&logo=discord&logoColor=%23FFFFFF)](https://discord.gg/QddvKsenvm)
[![Downloads](https://img.shields.io/github/downloads/ikonomov/DiabloX/total.svg)](https://github.com/ikonomov/DiabloX/releases)

DiabloX is a mod for Diablo 1 and a fork of [DevilutionX](https://github.com/diasurgical/devilutionX) with modifications to its code listed below.  The purpose of the changes is to increase the challenge by rebalancing various aspects of the game.

## How to Install

Note: You'll need access to the data from the original game. If you don't have an original CD then you can [buy Diablo from GoG.com](https://www.gog.com/game/diablo).

   * Download the latest [DiabloX release](https://github.com/ikonomov/DiabloX/releases) and extract the contents to a location of your choosing.
   * Copy `DIABDAT.MPQ` from the CD or GOG-installation (or [extract it from the GoG installer](https://github.com/diasurgical/devilutionX/wiki/Extracting-the-.MPQs-from-the-GoG-installer)) to the DiabloX folder.

More detailed instructions can be found by following the [Installation Instructions](./docs/installing.md) for DevilutionX.  

## Test builds

If you want to help test the latest state of the next version you can fetch the build artifact from one of the build servers:

*Note: You must be logged into GitHub to download the attachments.*

[![Windows MinGW x64](https://github.com/ikonomov/DiabloX/actions/workflows/Windows_MinGW_x64.yml/badge.svg)](https://github.com/ikonomov/DiabloX/actions/workflows/Windows_MinGW_x64.yml?query=branch%3Amaster)
[![Windows MinGW x86](https://github.com/ikonomov/DiabloX/actions/workflows/Windows_MinGW_x86.yml/badge.svg)](https://github.com/ikonomov/DiabloX/actions/workflows/Windows_MinGW_x86.yml?query=branch%3Amaster)

## Multiplayer:
   * ZeroTier does not need port forwarding or an extra application.
   * TCP/IP requires the host to expose port 6112.

## Saved Games and configuration file location:
`C:\Users\[username]\AppData\Roaming\ikonomov\DiabloX`

## [Summary of Changes in DevilutionX from Diablo](https://github.com/ikonomov/DiabloX/wiki/Summary-of-Changes-in-DevilutionX-from-Diablo)

## Changes in DiabloX from DevilutionX:
*The original values are listed in [square brackets].*
#### Characters:
   * The required experience points for each clvl have been increased by 100%.
   * Regenerate 1% of max mana per second.
   * +100% dexterity modifier to armor class formula when using melee weapons and no shield.
   * +50% strength modifier to damage formula when using two-handed melee weapons.
   * Sorcerer starts the game equipped with two Potions of Healing [Mana].
   * Maximum stats:
      * Warrior's strength: 150 [250]
      * Rogue's dexterity: 150 [250]
      * Sorcerer's magic: 150 [250]
   * Block bonuses:
      * Warrior: 20 [30]
      * Rogue: 10 [20]
      * Sorcerer: 0 [10]
#### Spells:
   * Damage:
      * Fire Wall: 1/2 of the original
      * Apocalypse: 1/3 of the original
      * Elemental, Fireball, Flash: 1/4 of the original
      * Chain Lightning: min: 1 [2], max: (3+clvl)/2 [2+clvl]; creates 1+(slvl+5)/3 number of bolts that use the slvl and duration of Lightning
   * The 1/2 spell damage modifier in player versus player has been removed.
   * Information inside the spell book has been modified with more accurate values.
   * Apocalypse only works on monsters within the line of sight.
   * Stone Curse does not affect unique monsters.
   * Mana cost:
      * Town Portal initial: 53 [35]; minimum: 36 [18]
      * Phasing initial: 16 [12]; minimum: 8 [4]
      * Teleport initial: 50 [35]; minimum: 30 [15]
      * Stone Curse initial: 160 [60]; minimum: 100 [40]; decrease per slvl: 6 [3]
      * Fire Wall initial: 100 [28]; minimum: 60 [16]; decrease per slvl: 6 [2]
#### Items:
   * Potions cost 100% more gold.
   * Long War Bow has been restored to its original Diablo graphic.
   * The maximum ilvl for affixes of premium items sold by Griswold has been increased to 36 [30].
   * Effects of drinking Spectral Elixir: -1 to one attribute, +1 to all others [+3 to all attributes].
   * The additional hit recovery reduction when equipping three or more items having all of the three suffixes (balance, stability and harmony) has been removed.
   * Damage versus demons affix: +50% [+200%].
   * Set to maximum values:
      * Affixes with a range of values.
      * Staff charges.
      * Base armor except:
         * Kite Shield: 12 [8-15]
         * Gothic Shield: 14 [14-18]
         * Tower Shield: 15 [12-20]
         * Gothic Plate: 55 [50-60]
         * Full Plate Mail: 60 [60-75]
   * Removed affixes:
      * readiness (has no effect)
      * speed (same functionality as haste)
   * Renamed affixes:
      * fine (armor) to stout
      * crimson (31-40) to russet
      * haste to speed
<details>
  <summary>Qlvl (click to expand)</summary>

   * Qlvl:
      * Elixir of Strength, Magic and Dexterity: 20 [15]
      * Affix
         * moon: 13 [11]
         * stars: 24 [17]
         * heavens: 35 [25]
         * zodiac: 46 [30]
         * whale: 46 [60]
         * snake: 12 [9]
         * serpent: 22 [15]
         * drake: 35 [21]
         * dragon: 43 [27]
         * wyrm: 46 [35]
         * hydra: 50 [60]
         * godly: 44 [60]
         * steel: 8 [6]
         * silver: 12 [9]
         * gold: 17 [12]
         * platinum: 22 [16]
         * mithril: 28 [20]
         * meteoric: 35 [23]
         * weird: 43 [35]
         * strange: 50 [60]
         * master: 29 [28]
         * champion: 36 [40]
         * king: 44 [28]
         * jagged: 1 [4]
         * deadly: 4 [6]
         * heavy: 8 [9]
         * brutal: 17 [16]
         * massive: 22 [20]
         * savage: 28 [23]
         * merciless: 43 [60]
         * slaying: 13 [15]
         * gore: 20 [25]
         * carnage: 27 [35]
         * slaughter: 35 [60]
         * osmosis: 30 [50]
         * thunder: 50 [60]
         * blood: 24 [19]
         * vampires: 24 [19]
         * amber: 14 [12]
         * jade: 23 [18]
         * obsidian: 35 [24]
         * emerald: 43 [31]
         * swiftness: 18 [10]
         * speed: 25 [haste: 27]
         * harmony: 24 [20]
         * ages: 24 [25]
      * Unique item:
         * Bloodslayer: 10 [3]
         * The Mangler: 9 [2]
         * Wicked Axe: 12 [5]
         * The Blackoak Bow: 8 [5]
         * Deadly Hunter: 10 [3]
         * Civerb’s Cudgel: 10 [1]
         * Lightforge: 14 [1]
         * Helm of Spirits: 14 [1]
         * The Bleeder: 9 [2]
         * Bramble: 7 [1]
         * Constricting Ring: 15 [5]
         * Split Skull Shield: 4 [1]
         * Immolator: 7 [4]
         * Gonnagal’s Dirk: 5 [1]
         * Shadowhawk: 12 [8]
</details>

#### Shrines:
   * Fascinating, Ornate and Sacred reduce magic instead of mana by 10%.
   * Abandoned, Creepy, Eerie and Quiet increase attributes by 1 [2].
   * Hidden increases durability of items to a maximum of 250 [255].
#### Monsters:
   * Damage has been increased by 50%.
   * Bonus points to health have been increased to 100 [1] in Nightmare and 300 [3] in Hell difficulty for multi player and 1/2 of the bonus points for single player.
   * Number per dlvl has been increased by 50% with a maximum of 250 [200].
   * Ilvl for item drops has been increased by 8 in nightmare and 16 in hell.
   * Black Deaths have been changed to reduce vitality instead of life by one point on successful hit.
   * Diablo has 100% more health, fastest hit recovery and resistance to Holy Bolt.
#### Interface:
   * The Lock-on target feature has been disabled.
   * Holding down left or right mouse buttons simulate multiple attacks.
   * Gold pile size has been increased to 20,000 [5,000].
   * The game name and application ID for Discord have been changed to DiabloX.
   * GameID, Stash and character names have been assigned unique names and identifiers to prevent issues with different builds.
   * The textbox for item labels and chat messages has been made transparent.
   * The game difficulty info has been moved from the automap to the version info pop-up message (hotkey `V`).
   * The speed settings have been removed.
   * [Keyboard controls](https://github.com/ikonomov/DiabloX/wiki/Keyboard-Controls):
      * Help: H [F1]
      * Quick Save: G [F2]
      * Quick Spells: F1-F8 [F5-F8]
      * Pause: Pause [Pause or P]
      * unbound: Quick Load, increase/decrease gamma
   * DevilutionX settings:
      * disabled support for Spawned Diablo and Diablo: Hellfire.
      * disabled friendly damage from spells and ranged attacks to other players
      * disabled run in town
      * disabled Adria refilling mana
      * enabled randomized quests
      * enabled all shrines
      * disabled quick cast
      * disabled auto refilling belt
      * disabled auto pickup except for gold (configurable for gold)
      * disabled auto equip except for weapons and shields (configurable for weapons and shields)
      * enabled zoom on start by default with a resolution of 1284x963

## Credits

   * [DevilutionX](https://github.com/diasurgical/devilutionX#credits)
   * [DiabloX](https://github.com/ikonomov/DiabloX/graphs/contributors)

## Legal

DiabloX is made publicly available and released under the Sustainable Use License (see [LICENSE](LICENSE.md))

The source code in this repository is for non-commercial use only. If you use the source code you may not charge others for access to it or any derivative work thereof.

Diablo® - Copyright © 1996 Blizzard Entertainment, Inc. All rights reserved. Diablo and Blizzard Entertainment are trademarks or registered trademarks of Blizzard Entertainment, Inc. in the U.S. and/or other countries.

DiabloX and any of its maintainers are in no way associated with or endorsed by Blizzard Entertainment®.
