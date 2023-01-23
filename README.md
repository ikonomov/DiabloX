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
         * Character maximum stats:
      * Warrior's strength: 150 [250]
      * Rogue's dexterity: 150 [250]
      * Sorcerer's magic: 150 [250]
   * The 1/2 bow damage modifier and slower arrow travel speed applied to all classes except rogues has been removed.
   * The 1/2 spell damage modifier in player versus player has been removed.
   * Warriors:
      * Character damage with two-handed melee weapons has been increased to (strength+vitality)·clvl/100 [strength·clvl/100].
      * Gain fastest hit recovery and bonus points to armor equal to clvl when not using shields with one-handed or two-handed melee weapons.
      * Receive reduction to repair costs above 100 gold equal to 10·clvl.
   * Rogues:
      * Character damage with staves has been increased to (strength+dexterity+vitality)·clvl/200 [(strength+dexterity)·clvl/200].
      * Gain fastest hit recovery and bonus points to armor equal to clvl when not using shields with one-handed weapons or staves.
      * Spellcasting speed has been changed to 0.55 [0.60] seconds.
   * Sorcerers:
      * Character damage with staves has been increased to (strength+vitality)·clvl/100 [strength·clvl/100].
      * Gain fastest hit recovery and bonus points to armor equal to clvl/2 when using staves.
      * Receive reduction to recharge costs above 100 gold equal to 10·clvl.
      * Sorcerer starts the game equipped with two Potions of Healing [Mana].
   * Monks:
      * Gain resistance bonus equal to clvl.
      * Gain To Hit bonus equal to clvl when bare-handed.
      * Bonus points to armor has been increased to 3·clvl [2·clvl] when wearing light or no armor.
      * Gain fast block when bare-handed.
      * Search skill has been changed to Identify.
      * Spellcasting speed has been changed to 0.60 [0.65] seconds.
   * Attack speed has been changed to (seconds per hit):
      * Warrior equipped with a staff: 0.50 [0.55]
      * Rogue:
         * swords, clubs and unarmed: 0.45 [0.50]
         * staves: 0.45 [0.55]
      * Sorcerer:
         * swords, clubs, staves and unarmed: 0.50 [0.60]
         * axes and bows: 0.70 [0.80]
         * unarmed equipped with a shield: 0.50 [0.45]
      * Monk:
         * swords and clubs: 0.50 [0.60]
         * bows: 0.60 [0.70]
         * unarmed clvl 1-10: 0.4 [0.35]
         * unarmed clvl 11-25: 0.35 [0.35]
         * unarmed clvl 26-50: 0.3 [0.35]
         * unarmed equipped with a shield: 0.45 [0.35]
   * Block bonuses have been reduced to:
      * Warrior: 20 [30]
      * Rogue: 10 [20]
      * Sorcerer: 0 [10]
      * Monk: 15 [25]
   * Monsters:
      * Health and damage has been increased by 50%.
      * The number per dlvl has been increased by 50% with a maximum of 250 [200].
      * The ilvl for item drops has been increased by 8 in nightmare and 16 in hell.
      * Black Deaths have been changed to reduce vitality instead of life by one point on successful hit.
      * Diablo has more health, fastest hit recovery and resistance to Holy Bolt.
      * Monsters have bonus points to health of 50/100 [1] single/multi player in Nightmare and 100/200 [3] in Hell difficulty.
   * Shrines:
      * Fascinating, Ornate and Sacred reduce magic instead of mana by 10%.
      * Abandoned, Creepy, Eerie and Quiet increase stats by 1 [2].
   * Items:
      * The maximum ilvl for affixes of premium items sold by Griswold has been increased to 36 [30].
      * Spectral Elixir increases stats by 1 [3].
      * Potions cost two times more gold.
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
         * vicious: 12 [12]
         * brutal: 17 [16]
         * massive: 22 [20]
         * savage: 28 [23]
         * ruthless: 35 [35]
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
         * jade: 23
         * obsidian: 35
         * emerald: 43
         
         * ages: 24


         * swiftness: 18
         * speed: 25

         * harmony: 24
         
         * swiftness: 15 [10]
         * speed: 25 [19]
         * the ages: 24 [25]
         * Obsidian: 25 [24]
         * Sharp: 5 [1]
         * Warrior's: 7 [10]
         * Soldier's: 10 [15]
         * Lord's: 13 [19]
         * Knight's: 17 [23]
         * Master's: 21 [28]
         * Champion's: 24 [40]
         * Wyrm's: 29 [35]
         * Hydra's: 31 [60]
         * Weird: 27 [35]
         * Ruthless: 27 [35]
      * Crimson affix (31-40) has been renamed to Russet.
      * The description of the speed affix has been renamed to fastest [faster].
      * The additional hit recovery reduction when equipping three or more items having all of the three suffixes (of balance, of stability and of harmony) has been removed.
      * Removed affixes:
         * +200% damage vs. demons in the description of Deadly Hunter (unused)
         * Fine (weapons)
         * readiness (unused)
         * haste
         * Holy and Godly from shields
      * Affixes of all magical and unique items with a range of values have been set to their maximum except:
         * Sharp (+% To Hit): 10 [1-5]
         * Warrior's (+% Damage Done): 50 [51-65]
         * Soldier's (+% Damage Done): 65 [66-80]
         * Lord's (+% Damage Done): 80 [81-95]
         * Knight's (+% Damage Done): 95 [96-110]
         * Master's (+% Damage Done): 110 [111-125]
         * Champion's (+% Damage Done): 125 [126-150]
         * King's (+% Damage Done): 150 [151-175]
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
      * Base armor values have been set to their maximum except:
         * Kite Shield: 12 [8-15]
         * Gothic Shield: 14 [14-18]
         * Tower Shield: 16 [12-20]
         * Gothic Plate: 55 [50-60]
         * Full Plate Mail: 60 [60-75]
      * Staff charges have been set to their maximum values.
      * Long War Bow has been restored to its original Diablo graphic.
   * Spells:
      * Apocalypse damage min: (1+clvl)/2 [clvl], max: (1+clvl)·3/2 [6·clvl]
      * Elemental, Fireball, Flash, Nova damage: 1/4 of the original
      * Lightning duration: (slvl/2+4)/20 [(slvl/2+6)/20] seconds
      * Chain Lightning damage min: 1 [2], max: 2+clvl/5 [2+clvl]; creates 1+(slvl+5)/3 number of bolts that use the slvl and duration of Lightning
      * Damage reduction from Mana Shield has been removed.
      * Apocalypse only works on monsters within the line of sight.
      * Spell information inside the spell book has been modified with more accurate values.
      * The mana cost:
         * Town Portal initial: 53 [35], minimum: 36 [18]
         * Phasing initial: 16 [12], minimum: 8 [4]
         * Teleport initial: 50 [35], minimum: 30 [15]
         * Stone Curse initial: 120 [60], minimum: 80 [40], decrease per level: 6 [3]
      * Stone Curse:
         * Does not affect unique monsters.
         * Magic requirement for scrolls: 57 [33]
         * Magic requirement for staves and reading slvl 1 books: 87 [51]
         * Staff charges: 12 [8-16]
   * Interface:
      * Holding down left or right mouse buttons simulate multiple attacks.
      * The gold pile size has been increased to 20,000 [5,000].
      * Game name has been changed to DiabloX with a version coresponding to the latest DevilutionX release.
      * GameID has been changed to a unique identifier to prevent issues when joining multi player games with different builds.
      * The application ID for Discord has been changed to DiabloX.
      * Adjusted textbox transparency for item labels and chat messages.
      * Lock-on target feature has been disabled.
      * The game difficulty info has been moved from the automap to the hotkey `V` version info pop-up message.
      * Speed settings have been removed.
      * The diablo.ini has been optimized for consistent gameplay including:
         * disabled friendly damage from spells and ranged attacks to other players
         * disabled run in town
         * disabled Adria refilling mana
         * disabled not randomized quests
         * all shrines are enabled
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
   * Hellfire has been modified to function like Diablo:
      * Diablo and Na-Krul have been assigned mlvl 30.
      * One of the four drops from Na-Krul, book of Apocalypse, has been changed to a random book with qlvl of 8 or higher.
      * Traps.
      * Stores:
         * Wirt's price modifier (50% more expensive).
         * Maximum prices for items sold in town is 140,000 (135,000 at Wirt).
         * Wirt's inventory is not influenced by the character's class, stats or items already carried.
         * Griswold and Wirt do not sell staves.
         * Griswold's inventory is not influenced by the character's stats or items already carried.
         * Number of items in Griswold's inventory.
         * Number of items in Pepin's inventory.
         * Number of items in Adria's inventory.
         * Adria sells books at random.
      * Disabled blocking elemental attacks while having higher than 0% resistances.
      * Range of magma ball and spit attack.
      * Blocking does not deflect missiles.
      * Speed of arrows while using fast bows.
      * Damages target’s armor affix.
      * Removed (added by Hellfire):
         * affixes
         * unique items except quest items
         * spells, staves and scrolls
         * oils and runes
         * shrines
      * Elixir of Magic and Vitality do not restore mana or life.
      * Sorcerer starts the game equipped with Short Staff of Charged Bolt [Mana].
   * Diablo has been modified to function like Hellfire:
      * Sorcerer's mana cost for spells is 1/2 without going below the minimum.
      * Monsters health regeneration is not affected by having fire arrows equipped.
      * Scavengers' health regeneration when eating.
      * Monsters pause after killing a character.
   * The "hidden" unofficial features from Hellfire have been removed.

# Credits

- [DevilutionX](https://github.com/diasurgical/devilutionX#credits)
- [DiabloX](https://github.com/ikonomov/DiabloX/graphs/contributors)

# Legal

DiabloX is made publicly available and released under the Sustainable Use License (see [LICENSE](LICENSE.md))

The source code in this repository is for non-commercial use only. If you use the source code you may not charge others for access to it or any derivative work thereof.

Diablo® - Copyright © 1996 Blizzard Entertainment, Inc. All rights reserved. Diablo and Blizzard Entertainment are trademarks or registered trademarks of Blizzard Entertainment, Inc. in the U.S. and/or other countries.

DiabloX and any of its maintainers are in no way associated with or endorsed by Blizzard Entertainment®.
