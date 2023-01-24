# DiabloX

[![Discord Channel](https://img.shields.io/discord/936748966652022834?color=%237289DA&logo=discord&logoColor=%23FFFFFF)](https://discord.gg/QddvKsenvm)
[![Downloads](https://img.shields.io/github/downloads/ikonomov/DiabloX/total.svg)](https://github.com/ikonomov/DiabloX/releases)

DiabloX is a mod for Diablo 1 and a fork of [DevilutionX](https://github.com/diasurgical/devilutionX) with modifications to its code listed below.  The purpose of the changes is to increase the challenge by rebalancing various aspects of the game.

# How to Install

Note: You'll need access to the data from the original game. If you don't have an original CD then you can [buy Diablo from GoG.com](https://www.gog.com/game/diablo). Alternately you can use `spawn.mpq` from the [shareware](https://github.com/diasurgical/devilutionx-assets/releases/download/v2/spawn.mpq) version, in place of `DIABDAT.MPQ`, to play the shareware portion of the game.

- Download the latest [DiabloX release](https://github.com/ikonomov/DiabloX/releases) and extract the contents to a location of your choosing.

- Copy `DIABDAT.MPQ` from the CD or GOG-installation (or [extract it from the GoG installer](https://github.com/diasurgical/devilutionX/wiki/Extracting-the-.MPQs-from-the-GoG-installer)) to the DiabloX folder.

More detailed instructions can be found by following the [Installation Instructions](./docs/installing.md) for DevilutionX.  

# Test builds

If you want to help test the latest state of the next version you can fetch the build artifact from one of the build servers:

*Note: You must be logged into GitHub to download the attachments.*

[![Linux x86](https://github.com/ikonomov/DiabloX/actions/workflows/Linux_x86.yml/badge.svg)](https://github.com/ikonomov/DiabloX/actions/workflows/Linux_x86.yml?query=branch%3Amaster)
[![Linux x86-64 SDL1](https://github.com/ikonomov/DiabloX/actions/workflows/Linux_x86_64_SDL1.yml/badge.svg)](https://github.com/ikonomov/DiabloX/actions/workflows/Linux_x86_64_SDL1.yml?query=branch%3Amaster)
[![MacOSX](https://github.com/ikonomov/DiabloX/actions/workflows/MacOSX.yml/badge.svg)](https://github.com/ikonomov/DiabloX/actions/workflows/MacOSX.yml?query=branch%3Amaster)
[![Windows x64](https://github.com/ikonomov/DiabloX/actions/workflows/Windows_MSVC_x64.yml/badge.svg)](https://github.com/ikonomov/DiabloX/actions/workflows/Windows_MSVC_x64.yml?query=branch%3Amaster)
[![Windows MinGW x64](https://github.com/ikonomov/DiabloX/actions/workflows/Windows_MinGW_x64.yml/badge.svg)](https://github.com/ikonomov/DiabloX/actions/workflows/Windows_MinGW_x64.yml?query=branch%3Amaster)
[![Windows MinGW x86](https://github.com/ikonomov/DiabloX/actions/workflows/Windows_MinGW_x86.yml/badge.svg)](https://github.com/ikonomov/DiabloX/actions/workflows/Windows_MinGW_x86.yml?query=branch%3Amaster)
[![Android](https://github.com/ikonomov/DiabloX/actions/workflows/Android.yml/badge.svg)](https://github.com/ikonomov/DiabloX/actions/workflows/Android.yml?query=branch%3Amaster)
[![iOS](https://github.com/ikonomov/DiabloX/actions/workflows/iOS.yml/badge.svg)](https://github.com/ikonomov/DiabloX/actions/workflows/iOS.yml?query=branch%3Amaster)
[![PS4](https://github.com/ikonomov/DiabloX/actions/workflows/PS4.yml/badge.svg)](https://github.com/ikonomov/DiabloX/actions/workflows/PS4.yml?query=branch%3Amaster)
[![Original Xbox](https://github.com/ikonomov/DiabloX/actions/workflows/xbox_nxdk.yml/badge.svg)](https://github.com/ikonomov/DiabloX/actions/workflows/xbox_nxdk.yml?query=branch%3Amaster)
[![Xbox One/Series](https://github.com/ikonomov/DiabloX/actions/workflows/xbox_one.yml/badge.svg)](https://github.com/ikonomov/DiabloX/actions/workflows/xbox_one.yml?query=branch%3Amaster)

# Multiplayer:
   * ZeroTier does not need port forwarding or an extra application.
   * TCP/IP requires the host to expose port 6112.

# Saved Games and configuration file location:
`C:\Users\[username]\AppData\Roaming\ikonomov\DiabloX`

# [Summary of Changes in DevilutionX from Diablo](https://github.com/ikonomov/DiabloX/wiki/Summary-of-Changes-in-DevilutionX-from-Diablo)

# Changes in DiabloX from DevilutionX:
*original values listed in [square brackets]*
   * Characters:
      * The required experience points for each clvl have been increased by 100%.
      * Regenerate 1% of max mana per second.
      * +100% dexterity modifier to armor class formula when using melee weapons and no shield.
      * +50% strength modifier to damage formula when using two-handed melee weapons.
      * Sorcerer starts the game equipped with two Potions of Healing [Mana].
      * Block bonuses have been reduced to:
         * Warrior: 20 [30]
         * Rogue: 10 [20]
         * Sorcerer: 0 [10]
   * Monsters:
      * Health and damage has been increased by 50%.
      * Bonus points to health have been increased to 100 [1] in Nightmare and 300 [3] in Hell difficulty for multi player and 1/2 of the bonus points for single player.
      * Number per dlvl has been increased by 50% with a maximum of 250 [200].
      * The ilvl for item drops has been increased by 8 in nightmare and 16 in hell.
      * Black Deaths have been changed to reduce vitality instead of life by one point on successful hit.
      * Diablo has 100% more health, fastest hit recovery and resistance to Holy Bolt.
   * Shrines:
      * Fascinating, Ornate and Sacred reduce magic instead of mana by 10%.
      * Abandoned, Creepy, Eerie and Quiet increase attributes by 1 [2].
      * Hidden increases durability of items to a maximum of 250 [255].
   * Items:
      * The maximum ilvl for affixes of premium items sold by Griswold has been increased to 36 [30].
      * Effects of drinking Spectral Elixir: -1 to one attribute, +1 to all others [+3 to all attributes].
      * Potions cost 100% more gold.
      * Base armor values have been set to their maximum except:
         * Field Plate: 40 [40-45]
         * Plate Mail: 45 [42-50]
         * Gothic Plate: 48 [50-60]
         * Full Plate Mail: 50 [60-75]
      * qlvl for Elixir of Strength, Magic and Dexterity: 20 [15]
      * affix qlvl:
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
         * harmony: 24 [20]
         * ages: 24 [25]
      * Removed affixes:
         * readiness (no effect)
         * speed (same functionality as haste)
      * Renamed affixes:
         * [fine] to stout (armor)
         * [crimson] to russet (31-40)
         * [haste] to speed
      * All affixes with a range of values have been set to their maximum.
      * Damage versus demons affix: +50% [+200%]
      * The additional hit recovery reduction when equipping three or more items having all of the three suffixes (balance, stability and harmony) has been removed.
      * unique item qlvl:
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
      * Staff charges have been set to their maximum values.
      * Long War Bow has been restored to its original Diablo graphic.
   * Spells:
      * Fire Wall damage: 1/2 of the original
      * Apocalypse damage: 1/3 of the original
      * Elemental, Fireball, Flash damage: 1/4 of the original
      * Chain Lightning damage: min: 1 [2], max: (3+clvl)/2 [2+clvl]; creates 1+(slvl+5)/3 number of bolts that use the slvl and duration of Lightning
      * Damage reduction from Mana Shield has been removed.
      * Apocalypse only works on monsters within the line of sight.
      * Spell information inside the spell book has been modified with more accurate values.
      * The 1/2 spell damage modifier in player versus player has been removed.
      * The mana cost:
         * Town Portal initial: 53 [35]; minimum: 36 [18]
         * Phasing initial: 16 [12]; minimum: 8 [4]
         * Teleport initial: 50 [35]; minimum: 30 [15]
         * Stone Curse initial: 160 [60]; minimum: 100 [40]; decrease per slvl: 6 [3]
         * Fire Wall initial: 100 [28]; minimum: 60 [16]; decrease per slvl: 6 [2]
      * Stone Curse does not affect unique monsters.
   * Interface:
      * Lock-on target feature has been disabled.
      * Holding down left or right mouse buttons simulate multiple attacks.
      * The gold pile size has been increased to 20,000 [5,000].
      * Game name has been changed to DiabloX with a version coresponding to the latest DevilutionX release.
      * GameID has been changed to a unique identifier to prevent issues when joining multi player games with different builds.
      * Stash and character names have been assigned unique names and identifiers to prevent issues with different builds.
      * The application ID for Discord has been changed to DiabloX.
      * Textbox transparency for item labels and chat messages has been increased.
      * The game difficulty info has been moved from the automap to the hotkey `V` version info pop-up message.
      * Speed settings have been removed.
      * The diablo.ini has been optimized for consistent gameplay including:
         * disabled friendly damage from spells and ranged attacks to other players
         * disabled run in town
         * disabled Adria refilling mana
         * disabled not randomized quests
         * enabled all shrines
         * disabled quick cast
         * disabled auto refilling belt
         * disabled auto pickup except for gold (configurable for gold)
         * disabled auto equip except for weapons and shields (configurable for weapons and shields)
         * enabled zoom on start by default with a resolution of 1284x963
      * [Keyboard controls](https://github.com/ikonomov/DiabloX/wiki/Keyboard-Controls):
         * Help: H [F1]
         * Quick Save: G [F2]
         * Quick Spells: F1-F8 [F5-F8]
         * Pause: Pause [Pause or P]
         * unbound: Quick Load, increase/decrease gamma

# Credits

- [DevilutionX](https://github.com/diasurgical/devilutionX#credits)
- [DiabloX](https://github.com/ikonomov/DiabloX/graphs/contributors)

# Legal

DiabloX is made publicly available and released under the Sustainable Use License (see [LICENSE](LICENSE.md))

The source code in this repository is for non-commercial use only. If you use the source code you may not charge others for access to it or any derivative work thereof.

Diablo® - Copyright © 1996 Blizzard Entertainment, Inc. All rights reserved. Diablo and Blizzard Entertainment are trademarks or registered trademarks of Blizzard Entertainment, Inc. in the U.S. and/or other countries.

DiabloX and any of its maintainers are in no way associated with or endorsed by Blizzard Entertainment®.
