/**
 * @file monster.cpp
 *
 * Implementation of monster functionality, AI, actions, spawning, loading, etc.
 */
#include "monster.h"

#include <algorithm>
#include <array>
#include <climits>

#include <fmt/format.h>

#include "control.h"
#include "cursor.h"
#include "dead.h"
#include "drlg_l1.h"
#include "drlg_l4.h"
#include "engine/cel_header.hpp"
#include "engine/load_file.hpp"
#include "engine/random.hpp"
#include "engine/render/cl2_render.hpp"
#include "init.h"
#include "lighting.h"
#include "minitext.h"
#include "missiles.h"
#include "movie.h"
#include "options.h"
#include "spelldat.h"
#include "storm/storm.h"
#include "themes.h"
#include "towners.h"
#include "trigs.h"
#include "utils/language.h"

#ifdef _DEBUG
#include "debug.h"
#endif

namespace devilution {

#define NIGHTMARE_TO_HIT_BONUS 85
#define HELL_TO_HIT_BONUS 120

#define NIGHTMARE_AC_BONUS 50
#define HELL_AC_BONUS 80

/** Tracks which missile files are already loaded */
int MissileFileFlag;

// BUGFIX: replace MonsterKillCounts[MAXMONSTERS] with MonsterKillCounts[NUM_MTYPES].
/** Tracks the total number of monsters killed per monster_id. */
int MonsterKillCounts[MAXMONSTERS];
int ActiveMonsters[MAXMONSTERS];
int ActiveMonsterCount;
bool sgbSaveSoundOn;
MonsterStruct Monsters[MAXMONSTERS];
int totalmonsters;
CMonster LevelMonsterTypes[MAX_LVLMTYPES];
int monstimgtot;
int uniquetrans;
int LevelMonsterTypeCount;

/* data */

// BUGFIX: MWVel velocity values are not rounded consistently. The correct
// formula for monster walk velocity is calculated as follows (for 16, 32 and 64
// pixel distances, respectively):
//
//    vel16 = (16 << monsterWalkShift) / nframes
//    vel32 = (32 << monsterWalkShift) / nframes
//    vel64 = (64 << monsterWalkShift) / nframes
//
// The correct monster walk velocity table is as follows:
//
//   int MWVel[24][3] = {
//      { 256, 512, 1024 },
//      { 128, 256, 512 },
//      { 85, 171, 341 },
//      { 64, 128, 256 },
//      { 51, 102, 205 },
//      { 43, 85, 171 },
//      { 37, 73, 146 },
//      { 32, 64, 128 },
//      { 28, 57, 114 },
//      { 26, 51, 102 },
//      { 23, 47, 93 },
//      { 21, 43, 85 },
//      { 20, 39, 79 },
//      { 18, 37, 73 },
//      { 17, 34, 68 },
//      { 16, 32, 64 },
//      { 15, 30, 60 },
//      { 14, 28, 57 },
//      { 13, 27, 54 },
//      { 13, 26, 51 },
//      { 12, 24, 49 },
//      { 12, 23, 47 },
//      { 11, 22, 45 },
//      { 11, 21, 43 }
//   };

/** Maps from monster walk animation frame num to monster velocity. */
int MWVel[24][3] = {
	{ 256, 512, 1024 },
	{ 128, 256, 512 },
	{ 85, 170, 341 },
	{ 64, 128, 256 },
	{ 51, 102, 204 },
	{ 42, 85, 170 },
	{ 36, 73, 146 },
	{ 32, 64, 128 },
	{ 28, 56, 113 },
	{ 26, 51, 102 },
	{ 23, 46, 93 },
	{ 21, 42, 85 },
	{ 19, 39, 78 },
	{ 18, 36, 73 },
	{ 17, 34, 68 },
	{ 16, 32, 64 },
	{ 15, 30, 60 },
	{ 14, 28, 57 },
	{ 13, 26, 54 },
	{ 12, 25, 51 },
	{ 12, 24, 48 },
	{ 11, 23, 46 },
	{ 11, 22, 44 },
	{ 10, 21, 42 }
};
/** Maps from monster action to monster animation letter. */
char animletter[7] = "nwahds";
/** Maps from direction to a left turn from the direction. */
Direction left[8] = { DIR_SE, DIR_S, DIR_SW, DIR_W, DIR_NW, DIR_N, DIR_NE, DIR_E };
/** Maps from direction to a right turn from the direction. */
Direction right[8] = { DIR_SW, DIR_W, DIR_NW, DIR_N, DIR_NE, DIR_E, DIR_SE, DIR_S };
/** Maps from direction to the opposite direction. */
Direction opposite[8] = { DIR_N, DIR_NE, DIR_E, DIR_SE, DIR_S, DIR_SW, DIR_W, DIR_NW };

/** Maps from monster AI ID to monster AI function. */
void (*AiProc[])(int i) = {
	&MAI_Zombie,
	&MAI_Fat,
	&MAI_SkelSd,
	&MAI_SkelBow,
	&MAI_Scav,
	&MAI_Rhino,
	&MAI_GoatMc,
	&MAI_GoatBow,
	&MAI_Fallen,
	&MAI_Magma,
	&MAI_SkelKing,
	&MAI_Bat,
	&MAI_Garg,
	&MAI_Cleaver,
	&MAI_Succ,
	&MAI_Sneak,
	&MAI_Storm,
	&MAI_Fireman,
	&MAI_Garbud,
	&MAI_Acid,
	&MAI_AcidUniq,
	&MAI_Golum,
	&MAI_Zhar,
	&MAI_SnotSpil,
	&MAI_Snake,
	&MAI_Counselor,
	&MAI_Mega,
	&MAI_Diablo,
	&MAI_Lazurus,
	&MAI_Lazhelp,
	&MAI_Lachdanan,
	&MAI_Warlord,
	&MAI_Firebat,
	&MAI_Torchant,
	&MAI_HorkDemon,
	&MAI_Lich,
	&MAI_ArchLich,
	&MAI_Psychorb,
	&MAI_Necromorb,
	&MAI_BoneDemon
};

void InitMonsterTRN(CMonster &monst)
{
	std::array<uint8_t, 256> colorTranslations;
	LoadFileInMem(monst.MData->TransFile, colorTranslations);

	std::replace(colorTranslations.begin(), colorTranslations.end(), 255, 0);

	int n = monst.MData->has_special ? 6 : 5;
	for (int i = 0; i < n; i++) {
		if (i == 1 && monst.mtype >= MT_COUNSLR && monst.mtype <= MT_ADVOCATE) {
			continue;
		}

		for (int j = 0; j < 8; j++) {
			Cl2ApplyTrans(
			    CelGetFrame(monst.Anims[i].CMem.get(), j),
			    colorTranslations,
			    monst.Anims[i].Frames);
		}
	}
}

void InitLevelMonsters()
{
	int i;

	LevelMonsterTypeCount = 0;
	monstimgtot = 0;
	MissileFileFlag = 0;

	for (i = 0; i < MAX_LVLMTYPES; i++) {
		LevelMonsterTypes[i].mPlaceFlags = 0;
	}

	ClrAllMonsters();
	ActiveMonsterCount = 0;
	totalmonsters = MAXMONSTERS;

	for (i = 0; i < MAXMONSTERS; i++) {
		ActiveMonsters[i] = i;
	}

	uniquetrans = 0;
}

int AddMonsterType(_monster_id type, placeflag placeflag)
{
	bool done = false;
	int i;

	for (i = 0; i < LevelMonsterTypeCount && !done; i++) {
		done = LevelMonsterTypes[i].mtype == type;
	}

	i--;

	if (!done) {
		i = LevelMonsterTypeCount;
		LevelMonsterTypeCount++;
		LevelMonsterTypes[i].mtype = type;
		monstimgtot += MonsterData[type].mImage;
		InitMonsterGFX(i);
		InitMonsterSND(i);
	}

	LevelMonsterTypes[i].mPlaceFlags |= placeflag;
	return i;
}

void GetLevelMTypes()
{
	int i;

	// this array is merged with skeltypes down below.
	_monster_id typelist[MAXMONSTERS];
	_monster_id skeltypes[NUM_MTYPES];

	int minl; // min level
	int maxl; // max level
	char mamask;
	const int numskeltypes = 19;

	int nt; // number of types

	if (gbIsSpawn)
		mamask = 1; // monster availability mask
	else
		mamask = 3; // monster availability mask

	AddMonsterType(MT_GOLEM, PLACE_SPECIAL);
	if (currlevel == 16) {
		AddMonsterType(MT_ADVOCATE, PLACE_SCATTER);
		AddMonsterType(MT_RBLACK, PLACE_SCATTER);
		AddMonsterType(MT_DIABLO, PLACE_SPECIAL);
		return;
	}

	if (currlevel == 18)
		AddMonsterType(MT_HORKSPWN, PLACE_SCATTER);
	if (currlevel == 19) {
		AddMonsterType(MT_HORKSPWN, PLACE_SCATTER);
		AddMonsterType(MT_HORKDMN, PLACE_UNIQUE);
	}
	if (currlevel == 20)
		AddMonsterType(MT_DEFILER, PLACE_UNIQUE);
	if (currlevel == 24) {
		AddMonsterType(MT_ARCHLICH, PLACE_SCATTER);
		AddMonsterType(MT_NAKRUL, PLACE_SPECIAL);
	}

	if (!setlevel) {
		if (QuestStatus(Q_BUTCHER))
			AddMonsterType(MT_CLEAVER, PLACE_SPECIAL);
		if (QuestStatus(Q_GARBUD))
			AddMonsterType(UniqMonst[UMT_GARBUD].mtype, PLACE_UNIQUE);
		if (QuestStatus(Q_ZHAR))
			AddMonsterType(UniqMonst[UMT_ZHAR].mtype, PLACE_UNIQUE);
		if (QuestStatus(Q_LTBANNER))
			AddMonsterType(UniqMonst[UMT_SNOTSPIL].mtype, PLACE_UNIQUE);
		if (QuestStatus(Q_VEIL))
			AddMonsterType(UniqMonst[UMT_LACHDAN].mtype, PLACE_UNIQUE);
		if (QuestStatus(Q_WARLORD))
			AddMonsterType(UniqMonst[UMT_WARLORD].mtype, PLACE_UNIQUE);

		if (gbIsMultiplayer && currlevel == quests[Q_SKELKING]._qlevel) {

			AddMonsterType(MT_SKING, PLACE_UNIQUE);

			nt = 0;
			for (i = MT_WSKELAX; i <= MT_WSKELAX + numskeltypes; i++) {
				if (IsSkel(i)) {
					minl = 15 * MonsterData[i].mMinDLvl / 30 + 1;
					maxl = 15 * MonsterData[i].mMaxDLvl / 30 + 1;

					if (currlevel >= minl && currlevel <= maxl) {
						if ((MonstAvailTbl[i] & mamask) != 0) {
							skeltypes[nt++] = (_monster_id)i;
						}
					}
				}
			}
			AddMonsterType(skeltypes[GenerateRnd(nt)], PLACE_SCATTER);
		}

		nt = 0;
		for (i = MT_NZOMBIE; i < NUM_MTYPES; i++) {
			minl = 15 * MonsterData[i].mMinDLvl / 30 + 1;
			maxl = 15 * MonsterData[i].mMaxDLvl / 30 + 1;

			if (currlevel >= minl && currlevel <= maxl) {
				if ((MonstAvailTbl[i] & mamask) != 0) {
					typelist[nt++] = (_monster_id)i;
				}
			}
		}

#ifdef _DEBUG
		if (monstdebug) {
			for (i = 0; i < debugmonsttypes; i++)
				AddMonsterType(DebugMonsters[i], PLACE_SCATTER);
		} else
#endif
		{

			while (nt > 0 && LevelMonsterTypeCount < MAX_LVLMTYPES && monstimgtot < 4000) {
				for (i = 0; i < nt;) {
					if (MonsterData[typelist[i]].mImage > 4000 - monstimgtot) {
						typelist[i] = typelist[--nt];
						continue;
					}

					i++;
				}

				if (nt != 0) {
					i = GenerateRnd(nt);
					AddMonsterType(typelist[i], PLACE_SCATTER);
					typelist[i] = typelist[--nt];
				}
			}
		}

	} else {
		if (setlvlnum == SL_SKELKING) {
			AddMonsterType(MT_SKING, PLACE_UNIQUE);
		}
	}
}

void InitMonsterGFX(int monst)
{
	int mtype = LevelMonsterTypes[monst].mtype;
	int width = MonsterData[mtype].width;

	for (int anim = 0; anim < 6; anim++) {
		int frames = MonsterData[mtype].Frames[anim];
		if (gbIsHellfire && mtype == MT_DIABLO && anim == 3)
			frames = 2;

		if ((animletter[anim] != 's' || MonsterData[mtype].has_special) && frames > 0) {
			char strBuff[256];
			sprintf(strBuff, MonsterData[mtype].GraphicType, animletter[anim]);

			byte *celBuf;
			{
				auto celData = LoadFileInMem(strBuff);
				celBuf = celData.get();
				LevelMonsterTypes[monst].Anims[anim].CMem = std::move(celData);
			}

			if (LevelMonsterTypes[monst].mtype != MT_GOLEM || (animletter[anim] != 's' && animletter[anim] != 'd')) {
				for (int i = 0; i < 8; i++) {
					byte *pCelStart = CelGetFrame(celBuf, i);
					LevelMonsterTypes[monst].Anims[anim].CelSpritesForDirections[i].emplace(pCelStart, width);
				}
			} else {
				for (int i = 0; i < 8; i++) {
					LevelMonsterTypes[monst].Anims[anim].CelSpritesForDirections[i].emplace(celBuf, width);
				}
			}
		}

		LevelMonsterTypes[monst].Anims[anim].Frames = frames;
		LevelMonsterTypes[monst].Anims[anim].Rate = MonsterData[mtype].Rate[anim];
	}

	LevelMonsterTypes[monst].mMinHP = MonsterData[mtype].mMinHP;
	LevelMonsterTypes[monst].mMaxHP = MonsterData[mtype].mMaxHP;
	if (!gbIsHellfire && mtype == MT_DIABLO) {
		LevelMonsterTypes[monst].mMinHP -= 2000;
		LevelMonsterTypes[monst].mMaxHP -= 2000;
	}
	LevelMonsterTypes[monst].mAFNum = MonsterData[mtype].mAFNum;
	LevelMonsterTypes[monst].MData = &MonsterData[mtype];

	if (MonsterData[mtype].has_trans) {
		InitMonsterTRN(LevelMonsterTypes[monst]);
	}

	if (mtype >= MT_NMAGMA && mtype <= MT_WMAGMA && (MissileFileFlag & 1) == 0) {
		MissileFileFlag |= 1;
		LoadMissileGFX(MFILE_MAGBALL);
	}
	if (mtype >= MT_STORM && mtype <= MT_MAEL && (MissileFileFlag & 2) == 0) {
		MissileFileFlag |= 2;
		LoadMissileGFX(MFILE_THINLGHT);
	}
	if (mtype == MT_SUCCUBUS && (MissileFileFlag & 4) == 0) {
		MissileFileFlag |= 4;
		LoadMissileGFX(MFILE_FLARE);
		LoadMissileGFX(MFILE_FLAREEXP);
	}
	if (mtype >= MT_INCIN && mtype <= MT_HELLBURN && (MissileFileFlag & 8) == 0) {
		MissileFileFlag |= 8;
		LoadMissileGFX(MFILE_KRULL);
	}
	if (mtype == MT_SNOWWICH && (MissileFileFlag & 0x20) == 0) {
		MissileFileFlag |= 0x20;
		LoadMissileGFX(MFILE_SCUBMISB);
		LoadMissileGFX(MFILE_SCBSEXPB);
	}
	if (mtype == MT_HLSPWN && (MissileFileFlag & 0x40) == 0) {
		MissileFileFlag |= 0x40;
		LoadMissileGFX(MFILE_SCUBMISD);
		LoadMissileGFX(MFILE_SCBSEXPD);
	}
	if (mtype == MT_SOLBRNR && (MissileFileFlag & 0x80) == 0) {
		MissileFileFlag |= 0x80;
		LoadMissileGFX(MFILE_SCUBMISC);
		LoadMissileGFX(MFILE_SCBSEXPC);
	}
	if (mtype >= MT_INCIN && mtype <= MT_HELLBURN && (MissileFileFlag & 8) == 0) {
		MissileFileFlag |= 8;
		LoadMissileGFX(MFILE_KRULL);
	}
	if (((mtype >= MT_NACID && mtype <= MT_XACID) || mtype == MT_SPIDLORD) && (MissileFileFlag & 0x10) == 0) {
		MissileFileFlag |= 0x10;
		LoadMissileGFX(MFILE_ACIDBF);
		LoadMissileGFX(MFILE_ACIDSPLA);
		LoadMissileGFX(MFILE_ACIDPUD);
	}
	if (mtype == MT_LICH && (MissileFileFlag & 0x100) == 0) {
		MissileFileFlag |= 0x100;
		LoadMissileGFX(MFILE_LICH);
		LoadMissileGFX(MFILE_EXORA1);
	}
	if (mtype == MT_ARCHLICH && (MissileFileFlag & 0x200) == 0) {
		MissileFileFlag |= 0x200;
		LoadMissileGFX(MFILE_ARCHLICH);
		LoadMissileGFX(MFILE_EXYEL2);
	}
	if ((mtype == MT_PSYCHORB || mtype == MT_BONEDEMN) && (MissileFileFlag & 0x400) == 0) {
		MissileFileFlag |= 0x400;
		LoadMissileGFX(MFILE_BONEDEMON);
	}
	if (mtype == MT_NECRMORB && (MissileFileFlag & 0x800) == 0) {
		MissileFileFlag |= 0x800;
		LoadMissileGFX(MFILE_NECROMORB);
		LoadMissileGFX(MFILE_EXRED3);
	}
	if (mtype == MT_PSYCHORB && (MissileFileFlag & 0x1000) == 0) {
		MissileFileFlag |= 0x1000;
		LoadMissileGFX(MFILE_EXBL2);
	}
	if (mtype == MT_BONEDEMN && (MissileFileFlag & 0x2000) == 0) {
		MissileFileFlag |= 0x2000;
		LoadMissileGFX(MFILE_EXBL3);
	}
	if (mtype == MT_DIABLO) {
		LoadMissileGFX(MFILE_FIREPLAR);
	}
}

void ClearMVars(int i)
{
	Monsters[i]._mVar1 = 0;
	Monsters[i]._mVar2 = 0;
	Monsters[i]._mVar3 = 0;
	Monsters[i].position.temp = { 0, 0 };
	Monsters[i].position.offset2 = { 0, 0 };
}

void InitMonster(int i, Direction rd, int mtype, Point position)
{
	CMonster *monst = &LevelMonsterTypes[mtype];

	Monsters[i]._mdir = rd;
	Monsters[i].position.tile = position;
	Monsters[i].position.future = position;
	Monsters[i].position.old = position;
	Monsters[i]._mMTidx = mtype;
	Monsters[i]._mmode = MM_STAND;
	Monsters[i].mName = _(monst->MData->mName);
	Monsters[i].MType = monst;
	Monsters[i].MData = monst->MData;
	Monsters[i].AnimInfo = {};
	Monsters[i].AnimInfo.pCelSprite = monst->Anims[MA_STAND].CelSpritesForDirections[rd] ? &*monst->Anims[MA_STAND].CelSpritesForDirections[rd] : nullptr;
	Monsters[i].AnimInfo.TicksPerFrame = monst->Anims[MA_STAND].Rate;
	Monsters[i].AnimInfo.TickCounterOfCurrentFrame = GenerateRnd(Monsters[i].AnimInfo.TicksPerFrame - 1);
	Monsters[i].AnimInfo.NumberOfFrames = monst->Anims[MA_STAND].Frames;
	Monsters[i].AnimInfo.CurrentFrame = GenerateRnd(Monsters[i].AnimInfo.NumberOfFrames - 1) + 1;

	Monsters[i].mLevel = monst->MData->mLevel;
	Monsters[i]._mmaxhp = (monst->mMinHP + GenerateRnd(monst->mMaxHP - monst->mMinHP + 1)) << 6;
	if (monst->mtype == MT_DIABLO && !gbIsHellfire) {
		Monsters[i]._mmaxhp /= 2;
		Monsters[i].mLevel -= 15;
	}

	if (!gbIsMultiplayer) {
		Monsters[i]._mmaxhp /= 2;
		if (Monsters[i]._mmaxhp < 64) {
			Monsters[i]._mmaxhp = 64;
		}
	}

	Monsters[i]._mhitpoints = Monsters[i]._mmaxhp;
	Monsters[i]._mAi = monst->MData->mAi;
	Monsters[i]._mint = monst->MData->mInt;
	Monsters[i]._mgoal = MGOAL_NORMAL;
	Monsters[i]._mgoalvar1 = 0;
	Monsters[i]._mgoalvar2 = 0;
	Monsters[i]._mgoalvar3 = 0;
	Monsters[i]._pathcount = 0;
	Monsters[i]._mDelFlag = false;
	Monsters[i]._uniqtype = 0;
	Monsters[i]._msquelch = 0;
	Monsters[i].mlid = NO_LIGHT; // BUGFIX monsters initial light id should be -1 (fixed)
	Monsters[i]._mRndSeed = AdvanceRndSeed();
	Monsters[i]._mAISeed = AdvanceRndSeed();
	Monsters[i].mWhoHit = 0;
	Monsters[i].mExp = monst->MData->mExp;
	Monsters[i].mHit = monst->MData->mHit;
	Monsters[i].mMinDamage = monst->MData->mMinDamage;
	Monsters[i].mMaxDamage = monst->MData->mMaxDamage;
	Monsters[i].mHit2 = monst->MData->mHit2;
	Monsters[i].mMinDamage2 = monst->MData->mMinDamage2;
	Monsters[i].mMaxDamage2 = monst->MData->mMaxDamage2;
	Monsters[i].mArmorClass = monst->MData->mArmorClass;
	Monsters[i].mMagicRes = monst->MData->mMagicRes;
	Monsters[i].leader = 0;
	Monsters[i].leaderflag = 0;
	Monsters[i]._mFlags = monst->MData->mFlags;
	Monsters[i].mtalkmsg = TEXT_NONE;

	if (Monsters[i]._mAi == AI_GARG) {
		Monsters[i].AnimInfo.pCelSprite = &*monst->Anims[MA_SPECIAL].CelSpritesForDirections[rd];
		Monsters[i].AnimInfo.CurrentFrame = 1;
		Monsters[i]._mFlags |= MFLAG_ALLOW_SPECIAL;
		Monsters[i]._mmode = MM_SATTACK;
	}

	if (sgGameInitInfo.nDifficulty == DIFF_NIGHTMARE) {
		Monsters[i]._mmaxhp = 3 * Monsters[i]._mmaxhp;
		if (gbIsHellfire)
			Monsters[i]._mmaxhp += (gbIsMultiplayer ? 100 : 50) << 6;
		else
			Monsters[i]._mmaxhp += 64;
		Monsters[i]._mhitpoints = Monsters[i]._mmaxhp;
		Monsters[i].mLevel += 15;
		Monsters[i].mExp = 2 * (Monsters[i].mExp + 1000);
		Monsters[i].mHit += NIGHTMARE_TO_HIT_BONUS;
		Monsters[i].mMinDamage = 2 * (Monsters[i].mMinDamage + 2);
		Monsters[i].mMaxDamage = 2 * (Monsters[i].mMaxDamage + 2);
		Monsters[i].mHit2 += NIGHTMARE_TO_HIT_BONUS;
		Monsters[i].mMinDamage2 = 2 * (Monsters[i].mMinDamage2 + 2);
		Monsters[i].mMaxDamage2 = 2 * (Monsters[i].mMaxDamage2 + 2);
		Monsters[i].mArmorClass += NIGHTMARE_AC_BONUS;
	} else if (sgGameInitInfo.nDifficulty == DIFF_HELL) {
		Monsters[i]._mmaxhp = 4 * Monsters[i]._mmaxhp;
		if (gbIsHellfire)
			Monsters[i]._mmaxhp += (gbIsMultiplayer ? 200 : 100) << 6;
		else
			Monsters[i]._mmaxhp += 192;
		Monsters[i]._mhitpoints = Monsters[i]._mmaxhp;
		Monsters[i].mLevel += 30;
		Monsters[i].mExp = 4 * (Monsters[i].mExp + 1000);
		Monsters[i].mHit += HELL_TO_HIT_BONUS;
		Monsters[i].mMinDamage = 4 * Monsters[i].mMinDamage + 6;
		Monsters[i].mMaxDamage = 4 * Monsters[i].mMaxDamage + 6;
		Monsters[i].mHit2 += HELL_TO_HIT_BONUS;
		Monsters[i].mMinDamage2 = 4 * Monsters[i].mMinDamage2 + 6;
		Monsters[i].mMaxDamage2 = 4 * Monsters[i].mMaxDamage2 + 6;
		Monsters[i].mArmorClass += HELL_AC_BONUS;
		Monsters[i].mMagicRes = monst->MData->mMagicRes2;
	}
}

void ClrAllMonsters()
{
	for (int i = 0; i < MAXMONSTERS; i++) {
		MonsterStruct *monst = &Monsters[i];
		ClearMVars(i);
		monst->mName = "Invalid Monster";
		monst->_mgoal = MGOAL_NONE;
		monst->_mmode = MM_STAND;
		monst->_mVar1 = 0;
		monst->_mVar2 = 0;
		monst->position.tile = { 0, 0 };
		monst->position.future = { 0, 0 };
		monst->position.old = { 0, 0 };
		monst->_mdir = static_cast<Direction>(GenerateRnd(8));
		monst->position.velocity = { 0, 0 };
		monst->AnimInfo = {};
		monst->_mFlags = 0;
		monst->_mDelFlag = false;
		monst->_menemy = GenerateRnd(gbActivePlayers);
		monst->enemyPosition = plr[monst->_menemy].position.future;
	}
}

bool MonstPlace(int xp, int yp)
{
	char f;

	if (xp < 0 || xp >= MAXDUNX
	    || yp < 0 || yp >= MAXDUNY
	    || dMonster[xp][yp] != 0
	    || dPlayer[xp][yp] != 0) {
		return false;
	}

	f = dFlags[xp][yp];

	if ((f & BFLAG_VISIBLE) != 0) {
		return false;
	}

	if ((f & BFLAG_POPULATED) != 0) {
		return false;
	}

	return !SolidLoc({ xp, yp });
}

void monster_some_crypt()
{
	MonsterStruct *mon;
	int hp;

	if (currlevel == 24 && UberDiabloMonsterIndex >= 0 && UberDiabloMonsterIndex < ActiveMonsterCount) {
		mon = &Monsters[UberDiabloMonsterIndex];
		PlayEffect(UberDiabloMonsterIndex, 2);
		quests[Q_NAKRUL]._qlog = false;
		mon->mArmorClass -= 50;
		hp = mon->_mmaxhp / 2;
		mon->mMagicRes = 0;
		mon->_mhitpoints = hp;
		mon->_mmaxhp = hp;
	}
}

void PlaceMonster(int i, int mtype, int x, int y)
{
	if (LevelMonsterTypes[mtype].mtype == MT_NAKRUL) {
		for (int j = 0; j < ActiveMonsterCount; j++) {
			if (Monsters[j]._mMTidx == mtype) {
				return;
			}
			if (Monsters[j].MType->mtype == MT_NAKRUL) {
				return;
			}
		}
	}
	dMonster[x][y] = i + 1;

	auto rd = static_cast<Direction>(GenerateRnd(8));
	InitMonster(i, rd, mtype, { x, y });
}

void PlaceUniqueMonst(int uniqindex, int miniontype, int bosspacksize)
{
	MonsterStruct *monst = &Monsters[ActiveMonsterCount];
	const UniqMonstStruct *uniq = &UniqMonst[uniqindex];

	if ((uniquetrans + 19) * 256 >= LIGHTSIZE) {
		return;
	}

	int uniqtype;
	for (uniqtype = 0; uniqtype < LevelMonsterTypeCount; uniqtype++) {
		if (LevelMonsterTypes[uniqtype].mtype == UniqMonst[uniqindex].mtype) {
			break;
		}
	}

	int count = 0;
	int xp;
	int yp;
	while (true) {
		xp = GenerateRnd(80) + 16;
		yp = GenerateRnd(80) + 16;
		int count2 = 0;
		for (int x = xp - 3; x < xp + 3; x++) {
			for (int y = yp - 3; y < yp + 3; y++) {
				if (y >= 0 && y < MAXDUNY && x >= 0 && x < MAXDUNX && MonstPlace(x, y)) {
					count2++;
				}
			}
		}

		if (count2 < 9) {
			count++;
			if (count < 1000) {
				continue;
			}
		}

		if (MonstPlace(xp, yp)) {
			break;
		}
	}

	if (uniqindex == UMT_SNOTSPIL) {
		xp = 2 * setpc_x + 24;
		yp = 2 * setpc_y + 28;
	}
	if (uniqindex == UMT_WARLORD) {
		xp = 2 * setpc_x + 22;
		yp = 2 * setpc_y + 23;
	}
	if (uniqindex == UMT_ZHAR) {
		for (int i = 0; i < themeCount; i++) {
			if (i == zharlib) {
				xp = 2 * themeLoc[i].x + 20;
				yp = 2 * themeLoc[i].y + 20;
				break;
			}
		}
	}
	if (!gbIsMultiplayer) {
		if (uniqindex == UMT_LAZURUS) {
			xp = 32;
			yp = 46;
		}
		if (uniqindex == UMT_RED_VEX) {
			xp = 40;
			yp = 45;
		}
		if (uniqindex == UMT_BLACKJADE) {
			xp = 38;
			yp = 49;
		}
		if (uniqindex == UMT_SKELKING) {
			xp = 35;
			yp = 47;
		}
	} else {
		if (uniqindex == UMT_LAZURUS) {
			xp = 2 * setpc_x + 19;
			yp = 2 * setpc_y + 22;
		}
		if (uniqindex == UMT_RED_VEX) {
			xp = 2 * setpc_x + 21;
			yp = 2 * setpc_y + 19;
		}
		if (uniqindex == UMT_BLACKJADE) {
			xp = 2 * setpc_x + 21;
			yp = 2 * setpc_y + 25;
		}
	}
	if (uniqindex == UMT_BUTCHER) {
		bool done = false;
		for (yp = 0; yp < MAXDUNY && !done; yp++) {
			for (xp = 0; xp < MAXDUNX && !done; xp++) {
				done = dPiece[xp][yp] == 367;
			}
		}
	}

	if (uniqindex == UMT_NAKRUL) {
		if (UberRow == 0 || UberCol == 0) {
			UberDiabloMonsterIndex = -1;
			return;
		}
		xp = UberRow - 2;
		yp = UberCol;
		UberDiabloMonsterIndex = ActiveMonsterCount;
	}
	PlaceMonster(ActiveMonsterCount, uniqtype, xp, yp);
	monst->_uniqtype = uniqindex + 1;

	if (uniq->mlevel != 0) {
		monst->mLevel = 2 * uniq->mlevel;
	} else {
		monst->mLevel += 5;
	}

	monst->mExp *= 2;
	monst->mName = _(uniq->mName);
	monst->_mmaxhp = uniq->mmaxhp << 6;

	if (!gbIsMultiplayer) {
		monst->_mmaxhp = monst->_mmaxhp / 2;
		if (monst->_mmaxhp < 64) {
			monst->_mmaxhp = 64;
		}
	}

	monst->_mhitpoints = monst->_mmaxhp;
	monst->_mAi = uniq->mAi;
	monst->_mint = uniq->mint;
	monst->mMinDamage = uniq->mMinDamage;
	monst->mMaxDamage = uniq->mMaxDamage;
	monst->mMinDamage2 = uniq->mMinDamage;
	monst->mMaxDamage2 = uniq->mMaxDamage;
	monst->mMagicRes = uniq->mMagicRes;
	monst->mtalkmsg = uniq->mtalkmsg;
	if (uniqindex == UMT_HORKDMN)
		monst->mlid = NO_LIGHT; // BUGFIX monsters initial light id should be -1 (fixed)
	else
		monst->mlid = AddLight(monst->position.tile, 3);

	if (gbIsMultiplayer) {
		if (monst->_mAi == AI_LAZHELP)
			monst->mtalkmsg = TEXT_NONE;
		if (monst->_mAi == AI_LAZURUS && quests[Q_BETRAYER]._qvar1 > 3) {
			monst->_mgoal = MGOAL_NORMAL;
		} else if (monst->mtalkmsg != TEXT_NONE) {
			monst->_mgoal = MGOAL_INQUIRING;
		}
	} else if (monst->mtalkmsg != TEXT_NONE) {
		monst->_mgoal = MGOAL_INQUIRING;
	}

	if (sgGameInitInfo.nDifficulty == DIFF_NIGHTMARE) {
		monst->_mmaxhp = 3 * monst->_mmaxhp;
		if (gbIsHellfire)
			monst->_mmaxhp += (gbIsMultiplayer ? 100 : 50) << 6;
		else
			monst->_mmaxhp += 64;
		monst->mLevel += 15;
		monst->_mhitpoints = monst->_mmaxhp;
		monst->mExp = 2 * (monst->mExp + 1000);
		monst->mMinDamage = 2 * (monst->mMinDamage + 2);
		monst->mMaxDamage = 2 * (monst->mMaxDamage + 2);
		monst->mMinDamage2 = 2 * (monst->mMinDamage2 + 2);
		monst->mMaxDamage2 = 2 * (monst->mMaxDamage2 + 2);
	} else if (sgGameInitInfo.nDifficulty == DIFF_HELL) {
		monst->_mmaxhp = 4 * monst->_mmaxhp;
		if (gbIsHellfire)
			monst->_mmaxhp += (gbIsMultiplayer ? 200 : 100) << 6;
		else
			monst->_mmaxhp += 192;
		monst->mLevel += 30;
		monst->_mhitpoints = monst->_mmaxhp;
		monst->mExp = 4 * (monst->mExp + 1000);
		monst->mMinDamage = 4 * monst->mMinDamage + 6;
		monst->mMaxDamage = 4 * monst->mMaxDamage + 6;
		monst->mMinDamage2 = 4 * monst->mMinDamage2 + 6;
		monst->mMaxDamage2 = 4 * monst->mMaxDamage2 + 6;
	}

	char filestr[64];
	sprintf(filestr, "Monsters\\Monsters\\%s.TRN", uniq->mTrnName);
	LoadFileInMem(filestr, &LightTables[256 * (uniquetrans + 19)], 256);

	monst->_uniqtrans = uniquetrans++;

	if ((uniq->mUnqAttr & 4) != 0) {
		monst->mHit = uniq->mUnqVar1;
		monst->mHit2 = uniq->mUnqVar1;

		if (sgGameInitInfo.nDifficulty == DIFF_NIGHTMARE) {
			monst->mHit += NIGHTMARE_TO_HIT_BONUS;
			monst->mHit2 += NIGHTMARE_TO_HIT_BONUS;
		} else if (sgGameInitInfo.nDifficulty == DIFF_HELL) {
			monst->mHit += HELL_TO_HIT_BONUS;
			monst->mHit2 += HELL_TO_HIT_BONUS;
		}
	}
	if ((uniq->mUnqAttr & 8) != 0) {
		monst->mArmorClass = uniq->mUnqVar1;

		if (sgGameInitInfo.nDifficulty == DIFF_NIGHTMARE) {
			monst->mArmorClass += NIGHTMARE_AC_BONUS;
		} else if (sgGameInitInfo.nDifficulty == DIFF_HELL) {
			monst->mArmorClass += HELL_AC_BONUS;
		}
	}

	ActiveMonsterCount++;

	if ((uniq->mUnqAttr & 1) != 0) {
		PlaceGroup(miniontype, bosspacksize, uniq->mUnqAttr, ActiveMonsterCount - 1);
	}

	if (monst->_mAi != AI_GARG) {
		monst->AnimInfo.pCelSprite = &*monst->MType->Anims[MA_STAND].CelSpritesForDirections[monst->_mdir];
		monst->AnimInfo.CurrentFrame = GenerateRnd(monst->AnimInfo.NumberOfFrames - 1) + 1;
		monst->_mFlags &= ~MFLAG_ALLOW_SPECIAL;
		monst->_mmode = MM_STAND;
	}
}

static void PlaceUniques()
{
	for (int u = 0; UniqMonst[u].mtype != -1; u++) {
		if (UniqMonst[u].mlevel != currlevel)
			continue;
		bool done = false;
		int mt;
		for (mt = 0; mt < LevelMonsterTypeCount; mt++) {
			done = (LevelMonsterTypes[mt].mtype == UniqMonst[u].mtype);
			if (done)
				break;
		}
		if (u == UMT_GARBUD && quests[Q_GARBUD]._qactive == QUEST_NOTAVAIL)
			done = false;
		if (u == UMT_ZHAR && quests[Q_ZHAR]._qactive == QUEST_NOTAVAIL)
			done = false;
		if (u == UMT_SNOTSPIL && quests[Q_LTBANNER]._qactive == QUEST_NOTAVAIL)
			done = false;
		if (u == UMT_LACHDAN && quests[Q_VEIL]._qactive == QUEST_NOTAVAIL)
			done = false;
		if (u == UMT_WARLORD && quests[Q_WARLORD]._qactive == QUEST_NOTAVAIL)
			done = false;
		if (done)
			PlaceUniqueMonst(u, mt, 8);
	}
}

void PlaceQuestMonsters()
{
	int skeltype;

	if (!setlevel) {
		if (QuestStatus(Q_BUTCHER)) {
			PlaceUniqueMonst(UMT_BUTCHER, 0, 0);
		}

		if (currlevel == quests[Q_SKELKING]._qlevel && gbIsMultiplayer) {
			skeltype = 0;

			for (skeltype = 0; skeltype < LevelMonsterTypeCount; skeltype++) {
				if (IsSkel(LevelMonsterTypes[skeltype].mtype)) {
					break;
				}
			}

			PlaceUniqueMonst(UMT_SKELKING, skeltype, 30);
		}

		if (QuestStatus(Q_LTBANNER)) {
			auto dunData = LoadFileInMem<uint16_t>("Levels\\L1Data\\Banner1.DUN");
			SetMapMonsters(dunData.get(), Point { setpc_x, setpc_y } * 2);
		}
		if (QuestStatus(Q_BLOOD)) {
			auto dunData = LoadFileInMem<uint16_t>("Levels\\L2Data\\Blood2.DUN");
			SetMapMonsters(dunData.get(), Point { setpc_x, setpc_y } * 2);
		}
		if (QuestStatus(Q_BLIND)) {
			auto dunData = LoadFileInMem<uint16_t>("Levels\\L2Data\\Blind2.DUN");
			SetMapMonsters(dunData.get(), Point { setpc_x, setpc_y } * 2);
		}
		if (QuestStatus(Q_ANVIL)) {
			auto dunData = LoadFileInMem<uint16_t>("Levels\\L3Data\\Anvil.DUN");
			SetMapMonsters(dunData.get(), Point { setpc_x + 2, setpc_y + 2 } * 2);
		}
		if (QuestStatus(Q_WARLORD)) {
			auto dunData = LoadFileInMem<uint16_t>("Levels\\L4Data\\Warlord.DUN");
			SetMapMonsters(dunData.get(), Point { setpc_x, setpc_y } * 2);
			AddMonsterType(UniqMonst[UMT_WARLORD].mtype, PLACE_SCATTER);
		}
		if (QuestStatus(Q_VEIL)) {
			AddMonsterType(UniqMonst[UMT_LACHDAN].mtype, PLACE_SCATTER);
		}
		if (QuestStatus(Q_ZHAR) && zharlib == -1) {
			quests[Q_ZHAR]._qactive = QUEST_NOTAVAIL;
		}

		if (currlevel == quests[Q_BETRAYER]._qlevel && gbIsMultiplayer) {
			AddMonsterType(UniqMonst[UMT_LAZURUS].mtype, PLACE_UNIQUE);
			AddMonsterType(UniqMonst[UMT_RED_VEX].mtype, PLACE_UNIQUE);
			PlaceUniqueMonst(UMT_LAZURUS, 0, 0);
			PlaceUniqueMonst(UMT_RED_VEX, 0, 0);
			PlaceUniqueMonst(UMT_BLACKJADE, 0, 0);
			auto dunData = LoadFileInMem<uint16_t>("Levels\\L4Data\\Vile1.DUN");
			SetMapMonsters(dunData.get(), Point { setpc_x, setpc_y } * 2);
		}

		if (currlevel == 24) {
			UberDiabloMonsterIndex = -1;
			int i1;
			for (i1 = 0; i1 < LevelMonsterTypeCount; i1++) {
				if (LevelMonsterTypes[i1].mtype == UniqMonst[UMT_NAKRUL].mtype)
					break;
			}

			if (i1 < LevelMonsterTypeCount) {
				for (int i2 = 0; i2 < ActiveMonsterCount; i2++) {
					if (Monsters[i2]._uniqtype != 0 || Monsters[i2]._mMTidx == i1) {
						UberDiabloMonsterIndex = i2;
						break;
					}
				}
			}
			if (UberDiabloMonsterIndex == -1)
				PlaceUniqueMonst(UMT_NAKRUL, 0, 0);
		}
	} else if (setlvlnum == SL_SKELKING) {
		PlaceUniqueMonst(UMT_SKELKING, 0, 0);
	}
}

void PlaceGroup(int mtype, int num, int leaderf, int leader)
{
	int placed = 0;

	for (int try1 = 0; try1 < 10; try1++) {
		while (placed != 0) {
			ActiveMonsterCount--;
			placed--;
			dMonster[Monsters[ActiveMonsterCount].position.tile.x][Monsters[ActiveMonsterCount].position.tile.y] = 0;
		}

		int xp;
		int yp;
		if ((leaderf & 1) != 0) {
			int offset = GenerateRnd(8);
			auto position = Monsters[leader].position.tile + static_cast<Direction>(offset);
			xp = position.x;
			yp = position.y;
		} else {
			do {
				xp = GenerateRnd(80) + 16;
				yp = GenerateRnd(80) + 16;
			} while (!MonstPlace(xp, yp));
		}
		int x1 = xp;
		int y1 = yp;

		if (num + ActiveMonsterCount > totalmonsters) {
			num = totalmonsters - ActiveMonsterCount;
		}

		int j = 0;
		for (int try2 = 0; j < num && try2 < 100; xp += Displacement::fromDirection(static_cast<Direction>(GenerateRnd(8))).deltaX, yp += Displacement::fromDirection(static_cast<Direction>(GenerateRnd(8))).deltaX) { /// BUGFIX: `yp += Point.y`
			if (!MonstPlace(xp, yp)
			    || (dTransVal[xp][yp] != dTransVal[x1][y1])
			    || ((leaderf & 2) != 0 && (abs(xp - x1) >= 4 || abs(yp - y1) >= 4))) {
				try2++;
				continue;
			}

			PlaceMonster(ActiveMonsterCount, mtype, xp, yp);
			if ((leaderf & 1) != 0) {
				Monsters[ActiveMonsterCount]._mmaxhp *= 2;
				Monsters[ActiveMonsterCount]._mhitpoints = Monsters[ActiveMonsterCount]._mmaxhp;
				Monsters[ActiveMonsterCount]._mint = Monsters[leader]._mint;

				if ((leaderf & 2) != 0) {
					Monsters[ActiveMonsterCount].leader = leader;
					Monsters[ActiveMonsterCount].leaderflag = 1;
					Monsters[ActiveMonsterCount]._mAi = Monsters[leader]._mAi;
				}

				if (Monsters[ActiveMonsterCount]._mAi != AI_GARG) {
					Monsters[ActiveMonsterCount].AnimInfo.pCelSprite = &*Monsters[ActiveMonsterCount].MType->Anims[MA_STAND].CelSpritesForDirections[Monsters[ActiveMonsterCount]._mdir];
					Monsters[ActiveMonsterCount].AnimInfo.CurrentFrame = GenerateRnd(Monsters[ActiveMonsterCount].AnimInfo.NumberOfFrames - 1) + 1;
					Monsters[ActiveMonsterCount]._mFlags &= ~MFLAG_ALLOW_SPECIAL;
					Monsters[ActiveMonsterCount]._mmode = MM_STAND;
				}
			}
			ActiveMonsterCount++;
			placed++;
			j++;
		}

		if (placed >= num) {
			break;
		}
	}

	if ((leaderf & 2) != 0) {
		Monsters[leader].packsize = placed;
	}
}

void LoadDiabMonsts()
{
	{
		auto dunData = LoadFileInMem<uint16_t>("Levels\\L4Data\\diab1.DUN");
		SetMapMonsters(dunData.get(), Point { diabquad1x, diabquad1y } * 2);
	}
	{
		auto dunData = LoadFileInMem<uint16_t>("Levels\\L4Data\\diab2a.DUN");
		SetMapMonsters(dunData.get(), Point { diabquad2x, diabquad2y } * 2);
	}
	{
		auto dunData = LoadFileInMem<uint16_t>("Levels\\L4Data\\diab3a.DUN");
		SetMapMonsters(dunData.get(), Point { diabquad3x, diabquad3y } * 2);
	}
	{
		auto dunData = LoadFileInMem<uint16_t>("Levels\\L4Data\\diab4a.DUN");
		SetMapMonsters(dunData.get(), Point { diabquad4x, diabquad4y } * 2);
	}
}

void InitMonsters()
{
#ifdef _DEBUG
	if (gbIsMultiplayer)
		CheckDungeonClear();
#endif
	if (!setlevel) {
		AddMonster({ 1, 0 }, DIR_S, 0, false);
		AddMonster({ 1, 0 }, DIR_S, 0, false);
		AddMonster({ 1, 0 }, DIR_S, 0, false);
		AddMonster({ 1, 0 }, DIR_S, 0, false);
	}

	if (!gbIsSpawn && !setlevel && currlevel == 16)
		LoadDiabMonsts();

	int nt = numtrigs;
	if (currlevel == 15)
		nt = 1;
	for (int i = 0; i < nt; i++) {
		for (int s = -2; s < 2; s++) {
			for (int t = -2; t < 2; t++)
				DoVision(trigs[i].position + Displacement { s, t }, 15, false, false);
		}
	}
	if (!gbIsSpawn)
		PlaceQuestMonsters();
	if (!setlevel) {
		if (!gbIsSpawn)
			PlaceUniques();
		int na = 0;
		for (int s = 16; s < 96; s++) {
			for (int t = 16; t < 96; t++) {
				if (!SolidLoc({ s, t }))
					na++;
			}
		}
		int numplacemonsters = na / 30;
		if (gbIsMultiplayer)
			numplacemonsters += numplacemonsters / 2;
		if (ActiveMonsterCount + numplacemonsters > MAXMONSTERS - 10)
			numplacemonsters = MAXMONSTERS - 10 - ActiveMonsterCount;
		totalmonsters = ActiveMonsterCount + numplacemonsters;
		int numscattypes = 0;
		int scattertypes[NUM_MTYPES];
		for (int i = 0; i < LevelMonsterTypeCount; i++) {
			if ((LevelMonsterTypes[i].mPlaceFlags & PLACE_SCATTER) != 0) {
				scattertypes[numscattypes] = i;
				numscattypes++;
			}
		}
		while (ActiveMonsterCount < totalmonsters) {
			int mtype = scattertypes[GenerateRnd(numscattypes)];
			if (currlevel == 1 || GenerateRnd(2) == 0)
				na = 1;
			else if (currlevel == 2 || (currlevel >= 21 && currlevel <= 24))
				na = GenerateRnd(2) + 2;
			else
				na = GenerateRnd(3) + 3;
			PlaceGroup(mtype, na, 0, 0);
		}
	}
	for (int i = 0; i < nt; i++) {
		for (int s = -2; s < 2; s++) {
			for (int t = -2; t < 2; t++)
				DoUnVision(trigs[i].position + Displacement { s, t }, 15);
		}
	}
}

void SetMapMonsters(const uint16_t *dunData, Point startPosition)
{
	AddMonsterType(MT_GOLEM, PLACE_SPECIAL);
	AddMonster({ 1, 0 }, DIR_S, 0, false);
	AddMonster({ 1, 0 }, DIR_S, 0, false);
	AddMonster({ 1, 0 }, DIR_S, 0, false);
	AddMonster({ 1, 0 }, DIR_S, 0, false);
	if (setlevel && setlvlnum == SL_VILEBETRAYER) {
		AddMonsterType(UniqMonst[UMT_LAZURUS].mtype, PLACE_UNIQUE);
		AddMonsterType(UniqMonst[UMT_RED_VEX].mtype, PLACE_UNIQUE);
		AddMonsterType(UniqMonst[UMT_BLACKJADE].mtype, PLACE_UNIQUE);
		PlaceUniqueMonst(UMT_LAZURUS, 0, 0);
		PlaceUniqueMonst(UMT_RED_VEX, 0, 0);
		PlaceUniqueMonst(UMT_BLACKJADE, 0, 0);
	}

	int width = SDL_SwapLE16(dunData[0]);
	int height = SDL_SwapLE16(dunData[1]);

	int layer2Offset = 2 + width * height;

	// The rest of the layers are at dPiece scale
	width *= 2;
	height *= 2;

	const uint16_t *monsterLayer = &dunData[layer2Offset + width * height];

	for (int j = 0; j < height; j++) {
		for (int i = 0; i < width; i++) {
			uint8_t monsterId = SDL_SwapLE16(monsterLayer[j * width + i]);
			if (monsterId != 0) {
				int mtype = AddMonsterType(MonstConvTbl[monsterId - 1], PLACE_SPECIAL);
				PlaceMonster(ActiveMonsterCount++, mtype, i + startPosition.x + 16, j + startPosition.y + 16);
			}
		}
	}
}

void DeleteMonster(int i)
{
	ActiveMonsterCount--;
	ActiveMonsters[i] = ActiveMonsters[ActiveMonsterCount];
}

int AddMonster(Point position, Direction dir, int mtype, bool inMap)
{
	if (ActiveMonsterCount < MAXMONSTERS) {
		int i = ActiveMonsters[ActiveMonsterCount++];
		if (inMap)
			dMonster[position.x][position.y] = i + 1;
		InitMonster(i, dir, mtype, position);
		return i;
	}

	return -1;
}

void AddDoppelganger(MonsterStruct &monster)
{
	if (monster.MType == nullptr) {
		return;
	}

	Point target = { 0, 0 };
	for (int d = 0; d < 8; d++) {
		const Point position = monster.position.tile + static_cast<Direction>(d);
		if (!SolidLoc(position)) {
			if (dPlayer[position.x][position.y] == 0 && dMonster[position.x][position.y] == 0) {
				if (dObject[position.x][position.y] == 0) {
					target = position;
					break;
				}
				int oi = dObject[position.x][position.y] > 0 ? dObject[position.x][position.y] - 1 : -(dObject[position.x][position.y] + 1);
				if (!object[oi]._oSolidFlag) {
					target = position;
					break;
				}
			}
		}
	}
	if (target != Point { 0, 0 }) {
		for (int j = 0; j < MAX_LVLMTYPES; j++) {
			if (LevelMonsterTypes[j].mtype == monster.MType->mtype) {
				AddMonster(target, monster._mdir, j, true);
				break;
			}
		}
	}
}

void NewMonsterAnim(int i, AnimStruct *anim, Direction md, AnimationDistributionFlags flags = AnimationDistributionFlags::None, int numSkippedFrames = 0, int distributeFramesBeforeFrame = 0)
{
	MonsterStruct *monst = &Monsters[i];
	auto *pCelSprite = &*anim->CelSpritesForDirections[md];
	monst->AnimInfo.SetNewAnimation(pCelSprite, anim->Frames, anim->Rate, flags, numSkippedFrames, distributeFramesBeforeFrame);
	monst->_mFlags &= ~(MFLAG_LOCK_ANIMATION | MFLAG_ALLOW_SPECIAL);
	monst->_mdir = md;
}

bool M_Ranged(int i)
{
	return IsAnyOf(Monsters[i]._mAi, AI_SKELBOW, AI_GOATBOW, AI_SUCC, AI_LAZHELP);
}

bool M_Talker(int i)
{
	return IsAnyOf(Monsters[i]._mAi, AI_LAZURUS, AI_WARLORD, AI_GARBUD, AI_ZHAR, AI_SNOTSPIL, AI_LACHDAN, AI_LAZHELP);
}

void M_Enemy(int i)
{
	BYTE enemyx;
	BYTE enemyy;

	int menemy = -1;
	int bestDist = -1;
	bool bestsameroom = false;
	MonsterStruct *monst = &Monsters[i];
	if ((monst->_mFlags & MFLAG_BERSERK) != 0 || (monst->_mFlags & MFLAG_GOLEM) == 0) {
		for (int pnum = 0; pnum < MAX_PLRS; pnum++) {
			if (!plr[pnum].plractive || currlevel != plr[pnum].plrlevel || plr[pnum]._pLvlChanging
			    || (((plr[pnum]._pHitPoints >> 6) == 0) && gbIsMultiplayer))
				continue;
			bool sameroom = (dTransVal[monst->position.tile.x][monst->position.tile.y] == dTransVal[plr[pnum].position.tile.x][plr[pnum].position.tile.y]);
			int dist = monst->position.tile.WalkingDistance(plr[pnum].position.tile);
			if ((sameroom && !bestsameroom)
			    || ((sameroom || !bestsameroom) && dist < bestDist)
			    || (menemy == -1)) {
				monst->_mFlags &= ~MFLAG_TARGETS_MONSTER;
				menemy = pnum;
				enemyx = plr[pnum].position.future.x;
				enemyy = plr[pnum].position.future.y;
				bestDist = dist;
				bestsameroom = sameroom;
			}
		}
	}
	for (int j = 0; j < ActiveMonsterCount; j++) {
		int mi = ActiveMonsters[j];
		if (mi == i)
			continue;
		if ((Monsters[mi]._mhitpoints >> 6) <= 0)
			continue;
		if (Monsters[mi].position.tile.x == 1 && Monsters[mi].position.tile.y == 0)
			continue;
		if (M_Talker(mi) && Monsters[mi].mtalkmsg != TEXT_NONE)
			continue;
		if ((monst->_mFlags & MFLAG_GOLEM) != 0 && (Monsters[mi]._mFlags & MFLAG_GOLEM) != 0) // prevent golems from fighting each other
			continue;

		int dist = Monsters[mi].position.tile.WalkingDistance(monst->position.tile);
		if (((monst->_mFlags & MFLAG_GOLEM) == 0
		        && (monst->_mFlags & MFLAG_BERSERK) == 0
		        && dist >= 2
		        && !M_Ranged(i))
		    || ((monst->_mFlags & MFLAG_GOLEM) == 0
		        && (monst->_mFlags & MFLAG_BERSERK) == 0
		        && (Monsters[mi]._mFlags & MFLAG_GOLEM) == 0)) {
			continue;
		}
		bool sameroom = dTransVal[monst->position.tile.x][monst->position.tile.y] == dTransVal[Monsters[mi].position.tile.x][Monsters[mi].position.tile.y];
		if ((sameroom && !bestsameroom)
		    || ((sameroom || !bestsameroom) && dist < bestDist)
		    || (menemy == -1)) {
			monst->_mFlags |= MFLAG_TARGETS_MONSTER;
			menemy = mi;
			enemyx = Monsters[mi].position.future.x;
			enemyy = Monsters[mi].position.future.y;
			bestDist = dist;
			bestsameroom = sameroom;
		}
	}
	if (menemy != -1) {
		monst->_mFlags &= ~MFLAG_NO_ENEMY;
		monst->_menemy = menemy;
		monst->enemyPosition = { enemyx, enemyy };
	} else {
		monst->_mFlags |= MFLAG_NO_ENEMY;
	}
}

Direction M_GetDir(int i)
{
	return GetDirection(Monsters[i].position.tile, Monsters[i].enemyPosition);
}

void M_StartStand(int i, Direction md)
{
	ClearMVars(i);
	if (Monsters[i].MType->mtype == MT_GOLEM)
		NewMonsterAnim(i, &Monsters[i].MType->Anims[MA_WALK], md);
	else
		NewMonsterAnim(i, &Monsters[i].MType->Anims[MA_STAND], md);
	Monsters[i]._mVar1 = Monsters[i]._mmode;
	Monsters[i]._mVar2 = 0;
	Monsters[i]._mmode = MM_STAND;
	Monsters[i].position.offset = { 0, 0 };
	Monsters[i].position.future = Monsters[i].position.tile;
	Monsters[i].position.old = Monsters[i].position.tile;
	Monsters[i]._mdir = md;
	M_Enemy(i);
}

void M_StartDelay(int i, int len)
{
	if (len <= 0) {
		return;
	}

	if (Monsters[i]._mAi != AI_LAZURUS) {
		Monsters[i]._mVar2 = len;
		Monsters[i]._mmode = MM_DELAY;
	}
}

void M_StartSpStand(int i, Direction md)
{
	NewMonsterAnim(i, &Monsters[i].MType->Anims[MA_SPECIAL], md);
	Monsters[i]._mmode = MM_SPSTAND;
	Monsters[i].position.offset = { 0, 0 };
	Monsters[i].position.future = Monsters[i].position.tile;
	Monsters[i].position.old = Monsters[i].position.tile;
	Monsters[i]._mdir = md;
}

void M_StartWalk(int i, int xvel, int yvel, int xadd, int yadd, Direction endDir)
{
	int fx = xadd + Monsters[i].position.tile.x;
	int fy = yadd + Monsters[i].position.tile.y;

	dMonster[fx][fy] = -(i + 1);
	Monsters[i]._mmode = MM_WALK;
	Monsters[i].position.old = Monsters[i].position.tile;
	Monsters[i].position.future = { fx, fy };
	Monsters[i].position.velocity = { xvel, yvel };
	Monsters[i]._mVar1 = xadd;
	Monsters[i]._mVar2 = yadd;
	Monsters[i]._mVar3 = endDir;
	Monsters[i]._mdir = endDir;
	NewMonsterAnim(i, &Monsters[i].MType->Anims[MA_WALK], endDir, AnimationDistributionFlags::ProcessAnimationPending, -1);
	Monsters[i].position.offset2 = { 0, 0 };
}

void M_StartWalk2(int i, int xvel, int yvel, int xoff, int yoff, int xadd, int yadd, Direction endDir)
{
	int fx = xadd + Monsters[i].position.tile.x;
	int fy = yadd + Monsters[i].position.tile.y;

	dMonster[Monsters[i].position.tile.x][Monsters[i].position.tile.y] = -(i + 1);
	Monsters[i]._mVar1 = Monsters[i].position.tile.x;
	Monsters[i]._mVar2 = Monsters[i].position.tile.y;
	Monsters[i].position.old = Monsters[i].position.tile;
	Monsters[i].position.tile = { fx, fy };
	Monsters[i].position.future = { fx, fy };
	dMonster[fx][fy] = i + 1;
	if (Monsters[i].mlid != NO_LIGHT)
		ChangeLightXY(Monsters[i].mlid, Monsters[i].position.tile);
	Monsters[i].position.offset = { xoff, yoff };
	Monsters[i]._mmode = MM_WALK2;
	Monsters[i].position.velocity = { xvel, yvel };
	Monsters[i]._mVar3 = endDir;
	Monsters[i]._mdir = endDir;
	NewMonsterAnim(i, &Monsters[i].MType->Anims[MA_WALK], endDir, AnimationDistributionFlags::ProcessAnimationPending, -1);
	Monsters[i].position.offset2 = { 16 * xoff, 16 * yoff };
}

void M_StartWalk3(int i, int xvel, int yvel, int xoff, int yoff, int xadd, int yadd, int mapx, int mapy, Direction endDir)
{
	int fx = xadd + Monsters[i].position.tile.x;
	int fy = yadd + Monsters[i].position.tile.y;
	int x = mapx + Monsters[i].position.tile.x;
	int y = mapy + Monsters[i].position.tile.y;

	if (Monsters[i].mlid != NO_LIGHT)
		ChangeLightXY(Monsters[i].mlid, { x, y });

	dMonster[Monsters[i].position.tile.x][Monsters[i].position.tile.y] = -(i + 1);
	dMonster[fx][fy] = -(i + 1);
	Monsters[i].position.temp = { x, y };
	dFlags[x][y] |= BFLAG_MONSTLR;
	Monsters[i].position.old = Monsters[i].position.tile;
	Monsters[i].position.future = { fx, fy };
	Monsters[i].position.offset = { xoff, yoff };
	Monsters[i]._mmode = MM_WALK3;
	Monsters[i].position.velocity = { xvel, yvel };
	Monsters[i]._mVar1 = fx;
	Monsters[i]._mVar2 = fy;
	Monsters[i]._mVar3 = endDir;
	Monsters[i]._mdir = endDir;
	NewMonsterAnim(i, &Monsters[i].MType->Anims[MA_WALK], endDir, AnimationDistributionFlags::ProcessAnimationPending, -1);
	Monsters[i].position.offset2 = { 16 * xoff, 16 * yoff };
}

void M_StartAttack(int i)
{
	Direction md = M_GetDir(i);
	NewMonsterAnim(i, &Monsters[i].MType->Anims[MA_ATTACK], md, AnimationDistributionFlags::ProcessAnimationPending);
	Monsters[i]._mmode = MM_ATTACK;
	Monsters[i].position.offset = { 0, 0 };
	Monsters[i].position.future = Monsters[i].position.tile;
	Monsters[i].position.old = Monsters[i].position.tile;
	Monsters[i]._mdir = md;
}

void M_StartRAttack(int i, missile_id missileType, int dam)
{
	Direction md = M_GetDir(i);
	NewMonsterAnim(i, &Monsters[i].MType->Anims[MA_ATTACK], md, AnimationDistributionFlags::ProcessAnimationPending);
	Monsters[i]._mmode = MM_RATTACK;
	Monsters[i]._mVar1 = missileType;
	Monsters[i]._mVar2 = dam;
	Monsters[i].position.offset = { 0, 0 };
	Monsters[i].position.future = Monsters[i].position.tile;
	Monsters[i].position.old = Monsters[i].position.tile;
	Monsters[i]._mdir = md;
}

void M_StartRSpAttack(int i, missile_id missileType, int dam)
{
	Direction md = M_GetDir(i);
	int distributeFramesBeforeFrame = 0;
	if (Monsters[i]._mAi == AI_MEGA)
		distributeFramesBeforeFrame = Monsters[i].MData->mAFNum2;
	NewMonsterAnim(i, &Monsters[i].MType->Anims[MA_SPECIAL], md, AnimationDistributionFlags::ProcessAnimationPending, 0, distributeFramesBeforeFrame);
	Monsters[i]._mmode = MM_RSPATTACK;
	Monsters[i]._mVar1 = missileType;
	Monsters[i]._mVar2 = 0;
	Monsters[i]._mVar3 = dam;
	Monsters[i].position.offset = { 0, 0 };
	Monsters[i].position.future = Monsters[i].position.tile;
	Monsters[i].position.old = Monsters[i].position.tile;
	Monsters[i]._mdir = md;
}

void M_StartSpAttack(int i)
{
	Direction md = M_GetDir(i);
	NewMonsterAnim(i, &Monsters[i].MType->Anims[MA_SPECIAL], md);
	Monsters[i]._mmode = MM_SATTACK;
	Monsters[i].position.offset = { 0, 0 };
	Monsters[i].position.future = Monsters[i].position.tile;
	Monsters[i].position.old = Monsters[i].position.tile;
	Monsters[i]._mdir = md;
}

void M_StartEat(int i)
{
	NewMonsterAnim(i, &Monsters[i].MType->Anims[MA_SPECIAL], Monsters[i]._mdir);
	Monsters[i]._mmode = MM_SATTACK;
	Monsters[i].position.offset = { 0, 0 };
	Monsters[i].position.future = Monsters[i].position.tile;
	Monsters[i].position.old = Monsters[i].position.tile;
}

void M_ClearSquares(int i)
{
	int mx = Monsters[i].position.old.x;
	int my = Monsters[i].position.old.y;
	int m1 = -(i + 1);
	int m2 = i + 1;

	for (int y = my - 1; y <= my + 1; y++) {
		if (y >= 0 && y < MAXDUNY) {
			for (int x = mx - 1; x <= mx + 1; x++) {
				if (x >= 0 && x < MAXDUNX && (dMonster[x][y] == m1 || dMonster[x][y] == m2))
					dMonster[x][y] = 0;
			}
		}
	}

	if (mx + 1 < MAXDUNX)
		dFlags[mx + 1][my] &= ~BFLAG_MONSTLR;
	if (my + 1 < MAXDUNY)
		dFlags[mx][my + 1] &= ~BFLAG_MONSTLR;
}

void M_GetKnockback(int i)
{
	Direction d = opposite[Monsters[i]._mdir];
	if (!DirOK(i, d)) {
		return;
	}

	M_ClearSquares(i);
	Monsters[i].position.old += d;
	NewMonsterAnim(i, &Monsters[i].MType->Anims[MA_GOTHIT], Monsters[i]._mdir, gGameLogicStep < GameLogicStep::ProcessMonsters ? AnimationDistributionFlags::ProcessAnimationPending : AnimationDistributionFlags::None);
	Monsters[i]._mmode = MM_GOTHIT;
	Monsters[i].position.offset = { 0, 0 };
	Monsters[i].position.tile = Monsters[i].position.old;
	Monsters[i].position.future = Monsters[i].position.tile;
	M_ClearSquares(i);
	dMonster[Monsters[i].position.tile.x][Monsters[i].position.tile.y] = i + 1;
}

void M_StartHit(int i, int pnum, int dam)
{
	if (pnum >= 0)
		Monsters[i].mWhoHit |= 1 << pnum;
	if (pnum == myplr) {
		delta_monster_hp(i, Monsters[i]._mhitpoints, currlevel);
		NetSendCmdMonDmg(false, i, dam);
	}
	PlayEffect(i, 1);
	if ((Monsters[i].MType->mtype >= MT_SNEAK && Monsters[i].MType->mtype <= MT_ILLWEAV) || dam >> 6 >= Monsters[i].mLevel + 3) {
		if (pnum >= 0) {
			Monsters[i]._menemy = pnum;
			Monsters[i].enemyPosition = plr[pnum].position.future;
			Monsters[i]._mFlags &= ~MFLAG_TARGETS_MONSTER;
			Monsters[i]._mdir = M_GetDir(i);
		}
		if (Monsters[i].MType->mtype == MT_BLINK) {
			M_Teleport(i);
		} else if ((Monsters[i].MType->mtype >= MT_NSCAV && Monsters[i].MType->mtype <= MT_YSCAV)
		    || Monsters[i].MType->mtype == MT_GRAVEDIG) {
			Monsters[i]._mgoal = MGOAL_NORMAL;
			Monsters[i]._mgoalvar1 = 0;
			Monsters[i]._mgoalvar2 = 0;
		}
		if (Monsters[i]._mmode != MM_STONE) {
			NewMonsterAnim(i, &Monsters[i].MType->Anims[MA_GOTHIT], Monsters[i]._mdir, gGameLogicStep < GameLogicStep::ProcessMonsters ? AnimationDistributionFlags::ProcessAnimationPending : AnimationDistributionFlags::None);
			Monsters[i]._mmode = MM_GOTHIT;
			Monsters[i].position.offset = { 0, 0 };
			Monsters[i].position.tile = Monsters[i].position.old;
			Monsters[i].position.future = Monsters[i].position.old;
			M_ClearSquares(i);
			dMonster[Monsters[i].position.tile.x][Monsters[i].position.tile.y] = i + 1;
		}
	}
}

void M_DiabloDeath(int i, bool sendmsg)
{
	MonsterStruct *monst = &Monsters[i];
	PlaySFX(USFX_DIABLOD);
	quests[Q_DIABLO]._qactive = QUEST_DONE;
	if (sendmsg)
		NetSendCmdQuest(true, Q_DIABLO);
	sgbSaveSoundOn = gbSoundOn;
	gbProcessPlayers = false;
	for (int j = 0; j < ActiveMonsterCount; j++) {
		int k = ActiveMonsters[j];
		if (k == i || monst->_msquelch == 0)
			continue;

		NewMonsterAnim(k, &Monsters[k].MType->Anims[MA_DEATH], Monsters[k]._mdir);
		Monsters[k]._mmode = MM_DEATH;
		Monsters[k].position.offset = { 0, 0 };
		Monsters[k]._mVar1 = 0;
		Monsters[k].position.tile = Monsters[k].position.old;
		Monsters[k].position.future = Monsters[k].position.tile;
		M_ClearSquares(k);
		dMonster[Monsters[k].position.tile.x][Monsters[k].position.tile.y] = k + 1;
	}
	AddLight(monst->position.tile, 8);
	DoVision(monst->position.tile, 8, false, true);
	int dist = monst->position.tile.WalkingDistance({ ViewX, ViewY });
	if (dist > 20)
		dist = 20;
	monst->_mVar3 = ViewX << 16;
	monst->position.temp.x = ViewY << 16;
	monst->position.temp.y = (int)((monst->_mVar3 - (monst->position.tile.x << 16)) / (double)dist);
	monst->position.offset2.deltaX = (int)((monst->position.temp.x - (monst->position.tile.y << 16)) / (double)dist);
}

void SpawnLoot(int i, bool sendmsg)
{
	MonsterStruct *monst = &Monsters[i];
	if (QuestStatus(Q_GARBUD) && monst->_uniqtype - 1 == UMT_GARBUD) {
		CreateTypeItem(monst->position.tile + Displacement { 1, 1 }, true, ITYPE_MACE, IMISC_NONE, true, false);
	} else if (monst->_uniqtype - 1 == UMT_DEFILER) {
		if (effect_is_playing(USFX_DEFILER8))
			stream_stop();
		quests[Q_DEFILER]._qlog = false;
		SpawnMapOfDoom(monst->position.tile);
	} else if (monst->_uniqtype - 1 == UMT_HORKDMN) {
		if (sgGameInitInfo.bTheoQuest != 0) {
			SpawnTheodore(monst->position.tile);
		} else {
			CreateAmulet(monst->position.tile, 13, false, true);
		}
	} else if (monst->MType->mtype == MT_HORKSPWN) {
	} else if (monst->MType->mtype == MT_NAKRUL) {
		int nSFX = IsUberRoomOpened ? USFX_NAKRUL4 : USFX_NAKRUL5;
		if (sgGameInitInfo.bCowQuest != 0)
			nSFX = USFX_NAKRUL6;
		if (effect_is_playing(nSFX))
			stream_stop();
		quests[Q_NAKRUL]._qlog = false;
		UberDiabloMonsterIndex = -2;
		CreateMagicWeapon(monst->position.tile, ITYPE_SWORD, ICURS_GREAT_SWORD, false, true);
		CreateMagicWeapon(monst->position.tile, ITYPE_STAFF, ICURS_WAR_STAFF, false, true);
		CreateMagicWeapon(monst->position.tile, ITYPE_BOW, ICURS_LONG_WAR_BOW, false, true);
		CreateSpellBook(monst->position.tile, SPL_APOCA, false, true);
	} else if (i > MAX_PLRS - 1) { // Golems should not spawn loot
		SpawnItem(i, monst->position.tile, sendmsg);
	}
}

void M2MStartHit(int mid, int i, int dam)
{
	assurance((DWORD)mid < MAXMONSTERS, mid);
	assurance(Monsters[mid].MType != nullptr, mid);

	if (i >= 0 && i < MAX_PLRS)
		Monsters[mid].mWhoHit |= 1 << i;

	delta_monster_hp(mid, Monsters[mid]._mhitpoints, currlevel);
	NetSendCmdMonDmg(false, mid, dam);
	PlayEffect(mid, 1);

	if ((Monsters[mid].MType->mtype >= MT_SNEAK && Monsters[mid].MType->mtype <= MT_ILLWEAV) || dam >> 6 >= Monsters[mid].mLevel + 3) {
		if (i >= 0)
			Monsters[mid]._mdir = opposite[Monsters[i]._mdir];

		if (Monsters[mid].MType->mtype == MT_BLINK) {
			M_Teleport(mid);
		} else if ((Monsters[mid].MType->mtype >= MT_NSCAV && Monsters[mid].MType->mtype <= MT_YSCAV)
		    || Monsters[mid].MType->mtype == MT_GRAVEDIG) {
			Monsters[mid]._mgoal = MGOAL_NORMAL;
			Monsters[mid]._mgoalvar1 = 0;
			Monsters[mid]._mgoalvar2 = 0;
		}

		if (Monsters[mid]._mmode != MM_STONE) {
			if (Monsters[mid].MType->mtype != MT_GOLEM) {
				NewMonsterAnim(mid, &Monsters[mid].MType->Anims[MA_GOTHIT], Monsters[mid]._mdir, gGameLogicStep < GameLogicStep::ProcessMonsters ? AnimationDistributionFlags::ProcessAnimationPending : AnimationDistributionFlags::None);
				Monsters[mid]._mmode = MM_GOTHIT;
			}

			Monsters[mid].position.offset = { 0, 0 };
			Monsters[mid].position.tile = Monsters[mid].position.old;
			Monsters[mid].position.future = Monsters[mid].position.old;
			M_ClearSquares(mid);
			dMonster[Monsters[mid].position.tile.x][Monsters[mid].position.tile.y] = mid + 1;
		}
	}
}

void MonstStartKill(int i, int pnum, bool sendmsg)
{
	assurance((DWORD)i < MAXMONSTERS, i);

	MonsterStruct *monst = &Monsters[i];
	assurance(monst->MType != nullptr, i);

	if (pnum >= 0)
		monst->mWhoHit |= 1 << pnum;
	if (pnum < MAX_PLRS && i >= MAX_PLRS) /// BUGFIX: i >= MAX_PLRS (fixed)
		AddPlrMonstExper(monst->mLevel, monst->mExp, monst->mWhoHit);
	MonsterKillCounts[monst->MType->mtype]++;
	monst->_mhitpoints = 0;
	SetRndSeed(monst->_mRndSeed);
	SpawnLoot(i, sendmsg);
	if (monst->MType->mtype == MT_DIABLO)
		M_DiabloDeath(i, true);
	else
		PlayEffect(i, 2);

	Direction md = pnum >= 0 ? M_GetDir(i) : monst->_mdir;
	monst->_mdir = md;
	NewMonsterAnim(i, &monst->MType->Anims[MA_DEATH], md, gGameLogicStep < GameLogicStep::ProcessMonsters ? AnimationDistributionFlags::ProcessAnimationPending : AnimationDistributionFlags::None);
	monst->_mmode = MM_DEATH;
	monst->_mgoal = MGOAL_NONE;
	monst->position.offset = { 0, 0 };
	monst->_mVar1 = 0;
	monst->position.tile = monst->position.old;
	monst->position.future = monst->position.old;
	M_ClearSquares(i);
	dMonster[monst->position.tile.x][monst->position.tile.y] = i + 1;
	CheckQuestKill(i, sendmsg);
	M_FallenFear(monst->position.tile);
	if ((monst->MType->mtype >= MT_NACID && monst->MType->mtype <= MT_XACID) || monst->MType->mtype == MT_SPIDLORD)
		AddMissile(monst->position.tile, { 0, 0 }, 0, MIS_ACIDPUD, TARGET_PLAYERS, i, monst->_mint + 1, 0);
}

void M2MStartKill(int i, int mid)
{
	assurance((DWORD)i < MAXMONSTERS, i);
	assurance((DWORD)mid < MAXMONSTERS, mid);
	assurance(Monsters[mid].MType != nullptr, mid); /// BUGFIX: should check `mid` (fixed)

	delta_kill_monster(mid, Monsters[mid].position.tile, currlevel);
	NetSendCmdLocParam1(false, CMD_MONSTDEATH, Monsters[mid].position.tile, mid);

	if (i < MAX_PLRS) {
		Monsters[mid].mWhoHit |= 1 << i;
		if (mid >= MAX_PLRS)
			AddPlrMonstExper(Monsters[mid].mLevel, Monsters[mid].mExp, Monsters[mid].mWhoHit);
	}

	MonsterKillCounts[Monsters[mid].MType->mtype]++;
	Monsters[mid]._mhitpoints = 0;
	SetRndSeed(Monsters[mid]._mRndSeed);

	SpawnLoot(mid, true);

	if (Monsters[mid].MType->mtype == MT_DIABLO)
		M_DiabloDeath(mid, true);
	else
		PlayEffect(mid, 2);

	Direction md = opposite[Monsters[i]._mdir];
	if (Monsters[mid].MType->mtype == MT_GOLEM)
		md = DIR_S;

	Monsters[mid]._mdir = md;
	NewMonsterAnim(mid, &Monsters[mid].MType->Anims[MA_DEATH], md, gGameLogicStep < GameLogicStep::ProcessMonsters ? AnimationDistributionFlags::ProcessAnimationPending : AnimationDistributionFlags::None);
	Monsters[mid]._mmode = MM_DEATH;
	Monsters[mid].position.offset = { 0, 0 };
	Monsters[mid].position.tile = Monsters[mid].position.old;
	Monsters[mid].position.future = Monsters[mid].position.old;
	M_ClearSquares(mid);
	dMonster[Monsters[mid].position.tile.x][Monsters[mid].position.tile.y] = mid + 1;
	CheckQuestKill(mid, true);
	M_FallenFear(Monsters[mid].position.tile);
	if (Monsters[mid].MType->mtype >= MT_NACID && Monsters[mid].MType->mtype <= MT_XACID)
		AddMissile(Monsters[mid].position.tile, { 0, 0 }, 0, MIS_ACIDPUD, TARGET_PLAYERS, mid, Monsters[mid]._mint + 1, 0);

	if (gbIsHellfire)
		M_StartStand(i, Monsters[i]._mdir);
}

void M_StartKill(int i, int pnum)
{
	assurance((DWORD)i < MAXMONSTERS, i);

	if (myplr == pnum) {
		delta_kill_monster(i, Monsters[i].position.tile, currlevel);
		if (i != pnum) {
			NetSendCmdLocParam1(false, CMD_MONSTDEATH, Monsters[i].position.tile, i);
		} else {
			NetSendCmdLocParam1(false, CMD_KILLGOLEM, Monsters[i].position.tile, currlevel);
		}
	}

	MonstStartKill(i, pnum, true);
}

void M_SyncStartKill(int i, Point position, int pnum)
{
	assurance((DWORD)i < MAXMONSTERS, i);

	if (Monsters[i]._mhitpoints > 0 || Monsters[i]._mmode == MM_DEATH) {
		return;
	}

	if (dMonster[position.x][position.y] == 0) {
		M_ClearSquares(i);
		Monsters[i].position.tile = position;
		Monsters[i].position.old = position;
	}

	if (Monsters[i]._mmode == MM_STONE) {
		MonstStartKill(i, pnum, false);
		Monsters[i].Petrify();
	} else {
		MonstStartKill(i, pnum, false);
	}
}

void M_StartFadein(int i, Direction md, bool backwards)
{
	assurance((DWORD)i < MAXMONSTERS, i);
	assurance(Monsters[i].MType != nullptr, i);

	NewMonsterAnim(i, &Monsters[i].MType->Anims[MA_SPECIAL], md);
	Monsters[i]._mmode = MM_FADEIN;
	Monsters[i].position.offset = { 0, 0 };
	Monsters[i].position.future = Monsters[i].position.tile;
	Monsters[i].position.old = Monsters[i].position.tile;
	Monsters[i]._mdir = md;
	Monsters[i]._mFlags &= ~MFLAG_HIDDEN;
	if (backwards) {
		Monsters[i]._mFlags |= MFLAG_LOCK_ANIMATION;
		Monsters[i].AnimInfo.CurrentFrame = Monsters[i].AnimInfo.NumberOfFrames;
	}
}

void M_StartFadeout(int i, Direction md, bool backwards)
{
	assurance((DWORD)i < MAXMONSTERS, i);
	assurance(Monsters[i].MType != nullptr, i);
	assurance(Monsters[i].MType != nullptr, i);

	NewMonsterAnim(i, &Monsters[i].MType->Anims[MA_SPECIAL], md);
	Monsters[i]._mmode = MM_FADEOUT;
	Monsters[i].position.offset = { 0, 0 };
	Monsters[i].position.future = Monsters[i].position.tile;
	Monsters[i].position.old = Monsters[i].position.tile;
	Monsters[i]._mdir = md;
	if (backwards) {
		Monsters[i]._mFlags |= MFLAG_LOCK_ANIMATION;
		Monsters[i].AnimInfo.CurrentFrame = Monsters[i].AnimInfo.NumberOfFrames;
	}
}

void M_StartHeal(int i)
{
	assurance((DWORD)i < MAXMONSTERS, i);
	assurance(Monsters[i].MType != nullptr, i);

	MonsterStruct *monst = &Monsters[i];
	monst->AnimInfo.pCelSprite = &*monst->MType->Anims[MA_SPECIAL].CelSpritesForDirections[monst->_mdir];
	monst->AnimInfo.CurrentFrame = monst->MType->Anims[MA_SPECIAL].Frames;
	monst->_mFlags |= MFLAG_LOCK_ANIMATION;
	monst->_mmode = MM_HEAL;
	monst->_mVar1 = monst->_mmaxhp / (16 * (GenerateRnd(5) + 4));
}

void M_ChangeLightOffset(int monst)
{
	assurance((DWORD)monst < MAXMONSTERS, monst);

	int lx = (Monsters[monst].position.offset.deltaX + 2 * Monsters[monst].position.offset.deltaY) / 8;
	int ly = (2 * Monsters[monst].position.offset.deltaY - Monsters[monst].position.offset.deltaX) / 8;

	if (Monsters[monst].mlid != NO_LIGHT)
		ChangeLightOff(Monsters[monst].mlid, { lx, ly });
}

bool M_DoStand(int i)
{
	commitment((DWORD)i < MAXMONSTERS, i);
	commitment(Monsters[i].MType != nullptr, i);

	MonsterStruct *monst = &Monsters[i];
	if (monst->MType->mtype == MT_GOLEM)
		monst->AnimInfo.pCelSprite = &*monst->MType->Anims[MA_WALK].CelSpritesForDirections[monst->_mdir];
	else
		monst->AnimInfo.pCelSprite = &*monst->MType->Anims[MA_STAND].CelSpritesForDirections[monst->_mdir];

	if (monst->AnimInfo.CurrentFrame == monst->AnimInfo.NumberOfFrames)
		M_Enemy(i);

	monst->_mVar2++;

	return false;
}

/**
 * @brief Continue movement towards new tile
 */
bool M_DoWalk(int i, int variant)
{
	commitment((DWORD)i < MAXMONSTERS, i);
	commitment(Monsters[i].MType != nullptr, i);

	//Check if we reached new tile
	bool isAnimationEnd = Monsters[i].AnimInfo.CurrentFrame == Monsters[i].MType->Anims[MA_WALK].Frames;
	if (isAnimationEnd) {
		switch (variant) {
		case MM_WALK:
			dMonster[Monsters[i].position.tile.x][Monsters[i].position.tile.y] = 0;
			Monsters[i].position.tile.x += Monsters[i]._mVar1;
			Monsters[i].position.tile.y += Monsters[i]._mVar2;
			dMonster[Monsters[i].position.tile.x][Monsters[i].position.tile.y] = i + 1;
			break;
		case MM_WALK2:
			dMonster[Monsters[i]._mVar1][Monsters[i]._mVar2] = 0;
			break;
		case MM_WALK3:
			dMonster[Monsters[i].position.tile.x][Monsters[i].position.tile.y] = 0;
			Monsters[i].position.tile = { Monsters[i]._mVar1, Monsters[i]._mVar2 };
			dFlags[Monsters[i].position.temp.x][Monsters[i].position.temp.y] &= ~BFLAG_MONSTLR;
			dMonster[Monsters[i].position.tile.x][Monsters[i].position.tile.y] = i + 1;
			break;
		}
		if (Monsters[i].mlid != NO_LIGHT)
			ChangeLightXY(Monsters[i].mlid, Monsters[i].position.tile);
		M_StartStand(i, Monsters[i]._mdir);
	} else { //We didn't reach new tile so update monster's "sub-tile" position
		if (Monsters[i].AnimInfo.TickCounterOfCurrentFrame == 0) {
			if (Monsters[i].AnimInfo.CurrentFrame == 0 && Monsters[i].MType->mtype == MT_FLESTHNG)
				PlayEffect(i, 3);
			Monsters[i].position.offset2 += Monsters[i].position.velocity;
			Monsters[i].position.offset.deltaX = Monsters[i].position.offset2.deltaX >> 4;
			Monsters[i].position.offset.deltaY = Monsters[i].position.offset2.deltaY >> 4;
		}
	}

	if (Monsters[i].mlid != NO_LIGHT) // BUGFIX: change uniqtype check to mlid check like it is in all other places (fixed)
		M_ChangeLightOffset(i);

	return isAnimationEnd;
}

void M_TryM2MHit(int i, int mid, int hper, int mind, int maxd)
{
	assurance((DWORD)mid < MAXMONSTERS, mid);
	assurance(Monsters[mid].MType != nullptr, mid);

	if (Monsters[mid]._mhitpoints >> 6 > 0 && (Monsters[mid].MType->mtype != MT_ILLWEAV || Monsters[mid]._mgoal != MGOAL_RETREAT)) {
		int hit = GenerateRnd(100);
		if (Monsters[mid]._mmode == MM_STONE)
			hit = 0;
		bool unused;
		if (!CheckMonsterHit(mid, &unused) && hit < hper) {
			int dam = (mind + GenerateRnd(maxd - mind + 1)) << 6;
			Monsters[mid]._mhitpoints -= dam;
			if (Monsters[mid]._mhitpoints >> 6 <= 0) {
				if (Monsters[mid]._mmode == MM_STONE) {
					M2MStartKill(i, mid);
					Monsters[mid].Petrify();
				} else {
					M2MStartKill(i, mid);
				}
			} else {
				if (Monsters[mid]._mmode == MM_STONE) {
					M2MStartHit(mid, i, dam);
					Monsters[mid].Petrify();
				} else {
					M2MStartHit(mid, i, dam);
				}
			}
		}
	}
}

void M_TryH2HHit(int i, int pnum, int hit, int minDam, int maxDam)
{
	assurance((DWORD)i < MAXMONSTERS, i);
	assurance(Monsters[i].MType != nullptr, i);

	if ((Monsters[i]._mFlags & MFLAG_TARGETS_MONSTER) != 0) {
		M_TryM2MHit(i, pnum, hit, minDam, maxDam);
		return;
	}
	if (plr[pnum]._pHitPoints >> 6 <= 0 || plr[pnum]._pInvincible || (plr[pnum]._pSpellFlags & 1) != 0)
		return;
	if (Monsters[i].position.tile.WalkingDistance(plr[pnum].position.tile) >= 2)
		return;

	int hper = GenerateRnd(100);
#ifdef _DEBUG
	if (debug_mode_dollar_sign || debug_mode_key_inverted_v)
		hper = 1000;
#endif
	int ac = plr[pnum]._pIBonusAC + plr[pnum]._pIAC;
	if ((plr[pnum].pDamAcFlags & ISPLHF_ACDEMON) != 0 && Monsters[i].MData->mMonstClass == MC_DEMON)
		ac += 40;
	if ((plr[pnum].pDamAcFlags & ISPLHF_ACUNDEAD) != 0 && Monsters[i].MData->mMonstClass == MC_UNDEAD)
		ac += 20;
	hit += 2 * (Monsters[i].mLevel - plr[pnum]._pLevel)
	    + 30
	    - ac
	    - plr[pnum]._pDexterity / 5;
	if (hit < 15)
		hit = 15;
	if (currlevel == 14 && hit < 20)
		hit = 20;
	if (currlevel == 15 && hit < 25)
		hit = 25;
	if (currlevel == 16 && hit < 30)
		hit = 30;
	int blkper = 100;
	if ((plr[pnum]._pmode == PM_STAND || plr[pnum]._pmode == PM_ATTACK) && plr[pnum]._pBlockFlag) {
		blkper = GenerateRnd(100);
	}
	int blk = plr[pnum]._pDexterity
	    + plr[pnum]._pBaseToBlk
	    - (Monsters[i].mLevel * 2)
	    + (plr[pnum]._pLevel * 2);
	if (blk < 0)
		blk = 0;
	if (blk > 100)
		blk = 100;
	if (hper >= hit)
		return;
	if (blkper < blk) {
		Direction dir = GetDirection(plr[pnum].position.tile, Monsters[i].position.tile);
		StartPlrBlock(pnum, dir);
		if (pnum == myplr && plr[pnum].wReflections > 0) {
			plr[pnum].wReflections--;
			int dam = GenerateRnd((maxDam - minDam + 1) << 6) + (minDam << 6);
			dam += plr[pnum]._pIGetHit << 6;
			if (dam < 64)
				dam = 64;
			int mdam = dam * (GenerateRnd(10) + 20L) / 100;
			Monsters[i]._mhitpoints -= mdam;
			dam -= mdam;
			if (dam < 0)
				dam = 0;
			if (Monsters[i]._mhitpoints >> 6 <= 0)
				M_StartKill(i, pnum);
			else
				M_StartHit(i, pnum, mdam);
		}
		return;
	}
	if (Monsters[i].MType->mtype == MT_YZOMBIE && pnum == myplr) {
		int currentMissileId = -1;
		for (int j = 0; j < nummissiles; j++) {
			int mi = missileactive[j];
			if (missile[mi]._mitype != MIS_MANASHIELD)
				continue;
			if (missile[mi]._misource == pnum)
				currentMissileId = mi;
		}
		if (plr[pnum]._pMaxHP > 64) {
			if (plr[pnum]._pMaxHPBase > 64) {
				plr[pnum]._pMaxHP -= 64;
				if (plr[pnum]._pHitPoints > plr[pnum]._pMaxHP) {
					plr[pnum]._pHitPoints = plr[pnum]._pMaxHP;
					if (currentMissileId >= 0)
						missile[currentMissileId]._miVar1 = plr[pnum]._pHitPoints;
				}
				plr[pnum]._pMaxHPBase -= 64;
				if (plr[pnum]._pHPBase > plr[pnum]._pMaxHPBase) {
					plr[pnum]._pHPBase = plr[pnum]._pMaxHPBase;
					if (currentMissileId >= 0)
						missile[currentMissileId]._miVar2 = plr[pnum]._pHPBase;
				}
			}
		}
	}
	int dam = (minDam << 6) + GenerateRnd((maxDam - minDam + 1) << 6);
	dam += (plr[pnum]._pIGetHit << 6);
	if (dam < 64)
		dam = 64;
	if (pnum == myplr) {
		if (plr[pnum].wReflections > 0) {
			plr[pnum].wReflections--;
			int mdam = dam * (GenerateRnd(10) + 20L) / 100;
			Monsters[i]._mhitpoints -= mdam;
			dam -= mdam;
			if (dam < 0)
				dam = 0;
			if (Monsters[i]._mhitpoints >> 6 <= 0)
				M_StartKill(i, pnum);
			else
				M_StartHit(i, pnum, mdam);
		}
		ApplyPlrDamage(pnum, 0, 0, dam);
	}
	if ((plr[pnum]._pIFlags & ISPL_THORNS) != 0) {
		int mdam = (GenerateRnd(3) + 1) << 6;
		Monsters[i]._mhitpoints -= mdam;
		if (Monsters[i]._mhitpoints >> 6 <= 0)
			M_StartKill(i, pnum);
		else
			M_StartHit(i, pnum, mdam);
	}
	if ((Monsters[i]._mFlags & MFLAG_NOLIFESTEAL) == 0 && Monsters[i].MType->mtype == MT_SKING && gbIsMultiplayer)
		Monsters[i]._mhitpoints += dam;
	if (plr[pnum]._pHitPoints >> 6 <= 0) {
		if (gbIsHellfire)
			M_StartStand(i, Monsters[i]._mdir);
		return;
	}
	StartPlrHit(pnum, dam, false);
	if ((Monsters[i]._mFlags & MFLAG_KNOCKBACK) != 0) {
		if (plr[pnum]._pmode != PM_GOTHIT)
			StartPlrHit(pnum, 0, true);

		Point newPosition = plr[pnum].position.tile + Monsters[i]._mdir;
		if (PosOkPlayer(pnum, newPosition)) {
			plr[pnum].position.tile = newPosition;
			FixPlayerLocation(pnum, plr[pnum]._pdir);
			FixPlrWalkTags(pnum);
			dPlayer[newPosition.x][newPosition.y] = pnum + 1;
			SetPlayerOld(plr[pnum]);
		}
	}
}

bool M_DoAttack(int i)
{
	commitment((DWORD)i < MAXMONSTERS, i);
	MonsterStruct *monst = &Monsters[i];
	commitment(monst->MType != nullptr, i);
	commitment(monst->MData != nullptr, i); // BUGFIX: should check MData (fixed)

	if (Monsters[i].AnimInfo.CurrentFrame == Monsters[i].MData->mAFNum) {
		M_TryH2HHit(i, Monsters[i]._menemy, Monsters[i].mHit, Monsters[i].mMinDamage, Monsters[i].mMaxDamage);
		if (Monsters[i]._mAi != AI_SNAKE)
			PlayEffect(i, 0);
	}
	if (Monsters[i].MType->mtype >= MT_NMAGMA && Monsters[i].MType->mtype <= MT_WMAGMA && Monsters[i].AnimInfo.CurrentFrame == 9) {
		M_TryH2HHit(i, Monsters[i]._menemy, Monsters[i].mHit + 10, Monsters[i].mMinDamage - 2, Monsters[i].mMaxDamage - 2);
		PlayEffect(i, 0);
	}
	if (Monsters[i].MType->mtype >= MT_STORM && Monsters[i].MType->mtype <= MT_MAEL && Monsters[i].AnimInfo.CurrentFrame == 13) {
		M_TryH2HHit(i, Monsters[i]._menemy, Monsters[i].mHit - 20, Monsters[i].mMinDamage + 4, Monsters[i].mMaxDamage + 4);
		PlayEffect(i, 0);
	}
	if (Monsters[i]._mAi == AI_SNAKE && Monsters[i].AnimInfo.CurrentFrame == 1)
		PlayEffect(i, 0);
	if (Monsters[i].AnimInfo.CurrentFrame == Monsters[i].AnimInfo.NumberOfFrames) {
		M_StartStand(i, Monsters[i]._mdir);
		return true;
	}

	return false;
}

bool M_DoRAttack(int i)
{
	commitment((DWORD)i < MAXMONSTERS, i);
	commitment(Monsters[i].MType != nullptr, i);
	commitment(Monsters[i].MData != nullptr, i);

	if (Monsters[i].AnimInfo.CurrentFrame == Monsters[i].MData->mAFNum) {
		if (Monsters[i]._mVar1 != -1) {
			int multimissiles = 1;
			if (Monsters[i]._mVar1 == MIS_CBOLT)
				multimissiles = 3;
			for (int mi = 0; mi < multimissiles; mi++) {
				Point sourcePosition = Monsters[i].position.tile;
				if (gbIsHellfire) {
					sourcePosition += Monsters[i]._mdir;
				}

				AddMissile(
				    sourcePosition,
				    Monsters[i].enemyPosition,
				    Monsters[i]._mdir,
				    Monsters[i]._mVar1,
				    TARGET_PLAYERS,
				    i,
				    Monsters[i]._mVar2,
				    0);
			}
		}
		PlayEffect(i, 0);
	}

	if (Monsters[i].AnimInfo.CurrentFrame == Monsters[i].AnimInfo.NumberOfFrames) {
		M_StartStand(i, Monsters[i]._mdir);
		return true;
	}

	return false;
}

bool M_DoRSpAttack(int i)
{
	commitment((DWORD)i < MAXMONSTERS, i);
	commitment(Monsters[i].MType != nullptr, i);
	commitment(Monsters[i].MData != nullptr, i); // BUGFIX: should check MData (fixed)

	if (Monsters[i].AnimInfo.CurrentFrame == Monsters[i].MData->mAFNum2 && Monsters[i].AnimInfo.TickCounterOfCurrentFrame == 0) {
		Point sourcePosition = Monsters[i].position.tile;
		if (gbIsHellfire) {
			sourcePosition += Monsters[i]._mdir;
		}

		AddMissile(
		    sourcePosition,
		    Monsters[i].enemyPosition,
		    Monsters[i]._mdir,
		    Monsters[i]._mVar1,
		    TARGET_PLAYERS,
		    i,
		    Monsters[i]._mVar3,
		    0);
		PlayEffect(i, 3);
	}

	if (Monsters[i]._mAi == AI_MEGA && Monsters[i].AnimInfo.CurrentFrame == Monsters[i].MData->mAFNum2) {
		if (Monsters[i]._mVar2++ == 0) {
			Monsters[i]._mFlags |= MFLAG_ALLOW_SPECIAL;
		} else if (Monsters[i]._mVar2 == 15) {
			Monsters[i]._mFlags &= ~MFLAG_ALLOW_SPECIAL;
		}
	}

	if (Monsters[i].AnimInfo.CurrentFrame == Monsters[i].AnimInfo.NumberOfFrames) {
		M_StartStand(i, Monsters[i]._mdir);
		return true;
	}

	return false;
}

bool M_DoSAttack(int i)
{
	commitment((DWORD)i < MAXMONSTERS, i);
	commitment(Monsters[i].MType != nullptr, i);
	commitment(Monsters[i].MData != nullptr, i);

	if (Monsters[i].AnimInfo.CurrentFrame == Monsters[i].MData->mAFNum2)
		M_TryH2HHit(i, Monsters[i]._menemy, Monsters[i].mHit2, Monsters[i].mMinDamage2, Monsters[i].mMaxDamage2);

	if (Monsters[i].AnimInfo.CurrentFrame == Monsters[i].AnimInfo.NumberOfFrames) {
		M_StartStand(i, Monsters[i]._mdir);
		return true;
	}

	return false;
}

bool M_DoFadein(int i)
{
	commitment((DWORD)i < MAXMONSTERS, i);

	if (((Monsters[i]._mFlags & MFLAG_LOCK_ANIMATION) == 0 || Monsters[i].AnimInfo.CurrentFrame != 1)
	    && ((Monsters[i]._mFlags & MFLAG_LOCK_ANIMATION) != 0 || Monsters[i].AnimInfo.CurrentFrame != Monsters[i].AnimInfo.NumberOfFrames)) {
		return false;
	}

	M_StartStand(i, Monsters[i]._mdir);
	Monsters[i]._mFlags &= ~MFLAG_LOCK_ANIMATION;

	return true;
}

bool M_DoFadeout(int i)
{
	commitment((DWORD)i < MAXMONSTERS, i);

	if (((Monsters[i]._mFlags & MFLAG_LOCK_ANIMATION) == 0 || Monsters[i].AnimInfo.CurrentFrame != 1)
	    && ((Monsters[i]._mFlags & MFLAG_LOCK_ANIMATION) != 0 || Monsters[i].AnimInfo.CurrentFrame != Monsters[i].AnimInfo.NumberOfFrames)) {
		return false;
	}

	int mt = Monsters[i].MType->mtype;
	if (mt < MT_INCIN || mt > MT_HELLBURN) {
		Monsters[i]._mFlags &= ~MFLAG_LOCK_ANIMATION;
		Monsters[i]._mFlags |= MFLAG_HIDDEN;
	} else {
		Monsters[i]._mFlags &= ~MFLAG_LOCK_ANIMATION;
	}

	M_StartStand(i, Monsters[i]._mdir);

	return true;
}

bool M_DoHeal(int i)
{
	commitment((DWORD)i < MAXMONSTERS, i);
	MonsterStruct *monst = &Monsters[i];
	if ((Monsters[i]._mFlags & MFLAG_NOHEAL) != 0) {
		monst->_mFlags &= ~MFLAG_ALLOW_SPECIAL;
		monst->_mmode = MM_SATTACK;
		return false;
	}

	if (monst->AnimInfo.CurrentFrame == 1) {
		monst->_mFlags &= ~MFLAG_LOCK_ANIMATION;
		monst->_mFlags |= MFLAG_ALLOW_SPECIAL;
		if (monst->_mVar1 + monst->_mhitpoints < monst->_mmaxhp) {
			monst->_mhitpoints = monst->_mVar1 + monst->_mhitpoints;
		} else {
			monst->_mhitpoints = monst->_mmaxhp;
			monst->_mFlags &= ~MFLAG_ALLOW_SPECIAL;
			monst->_mmode = MM_SATTACK;
		}
	}
	return false;
}

bool M_DoTalk(int i)
{
	commitment((DWORD)i < MAXMONSTERS, i);

	MonsterStruct *monst = &Monsters[i];
	M_StartStand(i, Monsters[i]._mdir);
	monst->_mgoal = MGOAL_TALKING; // CODEFIX: apply Monst instead of Monsters[i] in the rest of the function
	if (effect_is_playing(Texts[Monsters[i].mtalkmsg].sfxnr))
		return false;
	InitQTextMsg(Monsters[i].mtalkmsg);
	if (Monsters[i]._uniqtype - 1 == UMT_GARBUD) {
		if (Monsters[i].mtalkmsg == TEXT_GARBUD1) {
			quests[Q_GARBUD]._qactive = QUEST_ACTIVE;
			quests[Q_GARBUD]._qlog = true; // BUGFIX: (?) for other quests qactive and qlog go together, maybe this should actually go into the if above (fixed)
		}
		if (Monsters[i].mtalkmsg == TEXT_GARBUD2 && (Monsters[i]._mFlags & MFLAG_QUEST_COMPLETE) == 0) {
			SpawnItem(i, Monsters[i].position.tile + Displacement { 1, 1 }, true);
			Monsters[i]._mFlags |= MFLAG_QUEST_COMPLETE;
		}
	}
	if (Monsters[i]._uniqtype - 1 == UMT_ZHAR
	    && Monsters[i].mtalkmsg == TEXT_ZHAR1
	    && (Monsters[i]._mFlags & MFLAG_QUEST_COMPLETE) == 0) {
		quests[Q_ZHAR]._qactive = QUEST_ACTIVE;
		quests[Q_ZHAR]._qlog = true;
		CreateTypeItem(Monsters[i].position.tile + Displacement { 1, 1 }, false, ITYPE_MISC, IMISC_BOOK, true, false);
		Monsters[i]._mFlags |= MFLAG_QUEST_COMPLETE;
	}
	if (Monsters[i]._uniqtype - 1 == UMT_SNOTSPIL) {
		if (Monsters[i].mtalkmsg == TEXT_BANNER10 && (Monsters[i]._mFlags & MFLAG_QUEST_COMPLETE) == 0) {
			ObjChangeMap(setpc_x, setpc_y, (setpc_w / 2) + setpc_x + 2, (setpc_h / 2) + setpc_y - 2);
			auto tren = TransVal;
			TransVal = 9;
			DRLG_MRectTrans(setpc_x, setpc_y, (setpc_w / 2) + setpc_x + 4, setpc_y + (setpc_h / 2));
			TransVal = tren;
			quests[Q_LTBANNER]._qvar1 = 2;
			if (quests[Q_LTBANNER]._qactive == QUEST_INIT)
				quests[Q_LTBANNER]._qactive = QUEST_ACTIVE;
			Monsters[i]._mFlags |= MFLAG_QUEST_COMPLETE;
		}
		if (quests[Q_LTBANNER]._qvar1 < 2) {
			app_fatal("SS Talk = %i, Flags = %i", Monsters[i].mtalkmsg, Monsters[i]._mFlags);
		}
	}
	if (Monsters[i]._uniqtype - 1 == UMT_LACHDAN) {
		if (Monsters[i].mtalkmsg == TEXT_VEIL9) {
			quests[Q_VEIL]._qactive = QUEST_ACTIVE;
			quests[Q_VEIL]._qlog = true;
		}
		if (Monsters[i].mtalkmsg == TEXT_VEIL11 && (Monsters[i]._mFlags & MFLAG_QUEST_COMPLETE) == 0) {
			SpawnUnique(UITEM_STEELVEIL, Monsters[i].position.tile + DIR_S);
			Monsters[i]._mFlags |= MFLAG_QUEST_COMPLETE;
		}
	}
	if (Monsters[i]._uniqtype - 1 == UMT_WARLORD)
		quests[Q_WARLORD]._qvar1 = 2;
	if (Monsters[i]._uniqtype - 1 == UMT_LAZURUS && gbIsMultiplayer) {
		quests[Q_BETRAYER]._qvar1 = 6;
		Monsters[i]._mgoal = MGOAL_NORMAL;
		Monsters[i]._msquelch = UINT8_MAX;
		Monsters[i].mtalkmsg = TEXT_NONE;
	}
	return false;
}

void M_Teleport(int i)
{
	assurance((DWORD)i < MAXMONSTERS, i);

	MonsterStruct *monst = &Monsters[i];
	if (monst->_mmode == MM_STONE)
		return;

	int mx = monst->enemyPosition.x;
	int my = monst->enemyPosition.y;
	int rx = 2 * GenerateRnd(2) - 1;
	int ry = 2 * GenerateRnd(2) - 1;

	bool done = false;

	int x;
	int y;
	for (int j = -1; j <= 1 && !done; j++) {
		for (int k = -1; k < 1 && !done; k++) {
			if (j != 0 || k != 0) {
				x = mx + rx * j;
				y = my + ry * k;
				if (y >= 0 && y < MAXDUNY && x >= 0 && x < MAXDUNX && x != monst->position.tile.x && y != monst->position.tile.y) {
					if (PosOkMonst(i, { x, y }))
						done = true;
				}
			}
		}
	}

	if (done) {
		M_ClearSquares(i);
		dMonster[monst->position.tile.x][monst->position.tile.y] = 0;
		dMonster[x][y] = i + 1;
		monst->position.old = { x, y };
		monst->_mdir = M_GetDir(i);
	}
}

bool M_DoGotHit(int i)
{
	commitment((DWORD)i < MAXMONSTERS, i);
	commitment(Monsters[i].MType != nullptr, i);

	if (Monsters[i].AnimInfo.CurrentFrame == Monsters[i].AnimInfo.NumberOfFrames) {
		M_StartStand(i, Monsters[i]._mdir);

		return true;
	}

	return false;
}

void M_UpdateLeader(int i)
{
	assurance((DWORD)i < MAXMONSTERS, i);

	for (int j = 0; j < ActiveMonsterCount; j++) {
		int ma = ActiveMonsters[j];
		if (Monsters[ma].leaderflag == 1 && Monsters[ma].leader == i)
			Monsters[ma].leaderflag = 0;
	}

	if (Monsters[i].leaderflag == 1) {
		Monsters[Monsters[i].leader].packsize--;
	}
}

void DoEnding()
{
	if (gbIsMultiplayer) {
		SNetLeaveGame(LEAVE_ENDING);
	}

	music_stop();

	if (gbIsMultiplayer) {
		SDL_Delay(1000);
	}

	if (gbIsSpawn)
		return;

	switch (plr[myplr]._pClass) {
	case HeroClass::Sorcerer:
	case HeroClass::Monk:
		play_movie("gendata\\DiabVic1.smk", false);
		break;
	case HeroClass::Warrior:
	case HeroClass::Barbarian:
		play_movie("gendata\\DiabVic2.smk", false);
		break;
	default:
		play_movie("gendata\\DiabVic3.smk", false);
		break;
	}
	play_movie("gendata\\Diabend.smk", false);

	bool bMusicOn = gbMusicOn;
	gbMusicOn = true;

	int musicVolume = sound_get_or_set_music_volume(1);
	sound_get_or_set_music_volume(0);

	music_start(TMUSIC_L2);
	loop_movie = true;
	play_movie("gendata\\loopdend.smk", true);
	loop_movie = false;
	music_stop();

	sound_get_or_set_music_volume(musicVolume);
	gbMusicOn = bMusicOn;
}

void PrepDoEnding()
{
	gbSoundOn = sgbSaveSoundOn;
	gbRunGame = false;
	deathflag = false;
	cineflag = true;

	plr[myplr].pDiabloKillLevel = std::max(plr[myplr].pDiabloKillLevel, static_cast<uint8_t>(sgGameInitInfo.nDifficulty + 1));

	for (auto &player : plr) {
		player._pmode = PM_QUIT;
		player._pInvincible = true;
		if (gbIsMultiplayer) {
			if (player._pHitPoints >> 6 == 0)
				player._pHitPoints = 64;
			if (player._pMana >> 6 == 0)
				player._pMana = 64;
		}
	}
}

bool M_DoDeath(int i)
{
	commitment((DWORD)i < MAXMONSTERS, i);
	commitment(Monsters[i].MType != nullptr, i);

	Monsters[i]._mVar1++;
	if (Monsters[i].MType->mtype == MT_DIABLO) {
		if (Monsters[i].position.tile.x < ViewX) {
			ViewX--;
		} else if (Monsters[i].position.tile.x > ViewX) {
			ViewX++;
		}

		if (Monsters[i].position.tile.y < ViewY) {
			ViewY--;
		} else if (Monsters[i].position.tile.y > ViewY) {
			ViewY++;
		}

		if (Monsters[i]._mVar1 == 140)
			PrepDoEnding();
	} else if (Monsters[i].AnimInfo.CurrentFrame == Monsters[i].AnimInfo.NumberOfFrames) {
		if (Monsters[i]._uniqtype == 0)
			AddDead(Monsters[i].position.tile, Monsters[i].MType->mdeadval, Monsters[i]._mdir);
		else
			AddDead(Monsters[i].position.tile, Monsters[i]._udeadval, Monsters[i]._mdir);

		dMonster[Monsters[i].position.tile.x][Monsters[i].position.tile.y] = 0;
		Monsters[i]._mDelFlag = true;

		M_UpdateLeader(i);
	}
	return false;
}

bool M_DoSpStand(int i)
{
	commitment((DWORD)i < MAXMONSTERS, i);
	commitment(Monsters[i].MType != nullptr, i);

	if (Monsters[i].AnimInfo.CurrentFrame == Monsters[i].MData->mAFNum2)
		PlayEffect(i, 3);

	if (Monsters[i].AnimInfo.CurrentFrame == Monsters[i].AnimInfo.NumberOfFrames) {
		M_StartStand(i, Monsters[i]._mdir);
		return true;
	}

	return false;
}

bool M_DoDelay(int i)
{
	commitment((DWORD)i < MAXMONSTERS, i);
	commitment(Monsters[i].MType != nullptr, i);

	Monsters[i].AnimInfo.pCelSprite = &*Monsters[i].MType->Anims[MA_STAND].CelSpritesForDirections[M_GetDir(i)];
	if (Monsters[i]._mAi == AI_LAZURUS) {
		if (Monsters[i]._mVar2 > 8 || Monsters[i]._mVar2 < 0)
			Monsters[i]._mVar2 = 8;
	}

	if (Monsters[i]._mVar2-- == 0) {
		int oFrame = Monsters[i].AnimInfo.CurrentFrame;
		M_StartStand(i, Monsters[i]._mdir);
		Monsters[i].AnimInfo.CurrentFrame = oFrame;
		return true;
	}

	return false;
}

bool M_DoStone(int i)
{
	commitment((DWORD)i < MAXMONSTERS, i);

	if (Monsters[i]._mhitpoints <= 0) {
		dMonster[Monsters[i].position.tile.x][Monsters[i].position.tile.y] = 0;
		Monsters[i]._mDelFlag = true;
	}

	return false;
}

void M_WalkDir(int i, Direction md)
{
	assurance((DWORD)i < MAXMONSTERS, i);

	int mwi = Monsters[i].MType->Anims[MA_WALK].Frames - 1;
	switch (md) {
	case DIR_N:
		M_StartWalk(i, 0, -MWVel[mwi][1], -1, -1, DIR_N);
		break;
	case DIR_NE:
		M_StartWalk(i, MWVel[mwi][1], -MWVel[mwi][0], 0, -1, DIR_NE);
		break;
	case DIR_E:
		M_StartWalk3(i, MWVel[mwi][2], 0, -32, -16, 1, -1, 1, 0, DIR_E);
		break;
	case DIR_SE:
		M_StartWalk2(i, MWVel[mwi][1], MWVel[mwi][0], -32, -16, 1, 0, DIR_SE);
		break;
	case DIR_S:
		M_StartWalk2(i, 0, MWVel[mwi][1], 0, -32, 1, 1, DIR_S);
		break;
	case DIR_SW:
		M_StartWalk2(i, -MWVel[mwi][1], MWVel[mwi][0], 32, -16, 0, 1, DIR_SW);
		break;
	case DIR_W:
		M_StartWalk3(i, -MWVel[mwi][2], 0, 32, -16, -1, 1, 0, 1, DIR_W);
		break;
	case DIR_NW:
		M_StartWalk(i, -MWVel[mwi][1], -MWVel[mwi][0], -1, 0, DIR_NW);
		break;
	case DIR_OMNI:
		break;
	}
}

void GroupUnity(int i)
{
	assurance((DWORD)i < MAXMONSTERS, i);

	int leader;
	if (Monsters[i].leaderflag != 0) {
		leader = Monsters[i].leader;
		bool clear = LineClearSolid(Monsters[i].position.tile, Monsters[leader].position.future);
		if (clear || Monsters[i].leaderflag != 1) {
			if (clear
			    && Monsters[i].leaderflag == 2
			    && Monsters[i].position.tile.WalkingDistance(Monsters[leader].position.future) < 4) {
				Monsters[leader].packsize++;
				Monsters[i].leaderflag = 1;
			}
		} else {
			Monsters[leader].packsize--;
			Monsters[i].leaderflag = 2;
		}
	}

	if (Monsters[i].leaderflag == 1) {
		if (Monsters[i]._msquelch > Monsters[leader]._msquelch) {
			Monsters[leader].position.last = Monsters[i].position.tile;
			Monsters[leader]._msquelch = Monsters[i]._msquelch - 1;
		}
		if (Monsters[leader]._mAi == AI_GARG) {
			if ((Monsters[leader]._mFlags & MFLAG_ALLOW_SPECIAL) != 0) {
				Monsters[leader]._mFlags &= ~MFLAG_ALLOW_SPECIAL;
				Monsters[leader]._mmode = MM_SATTACK;
			}
		}
	} else if (Monsters[i]._uniqtype != 0) {
		if ((UniqMonst[Monsters[i]._uniqtype - 1].mUnqAttr & 2) != 0) {
			for (int j = 0; j < ActiveMonsterCount; j++) {
				int m = ActiveMonsters[j];
				if (Monsters[m].leaderflag == 1 && Monsters[m].leader == i) {
					if (Monsters[i]._msquelch > Monsters[m]._msquelch) {
						Monsters[m].position.last = Monsters[i].position.tile;
						Monsters[m]._msquelch = Monsters[i]._msquelch - 1;
					}
					if (Monsters[m]._mAi == AI_GARG) {
						if ((Monsters[m]._mFlags & MFLAG_ALLOW_SPECIAL) != 0) {
							Monsters[m]._mFlags &= ~MFLAG_ALLOW_SPECIAL;
							Monsters[m]._mmode = MM_SATTACK;
						}
					}
				}
			}
		}
	}
}

bool M_CallWalk(int i, Direction md)
{
	Direction mdtemp = md;
	bool ok = DirOK(i, md);
	if (GenerateRnd(2) != 0)
		ok = ok || (md = left[mdtemp], DirOK(i, md)) || (md = right[mdtemp], DirOK(i, md));
	else
		ok = ok || (md = right[mdtemp], DirOK(i, md)) || (md = left[mdtemp], DirOK(i, md));
	if (GenerateRnd(2) != 0) {
		ok = ok
		    || (md = right[right[mdtemp]], DirOK(i, md))
		    || (md = left[left[mdtemp]], DirOK(i, md));
	} else {
		ok = ok
		    || (md = left[left[mdtemp]], DirOK(i, md))
		    || (md = right[right[mdtemp]], DirOK(i, md));
	}
	if (ok)
		M_WalkDir(i, md);
	return ok;
}

bool M_PathWalk(int i)
{
	int8_t path[MAX_PATH_LENGTH];
	bool (*check)(int, Point);

	/** Maps from walking path step to facing direction. */
	const Direction plr2monst[9] = { DIR_S, DIR_NE, DIR_NW, DIR_SE, DIR_SW, DIR_N, DIR_E, DIR_S, DIR_W };

	commitment((DWORD)i < MAXMONSTERS, i);

	check = PosOkMonst3;
	if ((Monsters[i]._mFlags & MFLAG_CAN_OPEN_DOOR) == 0)
		check = PosOkMonst;

	if (FindPath(check, i, Monsters[i].position.tile.x, Monsters[i].position.tile.y, Monsters[i].enemyPosition.x, Monsters[i].enemyPosition.y, path) == 0) {
		return false;
	}

	M_CallWalk(i, plr2monst[path[0]]);
	return true;
}

bool M_CallWalk2(int i, Direction md)
{
	Direction mdtemp = md;
	bool ok = DirOK(i, md);    // Can we continue in the same direction
	if (GenerateRnd(2) != 0) { // Randomly go left or right
		ok = ok || (mdtemp = left[md], DirOK(i, left[md])) || (mdtemp = right[md], DirOK(i, right[md]));
	} else {
		ok = ok || (mdtemp = right[md], DirOK(i, right[md])) || (mdtemp = left[md], DirOK(i, left[md]));
	}

	if (ok)
		M_WalkDir(i, mdtemp);

	return ok;
}

bool M_DumbWalk(int i, Direction md)
{
	bool ok = DirOK(i, md);
	if (ok)
		M_WalkDir(i, md);

	return ok;
}

static Direction Turn(Direction direction, bool turnLeft)
{
	return turnLeft ? left[direction] : right[direction];
}

bool M_RoundWalk(int i, Direction direction, int *dir)
{
	Direction turn45deg = Turn(direction, *dir != 0);
	Direction turn90deg = Turn(turn45deg, *dir != 0);

	if (DirOK(i, turn90deg)) {
		// Turn 90 degrees
		M_WalkDir(i, turn90deg);
		return true;
	}

	if (DirOK(i, turn45deg)) {
		// Only do a small turn
		M_WalkDir(i, turn45deg);
		return true;
	}

	if (DirOK(i, direction)) {
		// Continue straight
		M_WalkDir(i, direction);
		return true;
	}

	// Try 90 degrees in the opposite than desired direction
	*dir = (*dir == 0) ? 1 : 0;
	return M_CallWalk(i, opposite[turn90deg]);
}

void MAI_Zombie(int i)
{
	assurance((DWORD)i < MAXMONSTERS, i);

	MonsterStruct *monst = &Monsters[i];
	if (monst->_mmode != MM_STAND) {
		return;
	}

	int mx = monst->position.tile.x;
	int my = monst->position.tile.y;
	if ((dFlags[mx][my] & BFLAG_VISIBLE) == 0) {
		return;
	}

	if (GenerateRnd(100) < 2 * monst->_mint + 10) {
		int dist = monst->enemyPosition.WalkingDistance({ mx, my });
		if (dist >= 2) {
			if (dist >= 2 * monst->_mint + 4) {
				Direction md = monst->_mdir;
				if (GenerateRnd(100) < 2 * monst->_mint + 20) {
					md = static_cast<Direction>(GenerateRnd(8));
				}
				M_DumbWalk(i, md);
			} else {
				M_CallWalk(i, M_GetDir(i));
			}
		} else {
			M_StartAttack(i);
		}
	}

	monst->CheckStandAnimationIsLoaded(monst->_mdir);
}

void MAI_SkelSd(int i)
{
	assurance((DWORD)i < MAXMONSTERS, i);

	MonsterStruct *monst = &Monsters[i];
	if (monst->_mmode != MM_STAND || monst->_msquelch == 0) {
		return;
	}

	int x = monst->position.tile.x - monst->enemyPosition.x;
	int y = monst->position.tile.y - monst->enemyPosition.y;
	Direction md = GetDirection(monst->position.tile, monst->position.last);
	monst->_mdir = md;
	if (abs(x) >= 2 || abs(y) >= 2) {
		if (monst->_mVar1 == MM_DELAY || (GenerateRnd(100) >= 35 - 4 * monst->_mint)) {
			M_CallWalk(i, md);
		} else {
			M_StartDelay(i, 15 - 2 * monst->_mint + GenerateRnd(10));
		}
	} else {
		if (monst->_mVar1 == MM_DELAY || (GenerateRnd(100) < 2 * monst->_mint + 20)) {
			M_StartAttack(i);
		} else {
			M_StartDelay(i, 2 * (5 - monst->_mint) + GenerateRnd(10));
		}
	}

	monst->CheckStandAnimationIsLoaded(md);
}

bool MAI_Path(int i)
{
	commitment((DWORD)i < MAXMONSTERS, i);

	MonsterStruct *monst = &Monsters[i];
	if (monst->MType->mtype != MT_GOLEM) {
		if (monst->_msquelch == 0)
			return false;
		if (monst->_mmode != MM_STAND)
			return false;
		if (monst->_mgoal != MGOAL_NORMAL && monst->_mgoal != MGOAL_MOVE && monst->_mgoal != MGOAL_ATTACK2)
			return false;
		if (monst->position.tile.x == 1 && monst->position.tile.y == 0)
			return false;
	}

	bool clear = LineClear(
	    PosOkMonst2,
	    i,
	    monst->position.tile,
	    monst->enemyPosition);
	if (!clear || (monst->_pathcount >= 5 && monst->_pathcount < 8)) {
		if ((monst->_mFlags & MFLAG_CAN_OPEN_DOOR) != 0)
			MonstCheckDoors(i);
		monst->_pathcount++;
		if (monst->_pathcount < 5)
			return false;
		if (M_PathWalk(i))
			return true;
	}

	if (monst->MType->mtype != MT_GOLEM)
		monst->_pathcount = 0;

	return false;
}

void MAI_Snake(int i)
{
	assurance((DWORD)i < MAXMONSTERS, i);
	char pattern[6] = { 1, 1, 0, -1, -1, 0 };
	MonsterStruct *monst = &Monsters[i];
	int pnum = monst->_menemy;
	if (monst->_mmode != MM_STAND || monst->_msquelch == 0)
		return;
	int fx = monst->enemyPosition.x;
	int fy = monst->enemyPosition.y;
	int mx = monst->position.tile.x - fx;
	int my = monst->position.tile.y - fy;
	Direction md = GetDirection(monst->position.tile, monst->position.last);
	monst->_mdir = md;
	if (abs(mx) >= 2 || abs(my) >= 2) {
		if (abs(mx) < 3 && abs(my) < 3 && LineClear(PosOkMonst, i, monst->position.tile, { fx, fy }) && monst->_mVar1 != MM_CHARGE) {
			if (AddMissile(monst->position.tile, { fx, fy }, md, MIS_RHINO, pnum, i, 0, 0) != -1) {
				PlayEffect(i, 0);
				dMonster[monst->position.tile.x][monst->position.tile.y] = -(i + 1);
				monst->_mmode = MM_CHARGE;
			}
		} else if (monst->_mVar1 == MM_DELAY || GenerateRnd(100) >= 35 - 2 * monst->_mint) {
			if (pattern[monst->_mgoalvar1] == -1)
				md = left[md];
			else if (pattern[monst->_mgoalvar1] == 1)
				md = right[md];

			monst->_mgoalvar1++;
			if (monst->_mgoalvar1 > 5)
				monst->_mgoalvar1 = 0;

			if (md != monst->_mgoalvar2) {
				int drift = md - monst->_mgoalvar2;
				if (drift < 0)
					drift += 8;

				if (drift < 4)
					md = right[monst->_mgoalvar2];
				else if (drift > 4)
					md = left[monst->_mgoalvar2];
				monst->_mgoalvar2 = md;
			}

			if (!M_DumbWalk(i, md))
				M_CallWalk2(i, monst->_mdir);
		} else {
			M_StartDelay(i, 15 - monst->_mint + GenerateRnd(10));
		}
	} else {
		if (monst->_mVar1 == MM_DELAY
		    || monst->_mVar1 == MM_CHARGE
		    || (GenerateRnd(100) < monst->_mint + 20)) {
			M_StartAttack(i);
		} else
			M_StartDelay(i, 10 - monst->_mint + GenerateRnd(10));
	}

	monst->CheckStandAnimationIsLoaded(monst->_mdir);
}

void MAI_Bat(int i)
{
	assurance((DWORD)i < MAXMONSTERS, i);

	MonsterStruct *monst = &Monsters[i];
	int pnum = monst->_menemy;
	if (monst->_mmode != MM_STAND || monst->_msquelch == 0) {
		return;
	}

	int xd = monst->position.tile.x - monst->enemyPosition.x;
	int yd = monst->position.tile.y - monst->enemyPosition.y;
	Direction md = GetDirection(monst->position.tile, monst->position.last);
	monst->_mdir = md;
	int v = GenerateRnd(100);
	if (monst->_mgoal == MGOAL_RETREAT) {
		if (monst->_mgoalvar1 == 0) {
			M_CallWalk(i, opposite[md]);
			monst->_mgoalvar1++;
		} else {
			if (GenerateRnd(2) != 0)
				M_CallWalk(i, left[md]);
			else
				M_CallWalk(i, right[md]);
			monst->_mgoal = MGOAL_NORMAL;
		}
		return;
	}

	int fx = monst->enemyPosition.x;
	int fy = monst->enemyPosition.y;
	if (monst->MType->mtype == MT_GLOOM
	    && (abs(xd) >= 5 || abs(yd) >= 5)
	    && v < 4 * monst->_mint + 33
	    && LineClear(PosOkMonst, i, monst->position.tile, { fx, fy })) {
		if (AddMissile(monst->position.tile, { fx, fy }, md, MIS_RHINO, pnum, i, 0, 0) != -1) {
			dMonster[monst->position.tile.x][monst->position.tile.y] = -(i + 1);
			monst->_mmode = MM_CHARGE;
		}
	} else if (abs(xd) >= 2 || abs(yd) >= 2) {
		if ((monst->_mVar2 > 20 && v < monst->_mint + 13)
		    || ((monst->_mVar1 == MM_WALK || monst->_mVar1 == MM_WALK2 || monst->_mVar1 == MM_WALK3)
		        && monst->_mVar2 == 0
		        && v < monst->_mint + 63)) {
			M_CallWalk(i, md);
		}
	} else if (v < 4 * monst->_mint + 8) {
		M_StartAttack(i);
		monst->_mgoal = MGOAL_RETREAT;
		monst->_mgoalvar1 = 0;
		if (monst->MType->mtype == MT_FAMILIAR) {
			AddMissile(monst->enemyPosition, { monst->enemyPosition.x + 1, 0 }, -1, MIS_LIGHTNING, TARGET_PLAYERS, i, GenerateRnd(10) + 1, 0);
		}
	}

	monst->CheckStandAnimationIsLoaded(md);
}

void MAI_SkelBow(int i)
{
	assurance((DWORD)i < MAXMONSTERS, i);

	MonsterStruct *monst = &Monsters[i];
	if (monst->_mmode != MM_STAND || monst->_msquelch == 0) {
		return;
	}

	int mx = monst->position.tile.x - monst->enemyPosition.x;
	int my = monst->position.tile.y - monst->enemyPosition.y;

	Direction md = M_GetDir(i);
	monst->_mdir = md;
	int v = GenerateRnd(100);

	bool walking = false;

	if (abs(mx) < 4 && abs(my) < 4) {
		if ((monst->_mVar2 > 20 && v < 2 * monst->_mint + 13)
		    || ((monst->_mVar1 == MM_WALK || monst->_mVar1 == MM_WALK2 || monst->_mVar1 == MM_WALK3)
		        && monst->_mVar2 == 0
		        && v < 2 * monst->_mint + 63)) {
			walking = M_DumbWalk(i, opposite[md]);
		}
	}

	if (!walking) {
		if (GenerateRnd(100) < 2 * monst->_mint + 3) {
			if (LineClearMissile(monst->position.tile, monst->enemyPosition))
				M_StartRAttack(i, MIS_ARROW, 4);
		}
	}

	monst->CheckStandAnimationIsLoaded(md);
}

void MAI_Fat(int i)
{
	assurance((DWORD)i < MAXMONSTERS, i);

	MonsterStruct *monst = &Monsters[i];
	if (monst->_mmode != MM_STAND || monst->_msquelch == 0) {
		return;
	}

	int mx = monst->position.tile.x - monst->enemyPosition.x;
	int my = monst->position.tile.y - monst->enemyPosition.y;
	Direction md = M_GetDir(i);
	monst->_mdir = md;
	int v = GenerateRnd(100);
	if (abs(mx) >= 2 || abs(my) >= 2) {
		if ((monst->_mVar2 > 20 && v < 4 * monst->_mint + 20)
		    || ((monst->_mVar1 == MM_WALK || monst->_mVar1 == MM_WALK2 || monst->_mVar1 == MM_WALK3)
		        && monst->_mVar2 == 0
		        && v < 4 * monst->_mint + 70)) {
			M_CallWalk(i, md);
		}
	} else if (v < 4 * monst->_mint + 15) {
		M_StartAttack(i);
	} else if (v < 4 * monst->_mint + 20) {
		M_StartSpAttack(i);
	}

	monst->CheckStandAnimationIsLoaded(md);
}

void MAI_Sneak(int i)
{
	assurance((DWORD)i < MAXMONSTERS, i);

	MonsterStruct *monst = &Monsters[i];
	if (monst->_mmode != MM_STAND) {
		return;
	}
	int mx = monst->position.tile.x;
	int my = monst->position.tile.y;
	if (dLight[mx][my] == LightsMax) {
		return;
	}
	mx -= monst->enemyPosition.x;
	my -= monst->enemyPosition.y;

	int dist = 5 - monst->_mint;
	if (monst->_mVar1 == MM_GOTHIT) {
		monst->_mgoal = MGOAL_RETREAT;
		monst->_mgoalvar1 = 0;
	} else if (abs(mx) >= dist + 3 || abs(my) >= dist + 3 || monst->_mgoalvar1 > 8) {
		monst->_mgoal = MGOAL_NORMAL;
		monst->_mgoalvar1 = 0;
	}
	Direction md = M_GetDir(i);
	if (monst->_mgoal == MGOAL_RETREAT && (monst->_mFlags & MFLAG_NO_ENEMY) == 0) {
		if ((monst->_mFlags & MFLAG_TARGETS_MONSTER) != 0)
			md = GetDirection(monst->position.tile, Monsters[monst->_menemy].position.tile);
		else
			md = GetDirection(monst->position.tile, plr[monst->_menemy].position.last);
		md = opposite[md];
		if (monst->MType->mtype == MT_UNSEEN) {
			if (GenerateRnd(2) != 0)
				md = left[md];
			else
				md = right[md];
		}
	}
	monst->_mdir = md;
	int v = GenerateRnd(100);
	if (abs(mx) < dist && abs(my) < dist && (monst->_mFlags & MFLAG_HIDDEN) != 0) {
		M_StartFadein(i, md, false);
	} else {
		if ((abs(mx) >= dist + 1 || abs(my) >= dist + 1) && (monst->_mFlags & MFLAG_HIDDEN) == 0) {
			M_StartFadeout(i, md, true);
		} else {
			if (monst->_mgoal == MGOAL_RETREAT
			    || ((abs(mx) >= 2 || abs(my) >= 2) && ((monst->_mVar2 > 20 && v < 4 * monst->_mint + 14) || ((monst->_mVar1 == MM_WALK || monst->_mVar1 == MM_WALK2 || monst->_mVar1 == MM_WALK3) && monst->_mVar2 == 0 && v < 4 * monst->_mint + 64)))) {
				monst->_mgoalvar1++;
				M_CallWalk(i, md);
			}
		}
	}
	if (monst->_mmode == MM_STAND) {
		if (abs(mx) >= 2 || abs(my) >= 2 || v >= 4 * monst->_mint + 10)
			monst->AnimInfo.pCelSprite = &*monst->MType->Anims[MA_STAND].CelSpritesForDirections[md];
		else
			M_StartAttack(i);
	}
}

void MAI_Fireman(int i)
{
	assurance((DWORD)i < MAXMONSTERS, i);

	MonsterStruct *monst = &Monsters[i];
	if (Monsters[i]._mmode != MM_STAND || monst->_msquelch == 0)
		return;

	int pnum = Monsters[i]._menemy;
	int fx = Monsters[i].enemyPosition.x;
	int fy = Monsters[i].enemyPosition.y;
	int xd = Monsters[i].position.tile.x - fx;
	int yd = Monsters[i].position.tile.y - fy;

	Direction md = M_GetDir(i);
	if (monst->_mgoal == MGOAL_NORMAL) {
		if (LineClearMissile(monst->position.tile, { fx, fy })
		    && AddMissile(monst->position.tile, { fx, fy }, md, MIS_FIREMAN, pnum, i, 0, 0) != -1) {
			monst->_mmode = MM_CHARGE;
			monst->_mgoal = MGOAL_ATTACK2;
			monst->_mgoalvar1 = 0;
		}
	} else if (monst->_mgoal == MGOAL_ATTACK2) {
		if (monst->_mgoalvar1 == 3) {
			monst->_mgoal = MGOAL_NORMAL;
			M_StartFadeout(i, md, true);
		} else if (LineClearMissile(monst->position.tile, { fx, fy })) {
			M_StartRAttack(i, MIS_KRULL, 4);
			monst->_mgoalvar1++;
		} else {
			M_StartDelay(i, GenerateRnd(10) + 5);
			monst->_mgoalvar1++;
		}
	} else if (monst->_mgoal == MGOAL_RETREAT) {
		M_StartFadein(i, md, false);
		monst->_mgoal = MGOAL_ATTACK2;
	}
	monst->_mdir = md;
	GenerateRnd(100);
	if (monst->_mmode != MM_STAND)
		return;

	if (abs(xd) < 2 && abs(yd) < 2 && monst->_mgoal == MGOAL_NORMAL) {
		M_TryH2HHit(i, Monsters[i]._menemy, Monsters[i].mHit, Monsters[i].mMinDamage, Monsters[i].mMaxDamage);
		monst->_mgoal = MGOAL_RETREAT;
		if (!M_CallWalk(i, opposite[md])) {
			M_StartFadein(i, md, false);
			monst->_mgoal = MGOAL_ATTACK2;
		}
	} else if (!M_CallWalk(i, md) && (monst->_mgoal == MGOAL_NORMAL || monst->_mgoal == MGOAL_RETREAT)) {
		M_StartFadein(i, md, false);
		monst->_mgoal = MGOAL_ATTACK2;
	}
}

void MAI_Fallen(int i)
{
	assurance((DWORD)i < MAXMONSTERS, i);

	MonsterStruct *monst = &Monsters[i];
	if (monst->_mgoal == MGOAL_ATTACK2) {
		if (monst->_mgoalvar1 != 0)
			monst->_mgoalvar1--;
		else
			monst->_mgoal = MGOAL_NORMAL;
	}
	if (monst->_mmode != MM_STAND || monst->_msquelch == 0) {
		return;
	}

	if (monst->_mgoal == MGOAL_RETREAT) {
		if (monst->_mgoalvar1-- == 0) {
			monst->_mgoal = MGOAL_NORMAL;
			M_StartStand(i, opposite[monst->_mdir]);
		}
	}

	if (monst->AnimInfo.CurrentFrame == monst->AnimInfo.NumberOfFrames) {
		if (GenerateRnd(4) != 0) {
			return;
		}
		if ((Monsters[i]._mFlags & MFLAG_NOHEAL) == 0) { // CODEFIX: - change to Monst-> in devilutionx
			M_StartSpStand(i, monst->_mdir);
			if (monst->_mmaxhp - (2 * monst->_mint + 2) >= monst->_mhitpoints)
				monst->_mhitpoints += 2 * monst->_mint + 2;
			else
				monst->_mhitpoints = monst->_mmaxhp;
		}
		int rad = 2 * monst->_mint + 4;
		for (int y = -rad; y <= rad; y++) {
			for (int x = -rad; x <= rad; x++) {
				int xpos = monst->position.tile.x + x;
				int ypos = monst->position.tile.y + y;
				if (y >= 0 && y < MAXDUNY && x >= 0 && x < MAXDUNX) {
					int m = dMonster[xpos][ypos];
					if (m > 0) {
						m--;
						if (Monsters[m]._mAi == AI_FALLEN) {
							Monsters[m]._mgoal = MGOAL_ATTACK2;
							Monsters[m]._mgoalvar1 = 30 * monst->_mint + 105;
						}
					}
				}
			}
		}
	} else if (monst->_mgoal == MGOAL_RETREAT) {
		M_CallWalk(i, monst->_mdir);
	} else if (monst->_mgoal == MGOAL_ATTACK2) {
		int xpos = monst->position.tile.x - monst->enemyPosition.x;
		int ypos = monst->position.tile.y - monst->enemyPosition.y;
		if (abs(xpos) < 2 && abs(ypos) < 2)
			M_StartAttack(i);
		else
			M_CallWalk(i, M_GetDir(i));
	} else
		MAI_SkelSd(i);
}

void MAI_Cleaver(int i)
{
	assurance((DWORD)i < MAXMONSTERS, i);

	MonsterStruct *monst = &Monsters[i];
	if (monst->_mmode != MM_STAND || monst->_msquelch == 0) {
		return;
	}

	int mx = monst->position.tile.x;
	int my = monst->position.tile.y;
	int x = mx - monst->enemyPosition.x;
	int y = my - monst->enemyPosition.y;

	Direction md = GetDirection({ mx, my }, monst->position.last);
	monst->_mdir = md;

	if (abs(x) >= 2 || abs(y) >= 2)
		M_CallWalk(i, md);
	else
		M_StartAttack(i);

	monst->CheckStandAnimationIsLoaded(md);
}

void MAI_Round(int i, bool special)
{
	assurance((DWORD)i < MAXMONSTERS, i);
	MonsterStruct *monst = &Monsters[i];
	if (monst->_mmode != MM_STAND || monst->_msquelch == 0) {
		return;
	}

	int fx = monst->enemyPosition.x;
	int fy = monst->enemyPosition.y;
	int mx = monst->position.tile.x - fx;
	int my = monst->position.tile.y - fy;
	Direction md = GetDirection(monst->position.tile, monst->position.last);
	if (monst->_msquelch < UINT8_MAX)
		MonstCheckDoors(i);
	int v = GenerateRnd(100);
	if ((abs(mx) >= 2 || abs(my) >= 2) && monst->_msquelch == UINT8_MAX && dTransVal[monst->position.tile.x][monst->position.tile.y] == dTransVal[fx][fy]) {
		if (monst->_mgoal == MGOAL_MOVE || ((abs(mx) >= 4 || abs(my) >= 4) && GenerateRnd(4) == 0)) {
			if (monst->_mgoal != MGOAL_MOVE) {
				monst->_mgoalvar1 = 0;
				monst->_mgoalvar2 = GenerateRnd(2);
			}
			monst->_mgoal = MGOAL_MOVE;
			int dist = std::max(abs(mx), abs(my));
			if ((monst->_mgoalvar1++ >= 2 * dist && DirOK(i, md)) || dTransVal[monst->position.tile.x][monst->position.tile.y] != dTransVal[fx][fy]) {
				monst->_mgoal = MGOAL_NORMAL;
			} else if (!M_RoundWalk(i, md, &monst->_mgoalvar2)) {
				M_StartDelay(i, GenerateRnd(10) + 10);
			}
		}
	} else {
		monst->_mgoal = MGOAL_NORMAL;
	}
	if (monst->_mgoal == MGOAL_NORMAL) {
		if (abs(mx) >= 2 || abs(my) >= 2) {
			if ((monst->_mVar2 > 20 && v < 2 * monst->_mint + 28)
			    || ((monst->_mVar1 == MM_WALK || monst->_mVar1 == MM_WALK2 || monst->_mVar1 == MM_WALK3)
			        && monst->_mVar2 == 0
			        && v < 2 * monst->_mint + 78)) {
				M_CallWalk(i, md);
			}
		} else if (v < 2 * monst->_mint + 23) {
			monst->_mdir = md;
			if (special && monst->_mhitpoints < (monst->_mmaxhp / 2) && GenerateRnd(2) != 0)
				M_StartSpAttack(i);
			else
				M_StartAttack(i);
		}
	}

	monst->CheckStandAnimationIsLoaded(md);
}

void MAI_GoatMc(int i)
{
	MAI_Round(i, true);
}

void MAI_Ranged(int i, missile_id missileType, bool special)
{
	assurance((DWORD)i < MAXMONSTERS, i);

	if (Monsters[i]._mmode != MM_STAND) {
		return;
	}

	MonsterStruct *monst = &Monsters[i];
	if (monst->_msquelch == UINT8_MAX || (monst->_mFlags & MFLAG_TARGETS_MONSTER) != 0) {
		int fx = monst->enemyPosition.x;
		int fy = monst->enemyPosition.y;
		int mx = monst->position.tile.x - fx;
		int my = monst->position.tile.y - fy;
		Direction md = M_GetDir(i);
		if (monst->_msquelch < UINT8_MAX)
			MonstCheckDoors(i);
		monst->_mdir = md;
		if (monst->_mVar1 == MM_RATTACK) {
			M_StartDelay(i, GenerateRnd(20));
		} else if (abs(mx) < 4 && abs(my) < 4) {
			if (GenerateRnd(100) < 10 * (monst->_mint + 7))
				M_CallWalk(i, opposite[md]);
		}
		if (monst->_mmode == MM_STAND) {
			if (LineClearMissile(monst->position.tile, { fx, fy })) {
				if (special)
					M_StartRSpAttack(i, missileType, 4);
				else
					M_StartRAttack(i, missileType, 4);
			} else {
				monst->AnimInfo.pCelSprite = &*monst->MType->Anims[MA_STAND].CelSpritesForDirections[md];
			}
		}
		return;
	}

	if (monst->_msquelch != 0) {
		int fx = monst->position.last.x;
		int fy = monst->position.last.y;
		Direction md = GetDirection(monst->position.tile, { fx, fy });
		M_CallWalk(i, md);
	}
}

void MAI_GoatBow(int i)
{
	MAI_Ranged(i, MIS_ARROW, false);
}

void MAI_Succ(int i)
{
	MAI_Ranged(i, MIS_FLARE, false);
}

void MAI_Lich(int i)
{
	MAI_Ranged(i, MIS_LICH, false);
}

void MAI_ArchLich(int i)
{
	MAI_Ranged(i, MIS_ARCHLICH, false);
}

void MAI_Psychorb(int i)
{
	MAI_Ranged(i, MIS_PSYCHORB, false);
}

void MAI_Necromorb(int i)
{
	MAI_Ranged(i, MIS_NECROMORB, false);
}

void MAI_AcidUniq(int i)
{
	MAI_Ranged(i, MIS_ACID, true);
}

void MAI_Firebat(int i)
{
	MAI_Ranged(i, MIS_FIREBOLT, false);
}

void MAI_Torchant(int i)
{
	MAI_Ranged(i, MIS_FIREBALL, false);
}

void MAI_Scav(int i)
{
	assurance((DWORD)i < MAXMONSTERS, i);

	MonsterStruct *monst = &Monsters[i];
	if (Monsters[i]._mmode != MM_STAND)
		return;
	if (monst->_mhitpoints < (monst->_mmaxhp / 2) && monst->_mgoal != MGOAL_HEALING) {
		if (monst->leaderflag != 0) {
			Monsters[monst->leader].packsize--;
			monst->leaderflag = 0;
		}
		monst->_mgoal = MGOAL_HEALING;
		monst->_mgoalvar3 = 10;
	}
	if (monst->_mgoal == MGOAL_HEALING && monst->_mgoalvar3 != 0) {
		monst->_mgoalvar3--;
		if (dDead[monst->position.tile.x][monst->position.tile.y] != 0) {
			M_StartEat(i);
			if ((monst->_mFlags & MFLAG_NOHEAL) == 0) {
				if (gbIsHellfire) {
					int mMaxHP = monst->_mmaxhp; // BUGFIX use _mmaxhp or we loose health when difficulty isn't normal (fixed)
					monst->_mhitpoints += mMaxHP / 8;
					if (monst->_mhitpoints > monst->_mmaxhp)
						monst->_mhitpoints = monst->_mmaxhp;
					if (monst->_mgoalvar3 <= 0 || monst->_mhitpoints == monst->_mmaxhp)
						dDead[monst->position.tile.x][monst->position.tile.y] = 0;
				} else {
					monst->_mhitpoints += 64;
				}
			}
			int targetHealth = monst->_mmaxhp;
			if (!gbIsHellfire)
				targetHealth = (monst->_mmaxhp / 2) + (monst->_mmaxhp / 4);
			if (monst->_mhitpoints >= targetHealth) {
				monst->_mgoal = MGOAL_NORMAL;
				monst->_mgoalvar1 = 0;
				monst->_mgoalvar2 = 0;
			}
		} else {
			if (monst->_mgoalvar1 == 0) {
				bool done = false;
				int x;
				int y;
				if (GenerateRnd(2) != 0) {
					for (y = -4; y <= 4 && !done; y++) {
						for (x = -4; x <= 4 && !done; x++) {
							// BUGFIX: incorrect check of offset against limits of the dungeon
							if (y < 0 || y >= MAXDUNY || x < 0 || x >= MAXDUNX)
								continue;
							done = dDead[monst->position.tile.x + x][monst->position.tile.y + y] != 0
							    && LineClearSolid(
							        monst->position.tile,
							        monst->position.tile + Displacement { x, y });
						}
					}
					x--;
					y--;
				} else {
					for (y = 4; y >= -4 && !done; y--) {
						for (x = 4; x >= -4 && !done; x--) {
							// BUGFIX: incorrect check of offset against limits of the dungeon
							if (y < 0 || y >= MAXDUNY || x < 0 || x >= MAXDUNX)
								continue;
							done = dDead[monst->position.tile.x + x][monst->position.tile.y + y] != 0
							    && LineClearSolid(
							        monst->position.tile,
							        monst->position.tile + Displacement { x, y });
						}
					}
					x++;
					y++;
				}
				if (done) {
					monst->_mgoalvar1 = x + monst->position.tile.x + 1;
					monst->_mgoalvar2 = y + monst->position.tile.y + 1;
				}
			}
			if (monst->_mgoalvar1 != 0) {
				int x = monst->_mgoalvar1 - 1;
				int y = monst->_mgoalvar2 - 1;
				monst->_mdir = GetDirection(monst->position.tile, { x, y });
				M_CallWalk(i, monst->_mdir);
			}
		}
	}

	if (monst->_mmode == MM_STAND)
		MAI_SkelSd(i);
}

void MAI_Garg(int i)
{
	assurance((DWORD)i < MAXMONSTERS, i);

	MonsterStruct *monst = &Monsters[i];
	int dx = monst->position.tile.x - monst->position.last.x;
	int dy = monst->position.tile.y - monst->position.last.y;
	Direction md = M_GetDir(i);
	if (monst->_msquelch != 0 && (monst->_mFlags & MFLAG_ALLOW_SPECIAL) != 0) {
		M_Enemy(i);
		int mx = monst->position.tile.x - monst->enemyPosition.x;
		int my = monst->position.tile.y - monst->enemyPosition.y;
		if (abs(mx) < monst->_mint + 2 && abs(my) < monst->_mint + 2) {
			monst->_mFlags &= ~MFLAG_ALLOW_SPECIAL;
		}
		return;
	}

	if (monst->_mmode != MM_STAND || monst->_msquelch == 0) {
		return;
	}

	if (monst->_mhitpoints < (monst->_mmaxhp / 2))
		if ((monst->_mFlags & MFLAG_NOHEAL) == 0)
			monst->_mgoal = MGOAL_RETREAT;
	if (monst->_mgoal == MGOAL_RETREAT) {
		if (abs(dx) >= monst->_mint + 2 || abs(dy) >= monst->_mint + 2) {
			monst->_mgoal = MGOAL_NORMAL;
			M_StartHeal(i);
		} else if (!M_CallWalk(i, opposite[md])) {
			monst->_mgoal = MGOAL_NORMAL;
		}
	}
	MAI_Round(i, false);
}

void MAI_RoundRanged(int i, missile_id missileType, bool checkdoors, int dam, int lessmissiles)
{
	assurance((DWORD)i < MAXMONSTERS, i);
	MonsterStruct *monst = &Monsters[i];
	if (monst->_mmode != MM_STAND || monst->_msquelch == 0) {
		return;
	}

	int fx = monst->enemyPosition.x;
	int fy = monst->enemyPosition.y;
	int mx = monst->position.tile.x - fx;
	int my = monst->position.tile.y - fy;
	Direction md = GetDirection(monst->position.tile, monst->position.last);
	if (checkdoors && monst->_msquelch < UINT8_MAX)
		MonstCheckDoors(i);
	int v = GenerateRnd(10000);
	int dist = std::max(abs(mx), abs(my));
	if (dist >= 2 && monst->_msquelch == UINT8_MAX && dTransVal[monst->position.tile.x][monst->position.tile.y] == dTransVal[fx][fy]) {
		if (monst->_mgoal == MGOAL_MOVE || (dist >= 3 && GenerateRnd(4 << lessmissiles) == 0)) {
			if (monst->_mgoal != MGOAL_MOVE) {
				monst->_mgoalvar1 = 0;
				monst->_mgoalvar2 = GenerateRnd(2);
			}
			monst->_mgoal = MGOAL_MOVE;
			if (monst->_mgoalvar1++ >= 2 * dist && DirOK(i, md)) {
				monst->_mgoal = MGOAL_NORMAL;
			} else if (v < (500 * (monst->_mint + 1) >> lessmissiles)
			    && (LineClearMissile(monst->position.tile, { fx, fy }))) {
				M_StartRSpAttack(i, missileType, dam);
			} else {
				M_RoundWalk(i, md, &monst->_mgoalvar2);
			}
		}
	} else {
		monst->_mgoal = MGOAL_NORMAL;
	}
	if (monst->_mgoal == MGOAL_NORMAL) {
		if (((dist >= 3 && v < ((500 * (monst->_mint + 2)) >> lessmissiles))
		        || v < ((500 * (monst->_mint + 1)) >> lessmissiles))
		    && LineClearMissile(monst->position.tile, { fx, fy })) {
			M_StartRSpAttack(i, missileType, dam);
		} else if (dist >= 2) {
			v = GenerateRnd(100);
			if (v < 1000 * (monst->_mint + 5)
			    || ((monst->_mVar1 == MM_WALK || monst->_mVar1 == MM_WALK2 || monst->_mVar1 == MM_WALK3) && monst->_mVar2 == 0 && v < 1000 * (monst->_mint + 8))) {
				M_CallWalk(i, md);
			}
		} else if (v < 1000 * (monst->_mint + 6)) {
			monst->_mdir = md;
			M_StartAttack(i);
		}
	}
	if (monst->_mmode == MM_STAND) {
		M_StartDelay(i, GenerateRnd(10) + 5);
	}
}

void MAI_Magma(int i)
{
	MAI_RoundRanged(i, MIS_MAGMABALL, true, 4, 0);
}

void MAI_Storm(int i)
{
	MAI_RoundRanged(i, MIS_LIGHTCTRL2, true, 4, 0);
}

void MAI_BoneDemon(int i)
{
	MAI_RoundRanged(i, MIS_BONEDEMON, true, 4, 0);
}

void MAI_Acid(int i)
{
	MAI_RoundRanged(i, MIS_ACID, false, 4, 1);
}

void MAI_Diablo(int i)
{
	MAI_RoundRanged(i, MIS_DIABAPOCA, false, 40, 0);
}

void MAI_RR2(int i, missile_id mistype, int dam)
{
	assurance((DWORD)i < MAXMONSTERS, i);

	MonsterStruct *monst = &Monsters[i];
	int mx = monst->position.tile.x - monst->enemyPosition.x;
	int my = monst->position.tile.y - monst->enemyPosition.y;
	if (abs(mx) >= 5 || abs(my) >= 5) {
		MAI_SkelSd(i);
		return;
	}

	if (monst->_mmode != MM_STAND || monst->_msquelch == 0) {
		return;
	}

	int fx = monst->enemyPosition.x;
	int fy = monst->enemyPosition.y;
	mx = monst->position.tile.x - fx;
	my = monst->position.tile.y - fy;
	Direction md = GetDirection(monst->position.tile, monst->position.last);
	if (monst->_msquelch < UINT8_MAX)
		MonstCheckDoors(i);
	int v = GenerateRnd(100);
	int dist = std::max(abs(mx), abs(my));
	if (dist >= 2 && monst->_msquelch == UINT8_MAX && dTransVal[monst->position.tile.x][monst->position.tile.y] == dTransVal[fx][fy]) {
		if (monst->_mgoal == MGOAL_MOVE || dist >= 3) {
			if (monst->_mgoal != MGOAL_MOVE) {
				monst->_mgoalvar1 = 0;
				monst->_mgoalvar2 = GenerateRnd(2);
			}
			monst->_mgoal = MGOAL_MOVE;
			monst->_mgoalvar3 = 4;
			if (monst->_mgoalvar1++ < 2 * dist || !DirOK(i, md)) {
				if (v < 5 * (monst->_mint + 16))
					M_RoundWalk(i, md, &monst->_mgoalvar2);
			} else
				monst->_mgoal = MGOAL_NORMAL;
		}
	} else {
		monst->_mgoal = MGOAL_NORMAL;
	}
	if (monst->_mgoal == MGOAL_NORMAL) {
		if (((dist >= 3 && v < 5 * (monst->_mint + 2)) || v < 5 * (monst->_mint + 1) || monst->_mgoalvar3 == 4) && LineClearMissile(monst->position.tile, { fx, fy })) {
			M_StartRSpAttack(i, mistype, dam);
		} else if (dist >= 2) {
			v = GenerateRnd(100);
			if (v < 2 * (5 * monst->_mint + 25)
			    || ((monst->_mVar1 == MM_WALK || monst->_mVar1 == MM_WALK2 || monst->_mVar1 == MM_WALK3)
			        && monst->_mVar2 == 0
			        && v < 2 * (5 * monst->_mint + 40))) {
				M_CallWalk(i, md);
			}
		} else {
			if (GenerateRnd(100) < 10 * (monst->_mint + 4)) {
				monst->_mdir = md;
				if (GenerateRnd(2) != 0)
					M_StartAttack(i);
				else
					M_StartRSpAttack(i, mistype, dam);
			}
		}
		monst->_mgoalvar3 = 1;
	}
	if (monst->_mmode == MM_STAND) {
		M_StartDelay(i, GenerateRnd(10) + 5);
	}
}

void MAI_Mega(int i)
{
	MAI_RR2(i, MIS_FLAMEC, 0);
}

void MAI_Golum(int i)
{
	assurance((DWORD)i < MAXMONSTERS, i);

	MonsterStruct *monst = &Monsters[i];
	if (monst->position.tile.x == 1 && monst->position.tile.y == 0) {
		return;
	}

	if (monst->_mmode == MM_DEATH
	    || monst->_mmode == MM_SPSTAND
	    || (monst->_mmode >= MM_WALK && monst->_mmode <= MM_WALK3)) {
		return;
	}

	if ((monst->_mFlags & MFLAG_TARGETS_MONSTER) == 0)
		M_Enemy(i);

	bool haveEnemy = (Monsters[i]._mFlags & MFLAG_NO_ENEMY) == 0;

	if (monst->_mmode == MM_ATTACK) {
		return;
	}

	int menemy = Monsters[i]._menemy;

	int mex = Monsters[i].position.tile.x - Monsters[menemy].position.future.x;
	int mey = Monsters[i].position.tile.y - Monsters[menemy].position.future.y;
	Direction md = GetDirection(Monsters[i].position.tile, Monsters[menemy].position.tile);
	Monsters[i]._mdir = md;
	if (abs(mex) < 2 && abs(mey) < 2 && haveEnemy) {
		menemy = Monsters[i]._menemy;
		Monsters[i].enemyPosition = Monsters[menemy].position.tile;
		if (Monsters[menemy]._msquelch == 0) {
			Monsters[menemy]._msquelch = UINT8_MAX;
			Monsters[Monsters[i]._menemy].position.last = Monsters[i].position.tile;
			for (int j = 0; j < 5; j++) {
				for (int k = 0; k < 5; k++) {
					menemy = dMonster[Monsters[i].position.tile.x + k - 2][Monsters[i].position.tile.y + j - 2]; // BUGFIX: Check if indexes are between 0 and 112
					if (menemy > 0)
						Monsters[menemy - 1]._msquelch = UINT8_MAX; // BUGFIX: should be `Monsters[_menemy-1]`, not Monsters[_menemy]. (fixed)
				}
			}
		}
		M_StartAttack(i);
		return;
	}

	if (haveEnemy && MAI_Path(i))
		return;

	Monsters[i]._pathcount++;
	if (Monsters[i]._pathcount > 8)
		Monsters[i]._pathcount = 5;

	bool ok = M_CallWalk(i, plr[i]._pdir);
	if (ok)
		return;

	md = left[md];
	for (int j = 0; j < 8 && !ok; j++) {
		md = right[md];
		ok = DirOK(i, md);
	}
	if (ok)
		M_WalkDir(i, md);
}

void MAI_SkelKing(int i)
{
	assurance((DWORD)i < MAXMONSTERS, i);
	MonsterStruct *monst = &Monsters[i];
	if (monst->_mmode != MM_STAND || monst->_msquelch == 0) {
		return;
	}

	int fx = monst->enemyPosition.x;
	int fy = monst->enemyPosition.y;
	int mx = monst->position.tile.x - fx;
	int my = monst->position.tile.y - fy;
	Direction md = GetDirection(monst->position.tile, monst->position.last);
	if (monst->_msquelch < UINT8_MAX)
		MonstCheckDoors(i);
	int v = GenerateRnd(100);
	int dist = std::max(abs(mx), abs(my));
	if (dist >= 2 && monst->_msquelch == UINT8_MAX && dTransVal[monst->position.tile.x][monst->position.tile.y] == dTransVal[fx][fy]) {
		if (monst->_mgoal == MGOAL_MOVE || ((abs(mx) >= 3 || abs(my) >= 3) && GenerateRnd(4) == 0)) {
			if (monst->_mgoal != MGOAL_MOVE) {
				monst->_mgoalvar1 = 0;
				monst->_mgoalvar2 = GenerateRnd(2);
			}
			monst->_mgoal = MGOAL_MOVE;
			if ((monst->_mgoalvar1++ >= 2 * dist && DirOK(i, md)) || dTransVal[monst->position.tile.x][monst->position.tile.y] != dTransVal[fx][fy]) {
				monst->_mgoal = MGOAL_NORMAL;
			} else if (!M_RoundWalk(i, md, &monst->_mgoalvar2)) {
				M_StartDelay(i, GenerateRnd(10) + 10);
			}
		}
	} else {
		monst->_mgoal = MGOAL_NORMAL;
	}
	if (monst->_mgoal == MGOAL_NORMAL) {
		if (!gbIsMultiplayer
		    && ((dist >= 3 && v < 4 * monst->_mint + 35) || v < 6)
		    && LineClearMissile(monst->position.tile, { fx, fy })) {
			Point newPosition = monst->position.tile + md;
			if (PosOkMonst(i, newPosition) && ActiveMonsterCount < MAXMONSTERS) {
				M_SpawnSkel(newPosition, md);
				M_StartSpStand(i, md);
			}
		} else {
			if (dist >= 2) {
				v = GenerateRnd(100);
				if (v >= monst->_mint + 25
				    && ((monst->_mVar1 != MM_WALK && monst->_mVar1 != MM_WALK2 && monst->_mVar1 != MM_WALK3) || monst->_mVar2 != 0 || (v >= monst->_mint + 75))) {
					M_StartDelay(i, GenerateRnd(10) + 10);
				} else {
					M_CallWalk(i, md);
				}
			} else if (v < monst->_mint + 20) {
				monst->_mdir = md;
				M_StartAttack(i);
			}
		}
	}

	monst->CheckStandAnimationIsLoaded(md);
}

void MAI_Rhino(int i)
{
	assurance((DWORD)i < MAXMONSTERS, i);
	MonsterStruct *monst = &Monsters[i];
	if (monst->_mmode != MM_STAND || monst->_msquelch == 0) {
		return;
	}

	int fx = monst->enemyPosition.x;
	int fy = monst->enemyPosition.y;
	int mx = monst->position.tile.x - fx;
	int my = monst->position.tile.y - fy;
	Direction md = GetDirection(monst->position.tile, monst->position.last);
	if (monst->_msquelch < UINT8_MAX)
		MonstCheckDoors(i);
	int v = GenerateRnd(100);
	int dist = std::max(abs(mx), abs(my));
	if (dist >= 2) {
		if (monst->_mgoal == MGOAL_MOVE || (dist >= 5 && GenerateRnd(4) != 0)) {
			if (monst->_mgoal != MGOAL_MOVE) {
				monst->_mgoalvar1 = 0;
				monst->_mgoalvar2 = GenerateRnd(2);
			}
			monst->_mgoal = MGOAL_MOVE;
			if (monst->_mgoalvar1++ >= 2 * dist || dTransVal[monst->position.tile.x][monst->position.tile.y] != dTransVal[fx][fy]) {
				monst->_mgoal = MGOAL_NORMAL;
			} else if (!M_RoundWalk(i, md, &monst->_mgoalvar2)) {
				M_StartDelay(i, GenerateRnd(10) + 10);
			}
		}
	} else {
		monst->_mgoal = MGOAL_NORMAL;
	}
	if (monst->_mgoal == MGOAL_NORMAL) {
		if (dist >= 5
		    && v < 2 * monst->_mint + 43
		    && LineClear(PosOkMonst, i, monst->position.tile, { fx, fy })) {
			if (AddMissile(monst->position.tile, { fx, fy }, md, MIS_RHINO, monst->_menemy, i, 0, 0) != -1) {
				if (monst->MData->snd_special)
					PlayEffect(i, 3);
				dMonster[monst->position.tile.x][monst->position.tile.y] = -(i + 1);
				monst->_mmode = MM_CHARGE;
			}
		} else {
			if (dist >= 2) {
				v = GenerateRnd(100);
				if (v >= 2 * monst->_mint + 33
				    && ((monst->_mVar1 != MM_WALK && monst->_mVar1 != MM_WALK2 && monst->_mVar1 != MM_WALK3)
				        || monst->_mVar2 != 0
				        || v >= 2 * monst->_mint + 83)) {
					M_StartDelay(i, GenerateRnd(10) + 10);
				} else {
					M_CallWalk(i, md);
				}
			} else if (v < 2 * monst->_mint + 28) {
				monst->_mdir = md;
				M_StartAttack(i);
			}
		}
	}

	monst->CheckStandAnimationIsLoaded(monst->_mdir);
}

void MAI_HorkDemon(int i)
{
	if ((DWORD)i >= MAXMONSTERS) {
		return;
	}

	MonsterStruct *monst = &Monsters[i];
	if (monst->_mmode != MM_STAND || monst->_msquelch == 0) {
		return;
	}

	int fx = monst->enemyPosition.x;
	int fy = monst->enemyPosition.y;
	int mx = monst->position.tile.x - fx;
	int my = monst->position.tile.y - fy;
	Direction md = GetDirection(monst->position.tile, monst->position.last);

	if (monst->_msquelch < 255) {
		MonstCheckDoors(i);
	}

	int v = GenerateRnd(100);

	if (abs(mx) < 2 && abs(my) < 2) {
		monst->_mgoal = MGOAL_NORMAL;
	} else if (monst->_mgoal == 4 || ((abs(mx) >= 5 || abs(my) >= 5) && GenerateRnd(4) != 0)) {
		if (monst->_mgoal != 4) {
			monst->_mgoalvar1 = 0;
			monst->_mgoalvar2 = GenerateRnd(2);
		}
		monst->_mgoal = MGOAL_MOVE;
		int dist = std::max(abs(mx), abs(my));
		if (monst->_mgoalvar1++ >= 2 * dist || dTransVal[monst->position.tile.x][monst->position.tile.y] != dTransVal[fx][fy]) {
			monst->_mgoal = MGOAL_NORMAL;
		} else if (!M_RoundWalk(i, md, &monst->_mgoalvar2)) {
			M_StartDelay(i, GenerateRnd(10) + 10);
		}
	}

	if (monst->_mgoal == 1) {
		if ((abs(mx) >= 3 || abs(my) >= 3) && v < 2 * monst->_mint + 43) {
			Point position = monst->position.tile + monst->_mdir;
			if (PosOkMonst(i, position) && ActiveMonsterCount < MAXMONSTERS) {
				M_StartRSpAttack(i, MIS_HORKDMN, 0);
			}
		} else if (abs(mx) < 2 && abs(my) < 2) {
			if (v < 2 * monst->_mint + 28) {
				monst->_mdir = md;
				M_StartAttack(i);
			}
		} else {
			v = GenerateRnd(100);
			if (v < 2 * monst->_mint + 33
			    || ((monst->_mVar1 == MM_WALK || monst->_mVar1 == MM_WALK2 || monst->_mVar1 == MM_WALK3) && monst->_mVar2 == 0 && v < 2 * monst->_mint + 83)) {
				M_CallWalk(i, md);
			} else {
				M_StartDelay(i, GenerateRnd(10) + 10);
			}
		}
	}

	monst->CheckStandAnimationIsLoaded(monst->_mdir);
}

void MAI_Counselor(int i)
{
	assurance((DWORD)i < MAXMONSTERS, i);

	MonsterStruct *monst = &Monsters[i];
	if (monst->_mmode != MM_STAND || monst->_msquelch == 0) {
		return;
	}
	int fx = monst->enemyPosition.x;
	int fy = monst->enemyPosition.y;
	int mx = monst->position.tile.x - fx;
	int my = monst->position.tile.y - fy;
	Direction md = GetDirection(monst->position.tile, monst->position.last);
	if (monst->_msquelch < UINT8_MAX)
		MonstCheckDoors(i);
	int v = GenerateRnd(100);
	if (monst->_mgoal == MGOAL_RETREAT) {
		if (monst->_mgoalvar1++ <= 3)
			M_CallWalk(i, opposite[md]);
		else {
			monst->_mgoal = MGOAL_NORMAL;
			M_StartFadein(i, md, true);
		}
	} else if (monst->_mgoal == MGOAL_MOVE) {
		int dist = std::max(abs(mx), abs(my));
		if (dist >= 2 && monst->_msquelch == UINT8_MAX && dTransVal[monst->position.tile.x][monst->position.tile.y] == dTransVal[fx][fy]) {
			if (monst->_mgoalvar1++ < 2 * dist || !DirOK(i, md)) {
				M_RoundWalk(i, md, &monst->_mgoalvar2);
			} else {
				monst->_mgoal = MGOAL_NORMAL;
				M_StartFadein(i, md, true);
			}
		}
	} else if (monst->_mgoal == MGOAL_NORMAL) {
		if (abs(mx) >= 2 || abs(my) >= 2) {
			if (v < 5 * (monst->_mint + 10) && LineClearMissile(monst->position.tile, { fx, fy })) {
				constexpr missile_id MissileTypes[4] = { MIS_FIREBOLT, MIS_CBOLT, MIS_LIGHTCTRL, MIS_FIREBALL };
				M_StartRAttack(i, MissileTypes[monst->_mint], monst->mMinDamage + GenerateRnd(monst->mMaxDamage - monst->mMinDamage + 1));
			} else if (GenerateRnd(100) < 30) {
				monst->_mgoal = MGOAL_MOVE;
				monst->_mgoalvar1 = 0;
				M_StartFadeout(i, md, false);
			} else
				M_StartDelay(i, GenerateRnd(10) + 2 * (5 - monst->_mint));
		} else {
			monst->_mdir = md;
			if (monst->_mhitpoints < (monst->_mmaxhp / 2)) {
				monst->_mgoal = MGOAL_RETREAT;
				monst->_mgoalvar1 = 0;
				M_StartFadeout(i, md, false);
			} else if (monst->_mVar1 == MM_DELAY
			    || GenerateRnd(100) < 2 * monst->_mint + 20) {
				M_StartRAttack(i, MIS_NULL, 0);
				AddMissile(monst->position.tile, { 0, 0 }, monst->_mdir, MIS_FLASH, TARGET_PLAYERS, i, 4, 0);
				AddMissile(monst->position.tile, { 0, 0 }, monst->_mdir, MIS_FLASH2, TARGET_PLAYERS, i, 4, 0);
			} else
				M_StartDelay(i, GenerateRnd(10) + 2 * (5 - monst->_mint));
		}
	}
	if (monst->_mmode == MM_STAND) {
		M_StartDelay(i, GenerateRnd(10) + 5);
	}
}

void MAI_Garbud(int i)
{
	assurance((DWORD)i < MAXMONSTERS, i);

	MonsterStruct *monst = &Monsters[i];
	if (monst->_mmode != MM_STAND) {
		return;
	}

	int mx = monst->position.tile.x;
	int my = monst->position.tile.y;
	Direction md = M_GetDir(i);

	if (monst->mtalkmsg >= TEXT_GARBUD1
	    && monst->mtalkmsg <= TEXT_GARBUD3
	    && (dFlags[mx][my] & BFLAG_VISIBLE) == 0
	    && monst->_mgoal == MGOAL_TALKING) {
		monst->_mgoal = MGOAL_INQUIRING;
		switch (monst->mtalkmsg) {
		case TEXT_GARBUD1:
			monst->mtalkmsg = TEXT_GARBUD2;
			break;
		case TEXT_GARBUD2:
			monst->mtalkmsg = TEXT_GARBUD3;
			break;
		case TEXT_GARBUD3:
			monst->mtalkmsg = TEXT_GARBUD4;
			break;
		default:
			break;
		}
	}

	if ((dFlags[mx][my] & BFLAG_VISIBLE) != 0) {
		if (monst->mtalkmsg == TEXT_GARBUD4) {
			if (!effect_is_playing(USFX_GARBUD4) && monst->_mgoal == MGOAL_TALKING) {
				monst->_mgoal = MGOAL_NORMAL;
				monst->_msquelch = UINT8_MAX;
				monst->mtalkmsg = TEXT_NONE;
			}
		}
	}

	if (monst->_mgoal == MGOAL_NORMAL || monst->_mgoal == MGOAL_MOVE)
		MAI_Round(i, true);

	Monsters[i]._mdir = md;

	monst->CheckStandAnimationIsLoaded(md);
}

void MAI_Zhar(int i)
{
	assurance((DWORD)i < MAXMONSTERS, i);

	MonsterStruct *monst = &Monsters[i];
	if (Monsters[i]._mmode != MM_STAND) {
		return;
	}

	int mx = monst->position.tile.x;
	int my = monst->position.tile.y;
	Direction md = M_GetDir(i);
	if (monst->mtalkmsg == TEXT_ZHAR1 && (dFlags[mx][my] & BFLAG_VISIBLE) == 0 && monst->_mgoal == MGOAL_TALKING) {
		monst->mtalkmsg = TEXT_ZHAR2;
		monst->_mgoal = MGOAL_INQUIRING;
	}

	if ((dFlags[mx][my] & BFLAG_VISIBLE) != 0) {
		if (monst->mtalkmsg == TEXT_ZHAR2) {
			if (!effect_is_playing(USFX_ZHAR2) && monst->_mgoal == MGOAL_TALKING) {
				monst->_msquelch = UINT8_MAX;
				monst->mtalkmsg = TEXT_NONE;
				monst->_mgoal = MGOAL_NORMAL;
			}
		}
	}

	if (monst->_mgoal == MGOAL_NORMAL || monst->_mgoal == MGOAL_RETREAT || monst->_mgoal == MGOAL_MOVE)
		MAI_Counselor(i);

	monst->_mdir = md;

	monst->CheckStandAnimationIsLoaded(md);
}

void MAI_SnotSpil(int i)
{
	assurance((DWORD)i < MAXMONSTERS, i);

	MonsterStruct *monst = &Monsters[i];
	if (Monsters[i]._mmode != MM_STAND) {
		return;
	}

	int mx = monst->position.tile.x;
	int my = monst->position.tile.y;
	Direction md = M_GetDir(i);

	if (monst->mtalkmsg == TEXT_BANNER10 && (dFlags[mx][my] & BFLAG_VISIBLE) == 0 && monst->_mgoal == MGOAL_TALKING) {
		monst->mtalkmsg = TEXT_BANNER11;
		monst->_mgoal = MGOAL_INQUIRING;
	}

	if (monst->mtalkmsg == TEXT_BANNER11 && quests[Q_LTBANNER]._qvar1 == 3) {
		monst->mtalkmsg = TEXT_NONE;
		monst->_mgoal = MGOAL_NORMAL;
	}

	if ((dFlags[mx][my] & BFLAG_VISIBLE) != 0) {
		if (monst->mtalkmsg == TEXT_BANNER12) {
			if (!effect_is_playing(USFX_SNOT3) && monst->_mgoal == MGOAL_TALKING) {
				ObjChangeMap(setpc_x, setpc_y, setpc_x + setpc_w + 1, setpc_y + setpc_h + 1);
				quests[Q_LTBANNER]._qvar1 = 3;
				RedoPlayerVision();
				monst->_msquelch = UINT8_MAX;
				monst->mtalkmsg = TEXT_NONE;
				monst->_mgoal = MGOAL_NORMAL;
			}
		}
		if (quests[Q_LTBANNER]._qvar1 == 3) {
			if (monst->_mgoal == MGOAL_NORMAL || monst->_mgoal == MGOAL_ATTACK2)
				MAI_Fallen(i);
		}
	}

	monst->_mdir = md;

	monst->CheckStandAnimationIsLoaded(md);
}

void MAI_Lazurus(int i)
{
	assurance((DWORD)i < MAXMONSTERS, i);

	MonsterStruct *monst = &Monsters[i];
	if (Monsters[i]._mmode != MM_STAND) {
		return;
	}

	int mx = monst->position.tile.x;
	int my = monst->position.tile.y;
	Direction md = M_GetDir(i);
	if ((dFlags[mx][my] & BFLAG_VISIBLE) != 0) {
		if (!gbIsMultiplayer) {
			if (monst->mtalkmsg == TEXT_VILE13 && monst->_mgoal == MGOAL_INQUIRING && plr[myplr].position.tile.x == 35 && plr[myplr].position.tile.y == 46) {
				PlayInGameMovie("gendata\\fprst3.smk");
				monst->_mmode = MM_TALK;
				quests[Q_BETRAYER]._qvar1 = 5;
			}

			if (monst->mtalkmsg == TEXT_VILE13 && !effect_is_playing(USFX_LAZ1) && monst->_mgoal == MGOAL_TALKING) {
				ObjChangeMapResync(1, 18, 20, 24);
				RedoPlayerVision();
				quests[Q_BETRAYER]._qvar1 = 6;
				monst->_mgoal = MGOAL_NORMAL;
				monst->_msquelch = UINT8_MAX;
				monst->mtalkmsg = TEXT_NONE;
			}
		}

		if (gbIsMultiplayer && monst->mtalkmsg == TEXT_VILE13 && monst->_mgoal == MGOAL_INQUIRING && quests[Q_BETRAYER]._qvar1 <= 3) {
			monst->_mmode = MM_TALK;
		}
	}

	if (monst->_mgoal == MGOAL_NORMAL || monst->_mgoal == MGOAL_RETREAT || monst->_mgoal == MGOAL_MOVE) {
		if (!gbIsMultiplayer && quests[Q_BETRAYER]._qvar1 == 4 && monst->mtalkmsg == TEXT_NONE) { // Fix save games affected by teleport bug
			ObjChangeMapResync(1, 18, 20, 24);
			RedoPlayerVision();
			quests[Q_BETRAYER]._qvar1 = 6;
		}
		monst->mtalkmsg = TEXT_NONE;
		MAI_Counselor(i);
	}

	monst->_mdir = md;

	monst->CheckStandAnimationIsLoaded(md);
}

void MAI_Lazhelp(int i)
{
	assurance((DWORD)i < MAXMONSTERS, i);
	if (Monsters[i]._mmode != MM_STAND)
		return;

	MonsterStruct *monst = &Monsters[i];
	int mx = monst->position.tile.x;
	int my = monst->position.tile.y;
	Direction md = M_GetDir(i);

	if ((dFlags[mx][my] & BFLAG_VISIBLE) != 0) {
		if (!gbIsMultiplayer) {
			if (quests[Q_BETRAYER]._qvar1 <= 5) {
				monst->_mgoal = MGOAL_INQUIRING;
			} else {
				monst->_mgoal = MGOAL_NORMAL;
				monst->mtalkmsg = TEXT_NONE;
			}
		} else
			monst->_mgoal = MGOAL_NORMAL;
	}
	if (monst->_mgoal == MGOAL_NORMAL)
		MAI_Succ(i);
	monst->_mdir = md;

	monst->CheckStandAnimationIsLoaded(md);
}

void MAI_Lachdanan(int i)
{
	assurance((DWORD)i < MAXMONSTERS, i);

	MonsterStruct *monst = &Monsters[i];
	if (Monsters[i]._mmode != MM_STAND) {
		return;
	}

	int mx = monst->position.tile.x;
	int my = monst->position.tile.y;
	Direction md = M_GetDir(i);

	if (monst->mtalkmsg == TEXT_VEIL9 && (dFlags[mx][my] & BFLAG_VISIBLE) == 0 && Monsters[i]._mgoal == MGOAL_TALKING) {
		monst->mtalkmsg = TEXT_VEIL10;
		Monsters[i]._mgoal = MGOAL_INQUIRING;
	}

	if ((dFlags[mx][my] & BFLAG_VISIBLE) != 0) {
		if (monst->mtalkmsg == TEXT_VEIL11) {
			if (!effect_is_playing(USFX_LACH3) && monst->_mgoal == MGOAL_TALKING) {
				monst->mtalkmsg = TEXT_NONE;
				quests[Q_VEIL]._qactive = QUEST_DONE;
				M_StartKill(i, -1);
			}
		}
	}

	monst->_mdir = md;

	monst->CheckStandAnimationIsLoaded(md);
}

void MAI_Warlord(int i)
{
	assurance((DWORD)i < MAXMONSTERS, i);

	MonsterStruct *monst = &Monsters[i];
	if (Monsters[i]._mmode != MM_STAND) {
		return;
	}

	int mx = monst->position.tile.x;
	int my = monst->position.tile.y;
	Direction md = M_GetDir(i);
	if ((dFlags[mx][my] & BFLAG_VISIBLE) != 0) {
		if (monst->mtalkmsg == TEXT_WARLRD9 && monst->_mgoal == MGOAL_INQUIRING)
			monst->_mmode = MM_TALK;
		if (monst->mtalkmsg == TEXT_WARLRD9 && !effect_is_playing(USFX_WARLRD1) && monst->_mgoal == MGOAL_TALKING) {
			monst->_msquelch = UINT8_MAX;
			monst->mtalkmsg = TEXT_NONE;
			monst->_mgoal = MGOAL_NORMAL;
		}
	}

	if (monst->_mgoal == MGOAL_NORMAL)
		MAI_SkelSd(i);

	monst->_mdir = md;

	monst->CheckStandAnimationIsLoaded(monst->_mdir);
}

void DeleteMonsterList()
{
	for (int i = 0; i < MAX_PLRS; i++) {
		if (Monsters[i]._mDelFlag) {
			Monsters[i].position.tile = { 1, 0 };
			Monsters[i].position.future = { 0, 0 };
			Monsters[i].position.old = { 0, 0 };
			Monsters[i]._mDelFlag = false;
		}
	}

	for (int i = MAX_PLRS; i < ActiveMonsterCount;) {
		if (Monsters[ActiveMonsters[i]]._mDelFlag) {
			DeleteMonster(i);
		} else {
			i++;
		}
	}
}

void ProcessMonsters()
{
	DeleteMonsterList();

	assert((DWORD)ActiveMonsterCount <= MAXMONSTERS);
	for (int i = 0; i < ActiveMonsterCount; i++) {
		int mi = ActiveMonsters[i];
		MonsterStruct *monst = &Monsters[mi];
		bool raflag = false;
		if (gbIsMultiplayer) {
			SetRndSeed(monst->_mAISeed);
			monst->_mAISeed = AdvanceRndSeed();
		}
		if ((Monsters[mi]._mFlags & MFLAG_NOHEAL) == 0 && monst->_mhitpoints < monst->_mmaxhp && monst->_mhitpoints >> 6 > 0) {
			if (monst->mLevel > 1) {
				monst->_mhitpoints += monst->mLevel / 2;
			} else {
				monst->_mhitpoints += monst->mLevel;
			}
		}
		int mx = monst->position.tile.x;
		int my = monst->position.tile.y;

		if ((dFlags[mx][my] & BFLAG_VISIBLE) != 0 && monst->_msquelch == 0) {
			if (monst->MType->mtype == MT_CLEAVER) {
				PlaySFX(USFX_CLEAVER);
			}
			if (monst->MType->mtype == MT_NAKRUL) {
				if (sgGameInitInfo.bCowQuest != 0) {
					PlaySFX(USFX_NAKRUL6);
				} else {
					if (IsUberRoomOpened)
						PlaySFX(USFX_NAKRUL4);
					else
						PlaySFX(USFX_NAKRUL5);
				}
			}
			if (monst->MType->mtype == MT_DEFILER)
				PlaySFX(USFX_DEFILER8);
			M_Enemy(mi);
		}

		int menemy;
		if ((monst->_mFlags & MFLAG_TARGETS_MONSTER) != 0) {
			menemy = monst->_menemy;
			assurance((DWORD)menemy < MAXMONSTERS, menemy);
			monst->position.last = Monsters[monst->_menemy].position.future;
			monst->enemyPosition = monst->position.last;
		} else {
			menemy = monst->_menemy;
			assurance((DWORD)menemy < MAX_PLRS, menemy);
			monst->enemyPosition = plr[monst->_menemy].position.future;
			if ((dFlags[mx][my] & BFLAG_VISIBLE) != 0) {
				monst->_msquelch = UINT8_MAX;
				monst->position.last = plr[monst->_menemy].position.future;
			} else if (monst->_msquelch != 0 && monst->MType->mtype != MT_DIABLO) { /// BUGFIX: change '_mAi' to 'MType->mtype'
				monst->_msquelch--;
			}
		}
		do {
			if ((monst->_mFlags & MFLAG_SEARCH) == 0 || !MAI_Path(mi)) {
				AiProc[monst->_mAi](mi);
			}
			switch (monst->_mmode) {
			case MM_STAND:
				raflag = M_DoStand(mi);
				break;
			case MM_WALK:
			case MM_WALK2:
			case MM_WALK3:
				raflag = M_DoWalk(mi, monst->_mmode);
				break;
			case MM_ATTACK:
				raflag = M_DoAttack(mi);
				break;
			case MM_GOTHIT:
				raflag = M_DoGotHit(mi);
				break;
			case MM_DEATH:
				raflag = M_DoDeath(mi);
				break;
			case MM_SATTACK:
				raflag = M_DoSAttack(mi);
				break;
			case MM_FADEIN:
				raflag = M_DoFadein(mi);
				break;
			case MM_FADEOUT:
				raflag = M_DoFadeout(mi);
				break;
			case MM_RATTACK:
				raflag = M_DoRAttack(mi);
				break;
			case MM_SPSTAND:
				raflag = M_DoSpStand(mi);
				break;
			case MM_RSPATTACK:
				raflag = M_DoRSpAttack(mi);
				break;
			case MM_DELAY:
				raflag = M_DoDelay(mi);
				break;
			case MM_CHARGE:
				raflag = false;
				break;
			case MM_STONE:
				raflag = M_DoStone(mi);
				break;
			case MM_HEAL:
				raflag = M_DoHeal(mi);
				break;
			case MM_TALK:
				raflag = M_DoTalk(mi);
				break;
			}
			if (raflag) {
				GroupUnity(mi);
			}
		} while (raflag);
		if (monst->_mmode != MM_STONE) {
			monst->AnimInfo.ProcessAnimation((monst->_mFlags & MFLAG_LOCK_ANIMATION) != 0, (monst->_mFlags & MFLAG_ALLOW_SPECIAL) != 0);
		}
	}

	DeleteMonsterList();
}

void FreeMonsters()
{
	for (int i = 0; i < LevelMonsterTypeCount; i++) {
		int mtype = LevelMonsterTypes[i].mtype;
		for (int j = 0; j < 6; j++) {
			if (animletter[j] != 's' || MonsterData[mtype].has_special) {
				LevelMonsterTypes[i].Anims[j].CMem = nullptr;
			}
		}
	}

	FreeMissiles2();
}

bool DirOK(int i, Direction mdir)
{
	commitment((DWORD)i < MAXMONSTERS, i);
	Point position = Monsters[i].position.tile;
	Point futurePosition = position + mdir;
	if (futurePosition.y < 0 || futurePosition.y >= MAXDUNY || futurePosition.x < 0 || futurePosition.x >= MAXDUNX || !PosOkMonst(i, futurePosition))
		return false;
	if (mdir == DIR_E) {
		if (SolidLoc(position + DIR_SE) || (dFlags[position.x + 1][position.y] & BFLAG_MONSTLR) != 0)
			return false;
	} else if (mdir == DIR_W) {
		if (SolidLoc(position + DIR_SW) || (dFlags[position.x][position.y + 1] & BFLAG_MONSTLR) != 0)
			return false;
	} else if (mdir == DIR_N) {
		if (SolidLoc(position + DIR_NE) || SolidLoc(position + DIR_NW))
			return false;
	} else if (mdir == DIR_S)
		if (SolidLoc(position + DIR_SW) || SolidLoc(position + DIR_SE))
			return false;
	if (Monsters[i].leaderflag == 1) {
		return futurePosition.WalkingDistance(Monsters[Monsters[i].leader].position.future) < 4;
	}
	if (Monsters[i]._uniqtype == 0 || (UniqMonst[Monsters[i]._uniqtype - 1].mUnqAttr & 2) == 0)
		return true;
	int mcount = 0;
	for (int x = futurePosition.x - 3; x <= futurePosition.x + 3; x++) {
		for (int y = futurePosition.y - 3; y <= futurePosition.y + 3; y++) {
			if (y < 0 || y >= MAXDUNY || x < 0 || x >= MAXDUNX)
				continue;
			int mi = dMonster[x][y];
			if (mi < 0)
				mi = -mi;
			if (mi != 0)
				mi--;
			// BUGFIX: should only run pack member check if mi was non-zero prior to executing the body of the above if-statement.
			if (Monsters[mi].leaderflag == 1
			    && Monsters[mi].leader == i
			    && Monsters[mi].position.future == Point { x, y }) {
				mcount++;
			}
		}
	}
	return mcount == Monsters[i].packsize;
}

bool PosOkMissile(int /*entity*/, Point position)
{
	return !nMissileTable[dPiece[position.x][position.y]] && (dFlags[position.x][position.y] & BFLAG_MONSTLR) == 0;
}

bool CheckNoSolid(int /*entity*/, Point position)
{
	return !nSolidTable[dPiece[position.x][position.y]];
}

bool LineClearSolid(Point startPoint, Point endPoint)
{
	return LineClear(CheckNoSolid, 0, startPoint, endPoint);
}

bool LineClearMissile(Point startPoint, Point endPoint)
{
	return LineClear(PosOkMissile, 0, startPoint, endPoint);
}

bool LineClear(bool (*clear)(int, Point), int entity, Point startPoint, Point endPoint)
{
	Point position = startPoint;

	int dx = endPoint.x - position.x;
	int dy = endPoint.y - position.y;
	if (abs(dx) > abs(dy)) {
		if (dx < 0) {
			std::swap(position, endPoint);
			dx = -dx;
			dy = -dy;
		}
		int d;
		int yincD;
		int dincD;
		int dincH;
		if (dy > 0) {
			d = 2 * dy - dx;
			dincD = 2 * dy;
			dincH = 2 * (dy - dx);
			yincD = 1;
		} else {
			d = 2 * dy + dx;
			dincD = 2 * dy;
			dincH = 2 * (dx + dy);
			yincD = -1;
		}
		bool done = false;
		while (!done && position != endPoint) {
			if ((d <= 0) ^ (yincD < 0)) {
				d += dincD;
			} else {
				d += dincH;
				position.y += yincD;
			}
			position.x++;
			done = position != startPoint && !clear(entity, position);
		}
	} else {
		if (dy < 0) {
			std::swap(position, endPoint);
			dy = -dy;
			dx = -dx;
		}
		int d;
		int xincD;
		int dincD;
		int dincH;
		if (dx > 0) {
			d = 2 * dx - dy;
			dincD = 2 * dx;
			dincH = 2 * (dx - dy);
			xincD = 1;
		} else {
			d = 2 * dx + dy;
			dincD = 2 * dx;
			dincH = 2 * (dy + dx);
			xincD = -1;
		}
		bool done = false;
		while (!done && position != endPoint) {
			if ((d <= 0) ^ (xincD < 0)) {
				d += dincD;
			} else {
				d += dincH;
				position.x += xincD;
			}
			position.y++;
			done = position != startPoint && !clear(entity, position);
		}
	}
	return position == endPoint;
}

void SyncMonsterAnim(int i)
{
	assurance((DWORD)i < MAXMONSTERS || i < 0, i);
	Monsters[i].MType = &LevelMonsterTypes[Monsters[i]._mMTidx];
	Monsters[i].MData = LevelMonsterTypes[Monsters[i]._mMTidx].MData;
	if (Monsters[i]._uniqtype != 0)
		Monsters[i].mName = _(UniqMonst[Monsters[i]._uniqtype - 1].mName);
	else
		Monsters[i].mName = _(Monsters[i].MData->mName);
	int mdir = Monsters[i]._mdir;

	int graphic = MA_STAND;

	switch (Monsters[i]._mmode) {
	case MM_STAND:
	case MM_DELAY:
	case MM_TALK:
		break;
	case MM_WALK:
	case MM_WALK2:
	case MM_WALK3:
		graphic = MA_WALK;
		break;
	case MM_ATTACK:
	case MM_RATTACK:
		graphic = MA_ATTACK;
		break;
	case MM_GOTHIT:
		graphic = MA_GOTHIT;
		break;
	case MM_DEATH:
		graphic = MA_DEATH;
		break;
	case MM_SATTACK:
	case MM_FADEIN:
	case MM_FADEOUT:
	case MM_SPSTAND:
	case MM_RSPATTACK:
	case MM_HEAL:
		graphic = MA_SPECIAL;
		break;
	case MM_CHARGE:
		graphic = MA_ATTACK;
		Monsters[i].AnimInfo.CurrentFrame = 1;
		Monsters[i].AnimInfo.NumberOfFrames = Monsters[i].MType->Anims[MA_ATTACK].Frames;
		break;
	default:
		Monsters[i].AnimInfo.CurrentFrame = 1;
		Monsters[i].AnimInfo.NumberOfFrames = Monsters[i].MType->Anims[MA_STAND].Frames;
		break;
	}

	if (Monsters[i].MType->Anims[graphic].CelSpritesForDirections[mdir])
		Monsters[i].AnimInfo.pCelSprite = &*Monsters[i].MType->Anims[graphic].CelSpritesForDirections[mdir];
	else
		Monsters[i].AnimInfo.pCelSprite = nullptr;
}

void M_FallenFear(Point position)
{
	for (int i = 0; i < ActiveMonsterCount; i++) {
		MonsterStruct *m = &Monsters[ActiveMonsters[i]];

		int rundist;
		switch (m->MType->mtype) {
		case MT_RFALLSP:
		case MT_RFALLSD:
			rundist = 7;
			break;
		case MT_DFALLSP:
		case MT_DFALLSD:
			rundist = 5;
			break;
		case MT_YFALLSP:
		case MT_YFALLSD:
			rundist = 3;
			break;
		case MT_BFALLSP:
		case MT_BFALLSD:
			rundist = 2;
			break;
		default:
			continue;
		}
		if (m->_mAi == AI_FALLEN
		    && position.WalkingDistance(m->position.tile) < 5
		    && m->_mhitpoints >> 6 > 0) {
			m->_mgoal = MGOAL_RETREAT;
			m->_mgoalvar1 = rundist;
			m->_mdir = GetDirection(position, m->position.tile);
		}
	}
}

const char *GetMonsterTypeText(const MonsterDataStruct &monsterData)
{
	switch (monsterData.mMonstClass) {
	case MC_ANIMAL:
		return _("Animal");
	case MC_DEMON:
		return _("Demon");
	case MC_UNDEAD:
		return _("Undead");
	}

	app_fatal("Unknown mMonstClass %i", monsterData.mMonstClass);
}

void PrintMonstHistory(int mt)
{
	if (sgOptions.Gameplay.bShowMonsterType) {
		strcpy(tempstr, fmt::format(_("Type: {:s}  Kills: {:d}"), GetMonsterTypeText(MonsterData[mt]), MonsterKillCounts[mt]).c_str());
	} else {
		strcpy(tempstr, fmt::format(_("Total kills: {:d}"), MonsterKillCounts[mt]).c_str());
	}

	AddPanelString(tempstr);
	if (MonsterKillCounts[mt] >= 30) {
		int minHP = MonsterData[mt].mMinHP;
		int maxHP = MonsterData[mt].mMaxHP;
		if (!gbIsHellfire && mt == MT_DIABLO) {
			minHP -= 2000;
			maxHP -= 2000;
		}
		if (!gbIsMultiplayer) {
			minHP /= 2;
			maxHP /= 2;
		}
		if (minHP < 1)
			minHP = 1;
		if (maxHP < 1)
			maxHP = 1;

		int hpBonusNightmare = 1;
		int hpBonusHell = 3;
		if (gbIsHellfire) {
			hpBonusNightmare = (!gbIsMultiplayer ? 50 : 100);
			hpBonusHell = (!gbIsMultiplayer ? 100 : 200);
		}
		if (sgGameInitInfo.nDifficulty == DIFF_NIGHTMARE) {
			minHP = 3 * minHP + hpBonusNightmare;
			maxHP = 3 * maxHP + hpBonusNightmare;
		} else if (sgGameInitInfo.nDifficulty == DIFF_HELL) {
			minHP = 4 * minHP + hpBonusHell;
			maxHP = 4 * maxHP + hpBonusHell;
		}
		strcpy(tempstr, fmt::format(_("Hit Points: {:d}-{:d}"), minHP, maxHP).c_str());
		AddPanelString(tempstr);
	}
	if (MonsterKillCounts[mt] >= 15) {
		int res = (sgGameInitInfo.nDifficulty != DIFF_HELL) ? MonsterData[mt].mMagicRes : MonsterData[mt].mMagicRes2;
		if ((res & (RESIST_MAGIC | RESIST_FIRE | RESIST_LIGHTNING | IMMUNE_MAGIC | IMMUNE_FIRE | IMMUNE_LIGHTNING)) == 0) {
			strcpy(tempstr, _("No magic resistance"));
			AddPanelString(tempstr);
		} else {
			if ((res & (RESIST_MAGIC | RESIST_FIRE | RESIST_LIGHTNING)) != 0) {
				strcpy(tempstr, _("Resists: "));
				if ((res & RESIST_MAGIC) != 0)
					strcat(tempstr, _("Magic "));
				if ((res & RESIST_FIRE) != 0)
					strcat(tempstr, _("Fire "));
				if ((res & RESIST_LIGHTNING) != 0)
					strcat(tempstr, _("Lightning "));
				tempstr[strlen(tempstr) - 1] = '\0';
				AddPanelString(tempstr);
			}
			if ((res & (IMMUNE_MAGIC | IMMUNE_FIRE | IMMUNE_LIGHTNING)) != 0) {
				strcpy(tempstr, _("Immune: "));
				if ((res & IMMUNE_MAGIC) != 0)
					strcat(tempstr, _("Magic "));
				if ((res & IMMUNE_FIRE) != 0)
					strcat(tempstr, _("Fire "));
				if ((res & IMMUNE_LIGHTNING) != 0)
					strcat(tempstr, _("Lightning "));
				tempstr[strlen(tempstr) - 1] = '\0';
				AddPanelString(tempstr);
			}
		}
	}
	pinfoflag = true;
}

void PrintUniqueHistory()
{
	if (sgOptions.Gameplay.bShowMonsterType) {
		strcpy(tempstr, fmt::format(_("Type: {:s}"), GetMonsterTypeText(*Monsters[pcursmonst].MData)).c_str());
		AddPanelString(tempstr);
	}

	int res = Monsters[pcursmonst].mMagicRes & (RESIST_MAGIC | RESIST_FIRE | RESIST_LIGHTNING | IMMUNE_MAGIC | IMMUNE_FIRE | IMMUNE_LIGHTNING);
	if (res == 0) {
		strcpy(tempstr, _("No resistances"));
		AddPanelString(tempstr);
		strcpy(tempstr, _("No Immunities"));
	} else {
		if ((res & (RESIST_MAGIC | RESIST_FIRE | RESIST_LIGHTNING)) != 0)
			strcpy(tempstr, _("Some Magic Resistances"));
		else
			strcpy(tempstr, _("No resistances"));
		AddPanelString(tempstr);
		if ((res & (IMMUNE_MAGIC | IMMUNE_FIRE | IMMUNE_LIGHTNING)) != 0) {
			strcpy(tempstr, _("Some Magic Immunities"));
		} else {
			strcpy(tempstr, _("No Immunities"));
		}
	}
	AddPanelString(tempstr);
	pinfoflag = true;
}

void MissToMonst(int i, Point position)
{
	assurance((DWORD)i < MAXMISSILES, i);

	MissileStruct *miss = &missile[i];
	int m = miss->_misource;

	assurance((DWORD)m < MAXMONSTERS, m);

	MonsterStruct *monst = &Monsters[m];
	Point oldPosition = miss->position.tile;
	dMonster[position.x][position.y] = m + 1;
	monst->_mdir = static_cast<Direction>(miss->_mimfnum);
	monst->position.tile = position;
	M_StartStand(m, monst->_mdir);
	if (monst->MType->mtype < MT_INCIN || monst->MType->mtype > MT_HELLBURN) {
		if ((monst->_mFlags & MFLAG_TARGETS_MONSTER) == 0)
			M_StartHit(m, -1, 0);
		else
			M2MStartHit(m, -1, 0);
	} else {
		M_StartFadein(m, monst->_mdir, false);
	}

	if ((monst->_mFlags & MFLAG_TARGETS_MONSTER) == 0) {
		int pnum = dPlayer[oldPosition.x][oldPosition.y] - 1;
		if (dPlayer[oldPosition.x][oldPosition.y] > 0) {
			if (monst->MType->mtype != MT_GLOOM && (monst->MType->mtype < MT_INCIN || monst->MType->mtype > MT_HELLBURN)) {
				M_TryH2HHit(m, dPlayer[oldPosition.x][oldPosition.y] - 1, 500, monst->mMinDamage2, monst->mMaxDamage2);
				if (pnum == dPlayer[oldPosition.x][oldPosition.y] - 1 && (monst->MType->mtype < MT_NSNAKE || monst->MType->mtype > MT_GSNAKE)) {
					if (plr[pnum]._pmode != PM_GOTHIT && plr[pnum]._pmode != PM_DEATH)
						StartPlrHit(pnum, 0, true);
					Point newPosition = oldPosition + monst->_mdir;
					if (PosOkPlayer(pnum, newPosition)) {
						plr[pnum].position.tile = newPosition;
						FixPlayerLocation(pnum, plr[pnum]._pdir);
						FixPlrWalkTags(pnum);
						dPlayer[newPosition.x][newPosition.y] = pnum + 1;
						SetPlayerOld(plr[pnum]);
					}
				}
			}
		}
		return;
	}

	if (dMonster[oldPosition.x][oldPosition.y] > 0) {
		if (monst->MType->mtype != MT_GLOOM && (monst->MType->mtype < MT_INCIN || monst->MType->mtype > MT_HELLBURN)) {
			M_TryM2MHit(m, dMonster[oldPosition.x][oldPosition.y] - 1, 500, monst->mMinDamage2, monst->mMaxDamage2);
			if (monst->MType->mtype < MT_NSNAKE || monst->MType->mtype > MT_GSNAKE) {
				Point newPosition = oldPosition + monst->_mdir;
				if (PosOkMonst(dMonster[oldPosition.x][oldPosition.y] - 1, newPosition)) {
					m = dMonster[oldPosition.x][oldPosition.y];
					dMonster[newPosition.x][newPosition.y] = m;
					dMonster[oldPosition.x][oldPosition.y] = 0;
					m--;
					Monsters[m].position.tile = newPosition;
					Monsters[m].position.future = newPosition;
				}
			}
		}
	}
}

bool PosOkMonst(int i, Point position)
{
	if (dPlayer[position.x][position.y] != 0 || dMonster[position.x][position.y] != 0)
		return false;

	return PosOkMonst2(i, position);
}

bool monster_posok(int i, Point position)
{
	int8_t mi = dMissile[position.x][position.y];
	if (mi == 0 || i < 0) {
		return true;
	}

	bool fire = false;
	bool lightning = false;
	if (mi > 0) {
		if (missile[mi - 1]._mitype == MIS_FIREWALL) { // BUGFIX: Change 'mi' to 'mi - 1' (fixed)
			fire = true;
		} else if (missile[mi - 1]._mitype == MIS_LIGHTWALL) { // BUGFIX: Change 'mi' to 'mi - 1' (fixed)
			lightning = true;
		}
	} else {
		for (int j = 0; j < nummissiles; j++) {
			mi = missileactive[j];
			if (missile[mi].position.tile == position) {
				if (missile[mi]._mitype == MIS_FIREWALL) {
					fire = true;
					break;
				}
				if (missile[mi]._mitype == MIS_LIGHTWALL) {
					lightning = true;
					break;
				}
			}
		}
	}
	if (fire && ((Monsters[i].mMagicRes & IMMUNE_FIRE) == 0 || Monsters[i].MType->mtype == MT_DIABLO))
		return false;
	if (lightning && ((Monsters[i].mMagicRes & IMMUNE_LIGHTNING) == 0 || Monsters[i].MType->mtype == MT_DIABLO))
		return false;

	return true;
}

bool PosOkMonst2(int i, Point position)
{
	if (SolidLoc(position))
		return false;

	if (dObject[position.x][position.y] != 0) {
		int oi = dObject[position.x][position.y] > 0 ? dObject[position.x][position.y] - 1 : -(dObject[position.x][position.y] + 1);
		if (object[oi]._oSolidFlag)
			return false;
	}

	return monster_posok(i, position);
}

bool PosOkMonst3(int i, Point position)
{
	bool isdoor = false;
	if (dObject[position.x][position.y] != 0) {
		int oi = dObject[position.x][position.y] > 0 ? dObject[position.x][position.y] - 1 : -(dObject[position.x][position.y] + 1);
		isdoor = IsAnyOf(object[oi]._otype, OBJ_L1LDOOR, OBJ_L1RDOOR, OBJ_L2LDOOR, OBJ_L2RDOOR, OBJ_L3LDOOR, OBJ_L3RDOOR);
		if (object[oi]._oSolidFlag && !isdoor)
			return false;
	}

	if ((SolidLoc(position) && !isdoor) || dPlayer[position.x][position.y] != 0 || dMonster[position.x][position.y] != 0)
		return false;

	return monster_posok(i, position);
}

bool IsSkel(int mt)
{
	return (mt >= MT_WSKELAX && mt <= MT_XSKELAX)
	    || (mt >= MT_WSKELBW && mt <= MT_XSKELBW)
	    || (mt >= MT_WSKELSD && mt <= MT_XSKELSD);
}

bool IsGoat(int mt)
{
	return (mt >= MT_NGOATMC && mt <= MT_GGOATMC)
	    || (mt >= MT_NGOATBW && mt <= MT_GGOATBW);
}

static int AddSkeleton(Point position, Direction dir, bool inMap)
{
	int j = 0;
	for (int i = 0; i < LevelMonsterTypeCount; i++) {
		if (IsSkel(LevelMonsterTypes[i].mtype))
			j++;
	}

	if (j == 0) {
		return -1;
	}

	int skeltypes = GenerateRnd(j);
	int m = 0;
	for (int i = 0; m < LevelMonsterTypeCount && i <= skeltypes; m++) {
		if (IsSkel(LevelMonsterTypes[m].mtype))
			i++;
	}
	return AddMonster(position, dir, m - 1, inMap);
}

int M_SpawnSkel(Point position, Direction dir)
{
	int skel = AddSkeleton(position, dir, true);
	if (skel != -1)
		M_StartSpStand(skel, dir);

	return skel;
}

void ActivateSpawn(int i, int x, int y, Direction dir)
{
	dMonster[x][y] = i + 1;
	Monsters[i].position.tile = { x, y };
	Monsters[i].position.future = { x, y };
	Monsters[i].position.old = { x, y };
	M_StartSpStand(i, dir);
}

bool SpawnSkeleton(int ii, Point position)
{
	if (ii == -1)
		return false;

	if (PosOkMonst(-1, position)) {
		Direction dir = GetDirection(position, position); // TODO useless calculation
		ActivateSpawn(ii, position.x, position.y, dir);
		return true;
	}

	bool monstok[3][3];

	bool savail = false;
	int yy = 0;
	for (int j = position.y - 1; j <= position.y + 1; j++) {
		int xx = 0;
		for (int k = position.x - 1; k <= position.x + 1; k++) {
			monstok[xx][yy] = PosOkMonst(-1, { k, j });
			savail = savail || monstok[xx][yy];
			xx++;
		}
		yy++;
	}
	if (!savail) {
		return false;
	}

	int rs = GenerateRnd(15) + 1;
	int x2 = 0;
	int y2 = 0;
	while (rs > 0) {
		if (monstok[x2][y2])
			rs--;
		if (rs > 0) {
			x2++;
			if (x2 == 3) {
				x2 = 0;
				y2++;
				if (y2 == 3)
					y2 = 0;
			}
		}
	}

	int dx = position.x - 1 + x2;
	int dy = position.y - 1 + y2;
	Direction dir = GetDirection({ dx, dy }, position);
	ActivateSpawn(ii, dx, dy, dir);

	return true;
}

int PreSpawnSkeleton()
{
	int skel = AddSkeleton({ 0, 0 }, DIR_S, false);
	if (skel != -1)
		M_StartStand(skel, DIR_S);

	return skel;
}

void TalktoMonster(int i)
{
	assurance((DWORD)i < MAXMONSTERS, i);

	MonsterStruct *monst = &Monsters[i];
	int pnum = monst->_menemy;
	monst->_mmode = MM_TALK;
	if (monst->_mAi != AI_SNOTSPIL && monst->_mAi != AI_LACHDAN) {
		return;
	}

	if (QuestStatus(Q_LTBANNER) && quests[Q_LTBANNER]._qvar1 == 2) {
		if (plr[pnum].TryRemoveInvItemById(IDI_BANNER)) {
			quests[Q_LTBANNER]._qactive = QUEST_DONE;
			monst->mtalkmsg = TEXT_BANNER12;
			monst->_mgoal = MGOAL_INQUIRING;
		}
	}
	if (QuestStatus(Q_VEIL) && monst->mtalkmsg >= TEXT_VEIL9) {
		if (plr[pnum].TryRemoveInvItemById(IDI_GLDNELIX)) {
			monst->mtalkmsg = TEXT_VEIL11;
			monst->_mgoal = MGOAL_INQUIRING;
		}
	}
}

void SpawnGolum(int i, Point position, int mi)
{
	assurance((DWORD)i < MAXMONSTERS, i);

	dMonster[position.x][position.y] = i + 1;
	Monsters[i].position.tile = position;
	Monsters[i].position.future = position;
	Monsters[i].position.old = position;
	Monsters[i]._pathcount = 0;
	Monsters[i]._mmaxhp = 2 * (320 * missile[mi]._mispllvl + plr[i]._pMaxMana / 3);
	Monsters[i]._mhitpoints = Monsters[i]._mmaxhp;
	Monsters[i].mArmorClass = 25;
	Monsters[i].mHit = 5 * (missile[mi]._mispllvl + 8) + 2 * plr[i]._pLevel;
	Monsters[i].mMinDamage = 2 * (missile[mi]._mispllvl + 4);
	Monsters[i].mMaxDamage = 2 * (missile[mi]._mispllvl + 8);
	Monsters[i]._mFlags |= MFLAG_GOLEM;
	M_StartSpStand(i, DIR_S);
	M_Enemy(i);
	if (i == myplr) {
		NetSendCmdGolem(
		    Monsters[i].position.tile.x,
		    Monsters[i].position.tile.y,
		    Monsters[i]._mdir,
		    Monsters[i]._menemy,
		    Monsters[i]._mhitpoints,
		    currlevel);
	}
}

bool CanTalkToMonst(int m)
{
	commitment((DWORD)m < MAXMONSTERS, m);

	if (Monsters[m]._mgoal == MGOAL_INQUIRING) {
		return true;
	}

	return Monsters[m]._mgoal == MGOAL_TALKING;
}

bool CheckMonsterHit(int m, bool *ret)
{
	commitment((DWORD)m < MAXMONSTERS, m);

	if (Monsters[m]._mAi == AI_GARG && (Monsters[m]._mFlags & MFLAG_ALLOW_SPECIAL) != 0) {
		Monsters[m]._mFlags &= ~MFLAG_ALLOW_SPECIAL;
		Monsters[m]._mmode = MM_SATTACK;
		*ret = true;
		return true;
	}

	if (Monsters[m].MType->mtype >= MT_COUNSLR && Monsters[m].MType->mtype <= MT_ADVOCATE) {
		if (Monsters[m]._mgoal != MGOAL_NORMAL) {
			*ret = false;
			return true;
		}
	}

	return false;
}

int encode_enemy(int m)
{
	if ((Monsters[m]._mFlags & MFLAG_TARGETS_MONSTER) != 0)
		return Monsters[m]._menemy + MAX_PLRS;

	return Monsters[m]._menemy;
}

void decode_enemy(int m, int enemy)
{
	if (enemy < MAX_PLRS) {
		Monsters[m]._mFlags &= ~MFLAG_TARGETS_MONSTER;
		Monsters[m]._menemy = enemy;
		Monsters[m].enemyPosition = plr[enemy].position.future;
	} else {
		Monsters[m]._mFlags |= MFLAG_TARGETS_MONSTER;
		enemy -= MAX_PLRS;
		Monsters[m]._menemy = enemy;
		Monsters[m].enemyPosition = Monsters[enemy].position.future;
	}
}

void MonsterStruct::CheckStandAnimationIsLoaded(int mdir)
{
	if (_mmode == MM_STAND || _mmode == MM_TALK)
		AnimInfo.pCelSprite = &*MType->Anims[MA_STAND].CelSpritesForDirections[mdir];
}

void MonsterStruct::Petrify()
{
	_mmode = MM_STONE;
	AnimInfo.IsPetrified = true;
}

bool MonsterStruct::IsWalking() const
{
	switch (_mmode) {
	case MM_WALK:
	case MM_WALK2:
	case MM_WALK3:
		return true;
	default:
		return false;
	}
}

} // namespace devilution
