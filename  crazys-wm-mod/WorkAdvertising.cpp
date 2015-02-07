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
extern cGangManager g_Gangs;
extern cMessageQue g_MessageQue;
extern cGold g_Gold;

// `J` Brothel Job - General
bool cJobManager::WorkAdvertising(sGirl* girl, sBrothel* brothel, bool Day0Night1, string& summary)
{
	int actiontype = ACTION_WORKADVERTISING;
	stringstream ss; string girlName = girl->m_Realname; ss << girlName;
	if (g_Girls.DisobeyCheck(girl, actiontype, brothel))
	{
		ss << " refused to advertise the brothel today.";
		girl->m_Events.AddMessage(ss.str(), IMGTYPE_PROFILE, EVENT_NOWORK);
		return true;
	}
	cConfig cfg;
	ss << " is assigned to advertize the brothel.\n\n";

	g_Girls.UnequipCombat(girl);	// put that shit away

	int enjoy = 0;
	int wages = 0;
	int tips = 0;
	int roll_a = g_Dice.d100(), roll_b = g_Dice.d100(), roll_c = g_Dice.d100();


	// How much will she help stretch your advertising budget? Let's find out
	double multiplier = JP_Advertising(girl, false);

	if (girl->is_slave() && !cfg.initial.slave_pay_outofpocket())	
		multiplier *= 0.9;	// unpaid slaves don't seem to want to advertise as much.
	if (girl->is_free())
		multiplier *= 1.1;	// paid free girls seem to attract more business

	// Complications
	if (roll_a <= 10)
	{
		enjoy -= g_Dice % 3 + 1;
		ss << gettext("She was harassed and made fun of while advertising.\n");
		if (girl->happiness() < 50)
		{
			enjoy -= 1;
			ss << gettext("Other then that she mostly just spent her time trying to not breakdown and cry.\n");
		}
		multiplier *= 0.8;
	}
	else if (roll_a >= 90)
	{
		enjoy += g_Dice % 3 + 1;
		ss << gettext("She made sure many people were interested in the buildings facilities.\n");
		multiplier *= 1.1;
	}
	else
	{
		enjoy += g_Dice % 2;
		ss << gettext("She had an uneventful day advertising.\n");
	}

	// add some more randomness
#if 0 // work in progress

	ss << gettext("She gave a tour of building making sure not to show the rooms that are messy.\n");
	if ()
	{
		ss << gettext("as she was walking the people around she would make sure to slip in a few recommendations for girls the people would enjoy getting services from.");
	}
	else
	{
		ss << gettext("She spent more time trying to flirt with the customers then actually getting anywhere with showing them around,\n");
		ss << gettext("She still got the job done but it was nowhere as good as it could have been");
	}
	if ()
	{
		message+=gettext("Most of the time she spent reading or doing anything else to spend her time to pass the time.\n");
	}
	else
	{
		message+=gettext("She decided not to get in trouble so she just waited there silently for someone to come so she could do her job properly.\n");
	}

#endif

	/* `J` If she is not happy at her job she may ask you to change her job.
	*	Submitted by MuteDay as a subcode of bad complications but I liked it and made it as a separate section
	*	I will probably make it its own function when it works better.
	*/
	if (girl->m_Enjoyment[actiontype] < -10) 						// if she does not like the job
	{
		int enjoyamount = girl->m_Enjoyment[actiontype];
		int saysomething = g_Dice%girl->confidence() - enjoyamount;	// the more she does not like the job the more likely she is to say something about it
		saysomething -= girl->pcfear() / (girl->is_free() ? 2 : 1);	// reduce by fear (half if free)

		if (saysomething > 50)
		{
			stringstream warning;
			warning << girlName << " comes up to you and asks you to change her job, She does not like advertizing.\n";
			girl->m_Events.AddMessage(warning.str(), IMGTYPE_PROFILE, EVENT_WARNING);
		}
		else if (saysomething > 25)
		{
			ss << gettext("She looks at you like she has something to say but then turns around and walks away.\n");
		}
	}



	ss << gettext("She managed to stretch the effectiveness of your advertising budget by about ") << int(multiplier) << gettext("%.");
	// if you pay slave girls out of pocket  or if she is a free girl  pay them
	if ((girl->is_slave() && cfg.initial.slave_pay_outofpocket()) || girl->is_free())
	{
		girl->m_Pay += 70;
		g_Gold.advertising_costs(70);
		ss << gettext(" You paid her 70 gold for her advertising efforts.");
	}
	else
	{
		ss << gettext(" You do not pay your slave for her advertising efforts.");
	}

	girl->m_Events.AddMessage(ss.str(), IMGTYPE_PROFILE, EVENT_SUMMARY);
	g_Girls.UpdateEnjoyment(girl, actiontype, enjoy, true);

	// now to boost the brothel's advertising level accordingly
	brothel->m_AdvertisingLevel += (multiplier / 100);

	// Improve girl
	int xp = 5, libido = 1, skill = 3;

	if (g_Girls.HasTrait(girl, "Quick Learner"))		{ skill += 1; xp += 3; }
	else if (g_Girls.HasTrait(girl, "Slow Learner"))	{ skill -= 1; xp -= 3; }
	if (g_Girls.HasTrait(girl, "Nymphomaniac"))			{ libido += 2; }

	g_Girls.UpdateStat(girl, STAT_EXP, xp);
	g_Girls.UpdateSkill(girl, SKILL_PERFORMANCE, g_Dice%skill + 1);
	g_Girls.UpdateSkill(girl, SKILL_SERVICE, g_Dice%skill + 1);
	g_Girls.UpdateTempStat(girl, STAT_LIBIDO, libido);

	g_Girls.PossiblyGainNewTrait(girl, "Charismatic", 70, actiontype, "Advertising on a daily basis has made " + girl->m_Realname + " more Charismatic.", Day0Night1 == SHIFT_NIGHT);
	
	g_Girls.PossiblyLoseExistingTrait(girl, "Nervous", 40, actiontype, girl->m_Realname + " seems to finally be getting over her shyness. She's not always so Nervous anymore.", Day0Night1 == SHIFT_NIGHT);

	return false;
}

double cJobManager::JP_Advertising(sGirl* girl, bool estimate)
{
	double cval = 0.0;
	double jobperformance = 0.0;
	if (estimate)	// for third detail string
	{
		jobperformance =
			g_Girls.GetSkill(girl, SKILL_PERFORMANCE) / 6.0 +
			g_Girls.GetSkill(girl, SKILL_SERVICE) / 6.0 +
			g_Girls.GetStat(girl, STAT_CHARISMA) / 6.0 +
			g_Girls.GetStat(girl, STAT_BEAUTY) / 10.0 +
			g_Girls.GetStat(girl, STAT_INTELLIGENCE) / 6.0 +
			g_Girls.GetStat(girl, STAT_CONFIDENCE) / 10.0 +
			g_Girls.GetStat(girl, STAT_FAME) / 10.0;
	}
	else			// for the actual check
	{
		cval = g_Girls.GetSkill(girl, SKILL_PERFORMANCE);	// `J` added
		if (cval > 0)
		{
			cval = g_Dice % (int)cval + (cval / 2);  // random 50%-150% range
			jobperformance += (cval / 6);  // add ~17% of performance skill to jobperformance
		}
		cval = g_Girls.GetSkill(girl, SKILL_SERVICE);
		if (cval > 0)	// `J` halved jobperformance to include performace without excessive change
		{
			cval = g_Dice % (int)cval + (cval / 2);  // random 50%-150% range
			jobperformance += (cval / 6);  // add ~17% of service skill to jobperformance
		}
		cval = g_Girls.GetStat(girl, STAT_CHARISMA);
		if (cval > 0)
		{
			cval = g_Dice % (int)cval + (cval / 2);  // random 50%-150% range
			jobperformance += (cval / 6);  // add ~17% of charisma to jobperformance
		}
		cval = g_Girls.GetStat(girl, STAT_BEAUTY);
		if (cval > 0)
		{
			cval = g_Dice % (int)cval + (cval / 2);  // random 50%-150% range
			jobperformance += (cval / 10);  // add 10% of beauty to jobperformance
		}
		cval = g_Girls.GetStat(girl, STAT_INTELLIGENCE);
		if (cval > 0)
		{
			cval = g_Dice % (int)cval + (cval / 2);  // random 50%-150% range
			jobperformance += (cval / 6);  // add ~17% of intelligence to jobperformance
		}
		cval = g_Girls.GetStat(girl, STAT_CONFIDENCE);
		if (cval > 0)
		{
			cval = g_Dice % (int)cval + (cval / 2);  // random 50%-150% range
			jobperformance += (cval / 10);  // add 10% of confidence to jobperformance
		}
		cval = g_Girls.GetStat(girl, STAT_FAME);
		if (cval > 0)
		{
			cval = g_Dice % (int)cval + (cval / 2);  // random 50%-150% range
			jobperformance += (cval / 10);  // add 10% of fame to jobperformance
		}

	}

	// useful traits
	if (girl->has_trait("Psychic"))		jobperformance += 10;
	if (girl->has_trait("Cool Person"))	jobperformance += 10;
	if (girl->has_trait("Sexy Air"))	jobperformance += 10;
	if (girl->has_trait("Charismatic"))	jobperformance += 10;
	if (girl->has_trait("Charming"))	jobperformance += 10;

	// unhelpful traits
	if (girl->has_trait("Nervous"))		jobperformance -= 5;
	if (girl->has_trait("Clumsy"))		jobperformance -= 5;
	if (girl->has_trait("Retarded"))	jobperformance -= 20;
	if (girl->has_trait("Malformed"))	jobperformance -= 20;

	return jobperformance;
}