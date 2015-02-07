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
#include "cClinic.h"
#include "cMovieStudio.h"
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
extern cMovieStudioManager g_Studios;
extern cArenaManager g_Arena;
extern cGangManager g_Gangs;
extern cMessageQue g_MessageQue;
extern cGold g_Gold;

// `J` Arena Job - Staff
bool cJobManager::WorkCityGuard(sGirl* girl, sBrothel* brothel, bool Day0Night1, string& summary)
{
	int actiontype = ACTION_WORKSECURITY;
	stringstream ss; string girlName = girl->m_Realname; ss << girlName;
	if (g_Girls.DisobeyCheck(girl, actiontype, brothel))			// they refuse to work 
	{
		ss << " refused to work during the " << (Day0Night1 ? "night" : "day") << " shift.";
		girl->m_Events.AddMessage(ss.str(), IMGTYPE_PROFILE, EVENT_NOWORK);
		return true;
	}
	ss << " helps guard the city.\n\n";

	g_Girls.EquipCombat(girl);	// ready armor and weapons!

	int roll_a = g_Dice.d100(), roll_b = g_Dice.d100();
	int wages = 150, enjoy = 0, enjoyc = 0, sus = 0;
	int imagetype = IMGTYPE_PROFILE;

	int agl = (g_Girls.GetStat(girl, STAT_AGILITY) / 2 + g_Dice % (g_Girls.GetSkill(girl, SKILL_COMBAT) / 2));


	sGirl* tempgirl = g_Girls.CreateRandomGirl(18, false, false, false, false, false, true);

	if (roll_a >= 50)
	{
		ss << girlName << " didn't find any trouble today.";
		sus -= 5;
	}
	else if (roll_a >= 25)
	{
		ss << girlName << " spotted a theif and ";
		if (agl >= 90)
		{
			ss << "was on them before they could blink.  Putting a stop to the theft.";
			sus -= 20;
			enjoy += 3;
		}
		else if (agl >= 75)
		{
			ss << "was on them before they could get away.  She is quick.";
			sus -= 15;
			enjoy += 1;
		}
		else if (agl >= 50)
		{
			ss << "was able to keep up, ";
			if (roll_b >= 50)	{ sus += 5; ss << "but they ended up giving her the slip."; }
			else /*        */	{ sus -= 10; ss << "and was able to catch them."; }
		}
		else
		{
			ss << "was left eating dust. Damn is she slow.";
			sus += 10;
			enjoy -= 3;
		}
	}
	else
	{
		Uint8 fight_outcome = 0;
		if (tempgirl)		// `J` reworked incase there are no Non-Human Random Girls
		{
			fight_outcome = g_Girls.girl_fights_girl(girl, tempgirl);
		}
		else
		{
			g_LogFile.write("Error: You have no Random Girls for your girls to fight\n");
			g_LogFile.write("Error: You need a Random Girl to allow WorkCityGuard randomness");
			fight_outcome = 7;
		}
		if (fight_outcome == 7)
		{
			ss << "There were no criminals around for her to fight.\n\n";
			ss << "(Error: You need a Random Girl to allow WorkCityGuard randomness)";
		}
		else if (fight_outcome == 1)	// she won
		{
			enjoy += 3; enjoyc += 3;
			ss << girlName << " ran into some trouble and ended up in a fight. She was able to win.";
			sus -= 20;
			imagetype = IMGTYPE_COMBAT;
		}
		else  // she lost or it was a draw
		{
			ss << girlName << " ran into some trouble and ended up in a fight. She was unable to win the fight.";
			enjoy -= 1; enjoyc -= 1;
			sus += 10;
			imagetype = IMGTYPE_COMBAT;
		}
	}

	// Cleanup
	if (tempgirl) delete tempgirl; tempgirl = 0;


	girl->m_Events.AddMessage(ss.str(), imagetype, Day0Night1);
	g_Brothels.GetPlayer()->suspicion(sus);
	girl->m_Pay = wages;


	// Improve girl
	int xp = 8, libido = 2, skill = 3;

	if (g_Girls.HasTrait(girl, "Quick Learner"))		{ skill += 1; xp += 3; }
	else if (g_Girls.HasTrait(girl, "Slow Learner"))	{ skill -= 1; xp -= 3; }
	if (g_Girls.HasTrait(girl, "Nymphomaniac"))			{ libido += 2; }

	g_Girls.UpdateStat(girl, STAT_EXP, xp);
	g_Girls.UpdateSkill(girl, SKILL_COMBAT, g_Dice % skill);
	g_Girls.UpdateSkill(girl, SKILL_MAGIC, g_Dice % skill);
	g_Girls.UpdateStat(girl, STAT_AGILITY, g_Dice % skill);
	g_Girls.UpdateStat(girl, STAT_CONSTITUTION, g_Dice % skill);
	g_Girls.UpdateTempStat(girl, STAT_LIBIDO, libido);


	g_Girls.UpdateEnjoyment(girl, actiontype, enjoy, true);
	g_Girls.UpdateEnjoyment(girl, ACTION_COMBAT, enjoyc, true);
	g_Girls.PossiblyGainNewTrait(girl, "Tough", 20, actiontype, "She has become pretty Tough from all of the fights she's been in.", Day0Night1);
	g_Girls.PossiblyGainNewTrait(girl, "Aggressive", 60, actiontype, "She is getting rather Aggressive from her enjoyment of combat.", Day0Night1);
	g_Girls.PossiblyGainNewTrait(girl, "Fleet of Foot", 30, actiontype, "She is getting rather fast from all the fighting.", Day0Night1);

	return false;
}

double cJobManager::JP_CityGuard(sGirl* girl, bool estimate)// not used
{
	double jobperformance = 0.0;
	if (estimate)// for third detail string
	{
	}
	else// for the actual check
	{
	}
	return jobperformance;
}