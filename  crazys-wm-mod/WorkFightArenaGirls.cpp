/*
* Copyright 2009, 2010, The Pink Petal Development Team.
* The Pink Petal Devloment Team are defined as the game's coders
* who meet on http://pinkpetal.org     // old site: http://pinkpetal .co.cc
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "cJobManager.h"
#include "cBrothel.h"
#include "cArena.h"
#include "cCustomers.h"
#include "cRng.h"
#include "cInventory.h"
#include "sConfig.h"
#include "cRival.h"
#include <sstream>
#include "CLog.h"
#include "cTrainable.h"
#include "cTariff.h"
#include "cGold.h"
#include "cGangs.h"
#include "cMessageBox.h"
#include "libintl.h"

extern cRng g_Dice;
extern CLog g_LogFile;
extern cCustomers g_Customers;
extern cInventory g_InvManager;
extern cBrothelManager g_Brothels;
extern cArenaManager g_Arena;
extern cGangManager g_Gangs;
extern cMessageQue g_MessageQue;
extern cGold g_Gold;

// `J` Arena Job - Fighting
bool cJobManager::WorkFightArenaGirls(sGirl* girl, sBrothel* brothel, bool Day0Night1, string& summary)
{
	int actiontype = ACTION_COMBAT;
	stringstream ss; string girlName = girl->m_Realname; ss << girlName;
	if (g_Girls.DisobeyCheck(girl, actiontype, brothel))			// they refuse to work 
	{
		ss << " refused to work during the " << (Day0Night1 ? "night" : "day") << " shift.";
		girl->m_Events.AddMessage(ss.str(), IMGTYPE_PROFILE, EVENT_NOWORK);
		return true;
	}
	ss << " was assigned to fight other girls in the arena.\n\n";

	cConfig cfg;
	int wages = 0, fight_outcome = 0, enjoyment = 0, fame = 0, imagetype = IMGTYPE_COMBAT;
	bool unique = false;

	double jobperformance = JP_FightArenaGirls(girl, false);

	g_Girls.EquipCombat(girl);		// ready armor and weapons!

	sGirl* tempgirl = g_Girls.CreateRandomGirl(18, false, false, false, false, false, true);
	if (tempgirl) fight_outcome = g_Girls.girl_fights_girl(girl, tempgirl);
	else fight_outcome = 7;			// `J` reworked incase there are no Non-Human Random Girls
	if (fight_outcome == 7)
	{
		g_LogFile.write("Error: You have no Arena Girls for your girls to fight\n");
		g_LogFile.write("Error: You need an Arena Girl to allow WorkFightArenaGirls randomness");
		ss << "There were no Arena Girls for her to fight.\n\n(Error: You need an Arena Girl to allow WorkFightArenaGirls randomness)";
		imagetype = IMGTYPE_PROFILE;
	}
	else if (fight_outcome == 1)	// she won
	{
		enjoyment = g_Dice % 3 + 1;
		fame = g_Dice % 3 + 1;
		sGirl* ugirl = 0;
		if (g_Dice.percent(20))		// chance of getting unique girl
		{
			ugirl = g_Girls.GetRandomGirl(false, false, true);
			if (ugirl != 0) unique = true;
		}
		if (unique)
		{
			ugirl->m_Stats[STAT_HEALTH] = g_Dice % 50 + 1;
			ugirl->m_Stats[STAT_HAPPINESS] = g_Dice % 80 + 1;
			ugirl->m_Stats[STAT_TIREDNESS] = g_Dice % 50 + 50;
			ugirl->m_States |= (1 << STATUS_ARENA);
			ss << girlName << " won her fight against " << ugirl->m_Realname << ".\n";
			if (g_Dice.percent(50))
			{
				ugirl->m_States |= (1 << STATUS_SLAVE);
				ss << ugirl->m_Realname << "'s owner could not afford to pay you your winnings so he gave her to you instead.";
			}
			else
			{
				ss << ugirl->m_Realname << " put up a good fight so you let her live as long as she came work for you.";
				wages = 100 + g_Dice % (girl->fame() + girl->charisma());
			}
			g_MessageQue.AddToQue(ss.str(), 0);
			g_Brothels.GetDungeon()->AddGirl(ugirl, DUNGEON_NEWARENA);
		}
		else
		{
			ss << "She won the fight.";
			wages = 100 + g_Dice % (girl->fame() + girl->charisma());
		}
	}
	else if (fight_outcome == 2) // she lost
	{
		enjoyment = -(g_Dice % 3 + 1);
		fame = -(g_Dice % 3 + 1);
		ss << "She lost the fight.";
		int cost = 150;
		brothel->m_Finance.arena_costs(cost);
		ss << " You had to pay " << cost << " gold cause your girl lost.";
		/*that should work but now need to make if you lose the girl if you dont have the gold zzzzz FIXME*/
	}
	else if (fight_outcome == 0)  // it was a draw
	{
		enjoyment = g_Dice % 3 - 2;
		fame = g_Dice % 3 - 2;
		ss << "The fight ended in a draw.";
	}

	if (tempgirl) delete tempgirl; tempgirl = 0;	// Cleanup


	// Improve girl
	int fightxp = (fight_outcome == 1) ? 3 : 1;
	int xp = 5 * fightxp, libido = 5, skill = 1;

	if (g_Girls.HasTrait(girl, "Quick Learner"))		{ skill += 1; xp += 5; }
	else if (g_Girls.HasTrait(girl, "Slow Learner"))	{ skill -= 1; xp -= 5; }
	if (g_Girls.HasTrait(girl, "Nymphomaniac"))			{ libido += 2; }

	if ((girl->is_slave() && !cfg.initial.slave_pay_outofpocket()))
	{
		wages = 0;
	}

	girl->m_Pay = wages;
	girl->m_Events.AddMessage(ss.str(), imagetype, Day0Night1);
	g_Girls.UpdateStat(girl, STAT_FAME, fame);
	g_Girls.UpdateStat(girl, STAT_EXP, xp);
	g_Girls.UpdateSkill(girl, SKILL_COMBAT, g_Dice%fightxp + skill);
	g_Girls.UpdateSkill(girl, SKILL_MAGIC, g_Dice%fightxp + skill);
	g_Girls.UpdateStat(girl, STAT_AGILITY, g_Dice%fightxp + skill);
	g_Girls.UpdateStat(girl, STAT_CONSTITUTION, g_Dice%fightxp + skill);
	g_Girls.UpdateTempStat(girl, STAT_LIBIDO, libido);
	g_Girls.UpdateEnjoyment(girl, actiontype, enjoyment, true);

	/* `J` this will be a place holder until a better payment system gets done
	*
	*/
	int earned = 0;
	for (int i = 0; i < jobperformance; i++)
	{
		earned += g_Dice % 10 + 5; // 5-15 gold per customer  This may need tweaked to get it where it should be for the pay
	}
	brothel->m_Finance.arena_income(earned);
	ss.str("");
	ss << girlName << " drew in " << jobperformance << " people to watch her and you earned " << earned << " from it.";
	girl->m_Events.AddMessage(ss.str(), IMGTYPE_PROFILE, Day0Night1);


	//gain traits
	g_Girls.PossiblyGainNewTrait(girl, "Tough", 65, actiontype, "She has become pretty Tough from all of the fights she's been in.", Day0Night1);
	g_Girls.PossiblyGainNewTrait(girl, "Fleet of Foot", 55, actiontype, "She is getting rather fast from all the fighting.", Day0Night1);
	g_Girls.PossiblyGainNewTrait(girl, "Aggressive", 70, actiontype, "She is getting rather Aggressive from her enjoyment of combat.", Day0Night1);
	//lose traits
	g_Girls.PossiblyLoseExistingTrait(girl, "Fragile", 35, actiontype, girlName + " has had to heal from so many injuries you can't say she is fragile anymore.", Day0Night1);
	return false;
}

double cJobManager::JP_FightArenaGirls(sGirl* girl, bool estimate)// not used
{
	double jobperformance = 0.0;

	if (estimate)// for third detail string
	{
		jobperformance +=
			(girl->fame() / 2) +
			(girl->charisma() / 2) +
			(girl->combat() / 2) +
			(girl->magic() / 2) +
			(girl->level());
	}
	else// for the actual check
	{
		jobperformance += (g_Girls.GetStat(girl, STAT_FAME) + g_Girls.GetStat(girl, STAT_CHARISMA)) / 2;

	}
	return jobperformance;
}
