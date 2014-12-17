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
#include "cBrothel.h"
#include "cScreenItemManagement.h"
#include "cWindowManager.h"
#include "cGold.h"
#include "InterfaceGlobals.h"
#include "InterfaceProcesses.h"
#include "sConfig.h"
#include "libintl.h"
#include "cGirls.h"
#include "main.h"

extern bool g_InitWin;
extern int g_CurrBrothel;
extern cGold g_Gold;
extern cBrothelManager g_Brothels;
extern cClinicManager g_Clinic;
extern cMovieStudioManager g_Studios;
extern cArenaManager g_Arena;
extern cCentreManager g_Centre;
extern cHouseManager g_House;
extern cInventory g_InvManager;
extern cWindowManager g_WinManager;
extern bool g_AllTogle;
extern string g_ReturnText;

extern	bool	g_AltKeys;	// New hotkeys --PP
extern	bool	g_R_Key;
extern	bool	g_F_Key;
extern	bool	g_T_Key;
extern	bool	g_G_Key;
extern	bool	g_Y_Key;
extern	bool	g_H_Key;
extern	bool	g_U_Key;
extern	bool	g_J_Key;
extern	bool	g_I_Key;
extern	bool	g_K_Key;

static int selection = -1;

extern sGirl *selected_girl;

extern	int		g_CurrentScreen;

bool cScreenItemManagement::ids_set = false;

/* `J` how many special slots at the top of the girls list:
* Currently only Player and Shop
* To be added: Store Room and Vault
//*/
static int NumberSpecialSlots = 2;
static int leftOwner = 0;
static int rightOwner = 1;

static int NumBrothelGirls = 0;		// all brothel girls
static int NumBrothelGirls0 = 0;
static int NumBrothelGirls1 = 0;
static int NumBrothelGirls2 = 0;
static int NumBrothelGirls3 = 0;
static int NumBrothelGirls4 = 0;
static int NumBrothelGirls5 = 0;
static int NumBrothelGirls6 = 0;

static int NumStudioGirls = 0;
static int NumArenaGirls = 0;
static int NumCentreGirls = 0;
static int NumClinicGirls = 0;
static int NumFarmGirls = 0;
static int NumHouseGirls = 0;
static int NumDungeonGirls = 0;

static int Brothel1Index = 0;
static int Brothel2Index = 0;
static int Brothel3Index = 0;
static int Brothel4Index = 0;
static int Brothel5Index = 0;
static int Brothel6Index = 0;

static int CentreIndex = 0;
static int ArenaIndex = 0;
static int ClinicIndex = 0;
static int StudioIndex = 0;
static int HouseIndex = 0;
static int FarmIndex = 0;
static int DungeonIndex = 0;
static int filter = 0;

static int leftItem = -2;
static int rightItem = -2;
static int sel_next_l = -1;
static int sel_next_r = -1;
static string sel_name_l = "";
static string sel_name_r = "";

int HateLove = 0;

static SDL_Color* RarityColor[9];
static bool AutoUseItems = false;

void cScreenItemManagement::set_ids()
{
	ids_set = true;
	curbrothel_id	= get_id("CurrentBrothel");
	back_id			= get_id("BackButton");
	shift_l_id		= get_id("ShiftLeftButton");
	shift_r_id		= get_id("ShiftRightButton");
	gold_id			= get_id("PlayerGold");
	desc_id			= get_id("ItemDesc");
	owners_l_id		= get_id("OwnersLeftList");
	items_l_id		= get_id("ItemsLeftList");
	owners_r_id		= get_id("OwnersRightList");
	items_r_id		= get_id("ItemsRightList");
	filter_id		= get_id("FilterList");
	autouse_id		= get_id("AutoUseItems");
	equip_l_id		= get_id("EquipLeftButton");
	unequip_l_id	= get_id("UnequipLeftButton");
	equip_r_id		= get_id("EquipRightButton");
	unequip_r_id	= get_id("UnequipRightButton");

	cConfig cfg;
	for (int i = 0; i < 9; i++) RarityColor[i] = cfg.items.rarity_color(i);
}

void cScreenItemManagement::init()
{
	g_CurrentScreen = SCREEN_INVENTORY;
	if (!g_InitWin) return;
	Focused();
	g_InitWin = false;

	////////////////////

	string brothel = gettext("Current Brothel: ");
	brothel += g_Brothels.GetName(g_CurrBrothel);
	EditTextItem(brothel, curbrothel_id);

	NumBrothelGirls = NumBrothelGirls0 = NumBrothelGirls1 = NumBrothelGirls2 = NumBrothelGirls3 = NumBrothelGirls4 = NumBrothelGirls5 = NumBrothelGirls6 = 0;
	NumStudioGirls = NumArenaGirls = NumCentreGirls = NumClinicGirls = NumFarmGirls = NumHouseGirls = NumDungeonGirls = 0;
	leftOwner = 0;
	rightOwner = 1;

	// clear list boxes
	ClearListBox(owners_l_id);
	ClearListBox(owners_r_id);
	ClearListBox(items_l_id);
	ClearListBox(items_r_id);
	ClearListBox(filter_id);

	// setup the filter
	AddToListBox(filter_id, 0, gettext("All"));
	AddToListBox(filter_id, 1, gettext("Rings"));
	AddToListBox(filter_id, 2, gettext("Dress"));
	AddToListBox(filter_id, 12, gettext("Underwear"));
	AddToListBox(filter_id, 3, gettext("Shoes"));
	AddToListBox(filter_id, 4, gettext("Consumables"));
	AddToListBox(filter_id, 5, gettext("Necklaces"));
	AddToListBox(filter_id, 6, gettext("Large Weapons"));
	AddToListBox(filter_id, 11, gettext("Small Weapons"));
	AddToListBox(filter_id, 8, gettext("Armor"));
	AddToListBox(filter_id, 9, gettext("Misc"));
	AddToListBox(filter_id, 10, gettext("Armbands"));
	if (filter == -1) filter = 0;
	SetSelectedItemInList(filter_id, filter, false);

	// add shop and player to list
	AddToListBox(owners_l_id, 0, gettext("Player"));
	AddToListBox(owners_l_id, 1, gettext("Shop"));

	AddToListBox(owners_r_id, 0, gettext("Player"));
	AddToListBox(owners_r_id, 1, gettext("Shop"));

	// and girls from current brothel to list
	int i = NumberSpecialSlots;
	sGirl* temp;
	temp = g_Brothels.GetGirl(0, 0);
	while (temp)
	{
		if (temp == 0) break;
		if (g_AllTogle && selected_girl == temp) rightOwner = i;
		AddToListBox(owners_l_id, i, temp->m_Realname);
		AddToListBox(owners_r_id, i, temp->m_Realname);
		NumBrothelGirls0++;
		NumBrothelGirls++;
		temp = temp->m_Next;
		i++;
	}
	if (g_Brothels.GetNumBrothels() > 1)
	{
		temp = g_Brothels.GetGirl(1, 0);
		while (temp)
		{
			if (temp == 0) break;
			if (g_AllTogle && selected_girl == temp) rightOwner = i;
			AddToListBox(owners_l_id, i, temp->m_Realname);
			AddToListBox(owners_r_id, i, temp->m_Realname);
			NumBrothelGirls1++;
			NumBrothelGirls++;
			temp = temp->m_Next;
			i++;
		}
	}
	if (g_Brothels.GetNumBrothels() > 2)
	{
		temp = g_Brothels.GetGirl(2, 0);
		while (temp)
		{
			if (temp == 0) break;
			if (g_AllTogle && selected_girl == temp) rightOwner = i;
			AddToListBox(owners_l_id, i, temp->m_Realname);
			AddToListBox(owners_r_id, i, temp->m_Realname);
			NumBrothelGirls2++;
			NumBrothelGirls++;
			temp = temp->m_Next;
			i++;
		}
	}
	if (g_Brothels.GetNumBrothels() > 3)
	{
		temp = g_Brothels.GetGirl(3, 0);
		while (temp)
		{
			if (temp == 0) break;
			if (g_AllTogle && selected_girl == temp) rightOwner = i;
			AddToListBox(owners_l_id, i, temp->m_Realname);
			AddToListBox(owners_r_id, i, temp->m_Realname);
			NumBrothelGirls3++;
			NumBrothelGirls++;
			temp = temp->m_Next;
			i++;
		}
	}
	if (g_Brothels.GetNumBrothels() > 4)
	{
		temp = g_Brothels.GetGirl(4, 0);
		while (temp)
		{
			if (temp == 0) break;
			if (g_AllTogle && selected_girl == temp) rightOwner = i;
			AddToListBox(owners_l_id, i, temp->m_Realname);
			AddToListBox(owners_r_id, i, temp->m_Realname);
			NumBrothelGirls4++;
			NumBrothelGirls++;
			temp = temp->m_Next;
			i++;
		}
	}
	if (g_Brothels.GetNumBrothels() > 5)
	{
		temp = g_Brothels.GetGirl(5, 0);
		while (temp)
		{
			if (temp == 0) break;
			if (g_AllTogle && selected_girl == temp) rightOwner = i;
			AddToListBox(owners_l_id, i, temp->m_Realname);
			AddToListBox(owners_r_id, i, temp->m_Realname);
			NumBrothelGirls5++;
			NumBrothelGirls++;
			temp = temp->m_Next;
			i++;
		}
	}
	if (g_Brothels.GetNumBrothels() > 6)
	{
		temp = g_Brothels.GetGirl(6, 0);
		while (temp)
		{
			if (temp == 0) break;
			if (g_AllTogle && selected_girl == temp) rightOwner = i;
			AddToListBox(owners_l_id, i, temp->m_Realname);
			AddToListBox(owners_r_id, i, temp->m_Realname);
			NumBrothelGirls6++;
			NumBrothelGirls++;
			temp = temp->m_Next;
			i++;
		}
	}
	//add girls from Studio
	temp = g_Studios.GetGirl(0, 0);
	while (temp)
	{
		if (temp == 0) break;
		if (g_AllTogle && selected_girl == temp) rightOwner = i;
		AddToListBox(owners_l_id, i, temp->m_Realname);
		AddToListBox(owners_r_id, i, temp->m_Realname);
		NumStudioGirls++;
		temp = temp->m_Next;
		i++;
	}

	//add girls from Arena
	temp = g_Arena.GetGirl(0, 0);
	while (temp)
	{
		if (temp == 0) break;
		if (g_AllTogle && selected_girl == temp) rightOwner = i;
		AddToListBox(owners_l_id, i, temp->m_Realname);
		AddToListBox(owners_r_id, i, temp->m_Realname);
		NumArenaGirls++;
		temp = temp->m_Next;
		i++;
	}

	// Add girls from centre
	temp = g_Centre.GetGirl(0, 0);
	while (temp)
	{
		if (temp == 0) break;
		if (g_AllTogle && selected_girl == temp) rightOwner = i;
		AddToListBox(owners_l_id, i, temp->m_Realname);
		AddToListBox(owners_r_id, i, temp->m_Realname);
		NumCentreGirls++;
		temp = temp->m_Next;
		i++;
	}

	//add girls from Clinic
	temp = g_Clinic.GetGirl(0, 0);
	while (temp)
	{
		if (temp == 0) break;
		if (g_AllTogle && selected_girl == temp) rightOwner = i;
		AddToListBox(owners_l_id, i, temp->m_Realname);
		AddToListBox(owners_r_id, i, temp->m_Realname);
		NumClinicGirls++;
		temp = temp->m_Next;
		i++;
	}

	//add girls from Farm
	temp = g_Farm.GetGirl(0, 0);
	while (temp)
	{
		if (temp == 0) break;
		if (g_AllTogle && selected_girl == temp) rightOwner = i;
		AddToListBox(owners_l_id, i, temp->m_Realname);
		AddToListBox(owners_r_id, i, temp->m_Realname);
		NumFarmGirls++;
		temp = temp->m_Next;
		i++;
	}

	//add girls from House
	temp = g_House.GetGirl(0, 0);
	while (temp)
	{
		if (temp == 0) break;
		if (g_AllTogle && selected_girl == temp) rightOwner = i;
		AddToListBox(owners_l_id, i, temp->m_Realname);
		AddToListBox(owners_r_id, i, temp->m_Realname);
		NumHouseGirls++;
		temp = temp->m_Next;
		i++;
	}

	// add current dungeon girls to list
	sDungeonGirl* temp2 = g_Brothels.GetDungeon()->GetGirl(0);
	while (temp2)
	{
		if (temp2 == 0) break;
		if (g_AllTogle && selected_girl == temp2->m_Girl) rightOwner = i;
		AddToListBox(owners_l_id, i, temp2->m_Girl->m_Realname, COLOR_RED);
		AddToListBox(owners_r_id, i, temp2->m_Girl->m_Realname, COLOR_RED);
		NumDungeonGirls++;
		temp2 = temp2->m_Next;
		i++;
	}

	Brothel1Index = NumBrothelGirls0;
	Brothel2Index = NumBrothelGirls0 + NumBrothelGirls1;
	Brothel3Index = NumBrothelGirls0 + NumBrothelGirls1 + NumBrothelGirls2;
	Brothel4Index = NumBrothelGirls0 + NumBrothelGirls1 + NumBrothelGirls2 + NumBrothelGirls3;
	Brothel5Index = NumBrothelGirls0 + NumBrothelGirls1 + NumBrothelGirls2 + NumBrothelGirls3 + NumBrothelGirls4;
	Brothel6Index = NumBrothelGirls0 + NumBrothelGirls1 + NumBrothelGirls2 + NumBrothelGirls3 + NumBrothelGirls4 + NumBrothelGirls5;

	StudioIndex = NumBrothelGirls;
	ArenaIndex = NumBrothelGirls + NumStudioGirls;
	CentreIndex = NumBrothelGirls + NumStudioGirls + NumArenaGirls;
	ClinicIndex = NumBrothelGirls + NumStudioGirls + NumArenaGirls + NumCentreGirls;
	FarmIndex = NumBrothelGirls + NumStudioGirls + NumArenaGirls + NumCentreGirls + NumClinicGirls;
	HouseIndex = NumBrothelGirls + NumStudioGirls + NumArenaGirls + NumCentreGirls + NumClinicGirls + NumFarmGirls;
	DungeonIndex = NumBrothelGirls + NumStudioGirls + NumArenaGirls + NumCentreGirls + NumClinicGirls + NumFarmGirls + NumHouseGirls;

	g_ReturnText = "";
	g_AllTogle = false;

	DisableButton(shift_l_id, true);
	DisableButton(shift_r_id, true);

	SetSelectedItemInList(owners_l_id, leftOwner);
	SetSelectedItemInList(owners_r_id, rightOwner);

	SetSelectedItemInList(items_l_id, leftItem);
	SetSelectedItemInList(items_r_id, rightItem);

	// disable the equip/unequip buttons
	DisableButton(equip_l_id, true);
	DisableButton(unequip_l_id, true);
	DisableButton(equip_r_id, true);
	DisableButton(unequip_r_id, true);
}

void cScreenItemManagement::process()
{
	if (!ids_set) set_ids();		// we need to make sure the ID variables are set
	if (check_keys()) return;		// handle arrow keys
	init();							// set up the window if needed
	check_events();					// check to see if there's a button event needing handling
}

bool cScreenItemManagement::check_keys()
{
	if (g_AltKeys)
	{
		if (g_R_Key)	{ selection = ArrowUpListBox(filter_id);		g_R_Key = false;	return true; }
		if (g_F_Key)	{ selection = ArrowDownListBox(filter_id);		g_F_Key = false;	return true; }
		if (g_T_Key)	{ selection = ArrowUpListBox(owners_l_id);		g_T_Key = false;	return true; }
		if (g_G_Key)	{ selection = ArrowDownListBox(owners_l_id);	g_G_Key = false;	return true; }
		if (g_Y_Key)	{ selection = ArrowUpListBox(owners_r_id);		g_Y_Key = false;	return true; }
		if (g_H_Key)	{ selection = ArrowDownListBox(owners_r_id);	g_H_Key = false;	return true; }
		if (g_U_Key)	{ selection = ArrowUpListBox(items_l_id);		g_U_Key = false;	return true; }
		if (g_J_Key)	{ selection = ArrowDownListBox(items_l_id);		g_J_Key = false;	return true; }
		if (g_I_Key)	{ selection = ArrowUpListBox(items_r_id);		g_I_Key = false;	return true; }
		if (g_K_Key)	{ selection = ArrowDownListBox(items_r_id);		g_K_Key = false;	return true; }
	}
	return false;
}

void cScreenItemManagement::check_events()
{
	if (g_InterfaceEvents.GetNumEvents() == 0) return;	// no events means we can go home

	if (g_InterfaceEvents.CheckButton(back_id))			// if it's the back button, pop the window off the stack and we're done
	{
		sel_pos_l = -2;
		sel_pos_r = -2;
		g_InitWin = true;
		g_WinManager.Pop();
		return;
	}
	if (g_InterfaceEvents.CheckButton(shift_r_id))		{ attempt_transfer(Left); }
	if (g_InterfaceEvents.CheckButton(shift_l_id))		{ attempt_transfer(Right); }
	if (g_InterfaceEvents.CheckListbox(owners_l_id))	{ refresh_item_list(Left); }
	if (g_InterfaceEvents.CheckListbox(owners_r_id))	{ refresh_item_list(Right); }

	if (g_InterfaceEvents.CheckCheckbox(autouse_id))	AutoUseItems = IsCheckboxOn(autouse_id);

	if (g_InterfaceEvents.CheckListbox(items_l_id))
	{
		int selection = GetLastSelectedItemFromList(items_l_id);
		DisableButton(shift_r_id, (selection < 0));

		if (selection != -1)
		{
			stringstream ss;
			ss << gettext("Cost: ");
			if (leftOwner == 0)	//Shop
			{
				ss << gettext("Value: ") << g_Brothels.m_Inventory[selection]->m_Cost << gettext(" gold    ");
				ss << gettext("Sell for: ") << int((float)g_Brothels.m_Inventory[selection]->m_Cost*0.5f) << gettext(" gold\n");
				ss << gettext("Item Name: ") << g_Brothels.m_Inventory[selection]->m_Name;
				ss << gettext("\n") << g_Brothels.m_Inventory[selection]->m_Desc;

				DisableButton(equip_l_id, true);
				DisableButton(unequip_l_id, true);
			}
			else if (leftOwner == 1)	//Player
			{
				ss << g_InvManager.GetShopItem(selection)->m_Cost << gettext(" gold\n");
				ss << gettext("Item Name: ") << g_InvManager.GetShopItem(selection)->m_Name;
				ss << "\n" << g_InvManager.GetShopItem(selection)->m_Desc;

				DisableButton(equip_l_id, true);
				DisableButton(unequip_l_id, true);
			}
			else
			{
				sGirl* targetGirl = 0;
				targetGirl = GirlSelectedFromList(leftOwner);

				HateLove = g_Girls.GetStat(targetGirl, STAT_PCLOVE) - g_Girls.GetStat(targetGirl, STAT_PCHATE);

				ss << gettext("Value: ") << targetGirl->m_Inventory[selection]->m_Cost << gettext(" gold    ");
				ss << gettext("Sell for: ") << int((float)targetGirl->m_Inventory[selection]->m_Cost*0.5f) << gettext(" gold\n");
				ss << gettext("Item Name: ") << targetGirl->m_Inventory[selection]->m_Name;
				ss << gettext("\n") << targetGirl->m_Inventory[selection]->m_Desc;

				if (g_InvManager.IsItemEquipable(targetGirl->m_Inventory[selection]))
				{
					DisableButton(equip_l_id, (targetGirl->m_EquipedItems[selection] == 1));
					DisableButton(unequip_l_id, !(targetGirl->m_EquipedItems[selection] == 1));
				}
				else
				{
					DisableButton(equip_l_id, true);
					DisableButton(unequip_l_id, true);
				}
			}
			EditTextItem(ss.str(), desc_id);
		}
	}
	if (g_InterfaceEvents.CheckListbox(items_r_id))
	{
		int selection = GetLastSelectedItemFromList(items_r_id);

		DisableButton(shift_l_id, (selection < 0));

		if (selection != -1)
		{
			stringstream ss;
			ss << gettext("Cost: ");
			if (rightOwner == 0)
			{
				ss << gettext("Value: ") << g_Brothels.m_Inventory[selection]->m_Cost << gettext(" gold    ");
				ss << gettext("Sell for: ") << int((float)g_Brothels.m_Inventory[selection]->m_Cost*0.5f) << gettext(" gold\n");
				ss << gettext("Item Name: ") << g_Brothels.m_Inventory[selection]->m_Name;
				ss << gettext("\n") << g_Brothels.m_Inventory[selection]->m_Desc;

				DisableButton(equip_r_id, true);
				DisableButton(unequip_r_id, true);
			}
			else if (rightOwner == 1)
			{
				ss << g_InvManager.GetShopItem(selection)->m_Cost << gettext(" gold\n");
				ss << gettext("Item Name: ") << g_InvManager.GetShopItem(selection)->m_Name;
				ss << "\n" << g_InvManager.GetShopItem(selection)->m_Desc;

				DisableButton(equip_r_id, true);
				DisableButton(unequip_r_id, true);
			}
			else
			{
				sGirl* targetGirl = 0;
				targetGirl = GirlSelectedFromList(rightOwner);

				HateLove = g_Girls.GetStat(targetGirl, STAT_PCLOVE) - g_Girls.GetStat(targetGirl, STAT_PCHATE);

				ss << gettext("Value: ") << targetGirl->m_Inventory[selection]->m_Cost << gettext(" gold    ");
				ss << gettext("Sell for: ") << int((float)targetGirl->m_Inventory[selection]->m_Cost*0.5f) << gettext(" gold\n");
				ss << gettext("Item Name: ") << targetGirl->m_Inventory[selection]->m_Name;
				ss << gettext("\n") << targetGirl->m_Inventory[selection]->m_Desc;

				if (g_InvManager.IsItemEquipable(targetGirl->m_Inventory[selection]))
				{
					DisableButton(equip_r_id, (targetGirl->m_EquipedItems[selection] == 1));
					DisableButton(unequip_r_id, !(targetGirl->m_EquipedItems[selection] == 1));
				}
				else
				{
					DisableButton(equip_r_id, true);
					DisableButton(unequip_r_id, true);
				}
			}
			EditTextItem(ss.str(), desc_id);
		}
	}
	if (g_InterfaceEvents.CheckListbox(filter_id))
	{
		int selection = GetLastSelectedItemFromList(filter_id);
		filter = selection;
		SetSelectedItemInList(owners_l_id, leftOwner);
		SetSelectedItemInList(owners_r_id, rightOwner);
	}
	if (g_InterfaceEvents.CheckButton(equip_l_id))
	{
		sGirl* targetGirl = 0;
		targetGirl = GirlSelectedFromList(leftOwner);

		HateLove = g_Girls.GetStat(targetGirl, STAT_PCLOVE) - g_Girls.GetStat(targetGirl, STAT_PCHATE);

		int selection = GetLastSelectedItemFromList(items_l_id);
		if (selection != -1)
		{
			g_InvManager.Equip(targetGirl, selection, true);
			DisableButton(equip_l_id, true);
			DisableButton(unequip_l_id, false);
			SetSelectedItemInList(owners_l_id, leftOwner);
			SetSelectedItemInList(owners_r_id, rightOwner);
		}
	}
	if (g_InterfaceEvents.CheckButton(unequip_l_id))
	{
		sGirl* targetGirl = 0;
		targetGirl = GirlSelectedFromList(leftOwner);

		HateLove = g_Girls.GetStat(targetGirl, STAT_PCLOVE) - g_Girls.GetStat(targetGirl, STAT_PCHATE);

		int selection = GetLastSelectedItemFromList(items_l_id);
		if (selection != -1)
		{
			g_InvManager.Unequip(targetGirl, selection);
			DisableButton(equip_l_id, false);
			DisableButton(unequip_l_id, true);
			SetSelectedItemInList(owners_l_id, leftOwner);
			SetSelectedItemInList(owners_r_id, rightOwner);
		}
	}
	if (g_InterfaceEvents.CheckButton(equip_r_id))
	{
		sGirl* targetGirl = 0;
		targetGirl = GirlSelectedFromList(rightOwner);

		HateLove = g_Girls.GetStat(targetGirl, STAT_PCLOVE) - g_Girls.GetStat(targetGirl, STAT_PCHATE);

		int selection = GetLastSelectedItemFromList(items_r_id);
		if (selection != -1)
		{
			g_InvManager.Equip(targetGirl, selection, true);
			DisableButton(equip_r_id, true);
			DisableButton(unequip_r_id, false);
			SetSelectedItemInList(owners_l_id, leftOwner);
			SetSelectedItemInList(owners_r_id, rightOwner);
		}
	}
	if (g_InterfaceEvents.CheckButton(unequip_r_id))
	{
		sGirl* targetGirl = 0;
		targetGirl = GirlSelectedFromList(rightOwner);

		HateLove = g_Girls.GetStat(targetGirl, STAT_PCLOVE) - g_Girls.GetStat(targetGirl, STAT_PCHATE);

		int selection = GetLastSelectedItemFromList(items_r_id);
		if (selection != -1)
		{
			g_InvManager.Unequip(targetGirl, selection);
			DisableButton(equip_r_id, false);
			DisableButton(unequip_r_id, true);
			SetSelectedItemInList(owners_l_id, leftOwner);
			SetSelectedItemInList(owners_r_id, rightOwner);
		}
	}
}

void cScreenItemManagement::refresh_item_list(Side which_list)
{
	// good enough place as any to update the cost shown on the screen
	string temp = gettext("PLAYER GOLD: ");
	temp += g_Gold.sval();
	EditTextItem(temp, gold_id);

	int item_list, owner_list, *sel_pos, *owner, *other_owner;
	string *sel_name;
	if (which_list == Left)
	{
		item_list = items_l_id;
		owner_list = owners_l_id;
		owner = &leftOwner;
		other_owner = &rightOwner;
		sel_pos = &sel_pos_l;
		sel_name = &sel_name_l;
	}
	else
	{
		item_list = items_r_id;
		owner_list = owners_r_id;
		owner = &rightOwner;
		other_owner = &leftOwner;
		sel_pos = &sel_pos_r;
		sel_name = &sel_name_r;
	}

	ClearListBox(item_list);
	int selection = GetSelectedItemFromList(owner_list);
	if (selection == *other_owner) SetSelectedItemInList(owner_list, *owner);
	else if (selection != -1)
	{
		*owner = selection;

		if (*owner == 0)	// player items
		{
			for (int i = 0; i < MAXNUM_INVENTORY; i++)
			{
				int ItemColor = -1;
				if (g_Brothels.m_Inventory[i])
				{
					stringstream it;
					it << g_Brothels.m_Inventory[i]->m_Name;
					if (*sel_name == it.str()) *sel_pos = i;  // if we just transferred this item here, might want to select it
					it << gettext(" (") << g_Brothels.m_NumItem[i] << gettext(")");
					int item_type = g_Brothels.m_Inventory[i]->m_Type;
					if ((filter == 0)  // unfiltered?
						|| (item_type == filter)  // matches filter exactly?
						|| ((filter == INVFOOD) && (item_type == INVMAKEUP))  // passes "consumable" filter?
						)
					{  // passed the filter, so add it
						if (*sel_name == g_Brothels.m_Inventory[i]->m_Name) *sel_pos = i;  // if we just transferred this item here, might want to select it
						AddToListBox(item_list, i, it.str());
						ItemColor = g_Brothels.m_Inventory[i]->m_Rarity;
					}
				}
				if (ItemColor > -1) SetSelectedItemTextColor(item_list, i, RarityColor[ItemColor]);
			}
		}
		else if (*owner == 1)	// shop items
		{
			for (int i = 0; i<NUM_SHOPITEMS; i++)
			{
				int ItemColor = -1;
				if (g_InvManager.GetShopItem(i))
				{
					int item_type = g_InvManager.GetShopItem(i)->m_Type;
					if ((filter == 0)  // unfiltered?
						|| (item_type == filter)  // matches filter exactly?
						|| ((filter == INVFOOD) && (item_type == INVMAKEUP))  // passes "consumable" filter?
						)
					{  // passed the filter, so add it
						if (*sel_name == g_InvManager.GetShopItem(i)->m_Name) *sel_pos = i;  // if we just transferred this item here, might want to select it
						AddToListBox(item_list, i, g_InvManager.GetShopItem(i)->m_Name);
						ItemColor = g_InvManager.GetShopItem(i)->m_Rarity;
					}
				}
				if (ItemColor > -1) SetSelectedItemTextColor(item_list, i, RarityColor[ItemColor]);
			}
		}
		else	// girl items
		{
			sGirl* targetGirl = 0;
			targetGirl = GirlSelectedFromList(*owner);

			for (int i = 0; i<40; i++)
			{
				int ItemColor = -1;
				if (targetGirl->m_Inventory[i])
				{
					int item_type = targetGirl->m_Inventory[i]->m_Type;
					if ((filter == 0)  // unfiltered?
						|| (item_type == filter)  // matches filter exactly?
						|| ((filter == INVFOOD) && (item_type == INVMAKEUP))  // passes "consumable" filter?
						)
					{  // passed the filter, so add it
						if (*sel_name == targetGirl->m_Inventory[i]->m_Name) *sel_pos = i;  // if we just transferred this item here, might want to select it
						AddToListBox(item_list, i, targetGirl->m_Inventory[i]->m_Name);
						ItemColor = targetGirl->m_Inventory[i]->m_Rarity;
					}
				}
				if (ItemColor > -1) SetSelectedItemTextColor(item_list, i, RarityColor[ItemColor]);
			}
		}
	}
	SortListItems(item_list, "");

	*sel_name = "";
	SetSelectedItemInList(item_list, *sel_pos);

	if (which_list == Left)
	{
		DisableButton(equip_r_id, true);
		DisableButton(unequip_r_id, true);
		if (GetSelectedItemFromList(item_list) < 0) SetSelectedItemInList(item_list, sel_next_l);
		leftItem = GetSelectedItemFromList(item_list);
		sel_next_l = -2;
	}
	else
	{
		DisableButton(equip_l_id, true);
		DisableButton(unequip_l_id, true);
		if (GetSelectedItemFromList(item_list) < 0) SetSelectedItemInList(item_list, sel_next_r);
		rightItem = GetSelectedItemFromList(item_list);
		sel_next_r = -2;
	}

	if (GetLastSelectedItemFromList(item_list) < 0)
	{
		EditTextItem("", desc_id);
		DisableButton((which_list == Left) ? shift_r_id : shift_l_id, true);
	}
}

void cScreenItemManagement::attempt_transfer(Side transfer_from)
{
	leftOwner = GetSelectedItemFromList(owners_l_id);
	rightOwner = GetSelectedItemFromList(owners_r_id);

	if (rightOwner == -1 || leftOwner == -1) return;

	int source_list, source_owner_list, source_owner, target_owner_list, target_owner;
	string *item_name;
	if (transfer_from == Left)
	{
		source_list			= items_l_id;
		source_owner_list	= owners_l_id;
		source_owner		= leftOwner;
		target_owner_list	= owners_r_id;
		target_owner		= rightOwner;
		item_name			= &sel_name_r;
	}
	else
	{
		source_list			= items_r_id;
		source_owner_list	= owners_r_id;
		source_owner		= rightOwner;
		target_owner_list	= owners_l_id;
		target_owner		= leftOwner;
		item_name			= &sel_name_l;
	}

	leftItem = GetLastSelectedItemFromList(items_l_id);
	rightItem = GetLastSelectedItemFromList(items_r_id);
	if (target_owner == 1)	// target is shop
	{
		sGirl* targetGirl = 0;
		if (source_owner > 1)	// taking from a girl and selling to shop
		{
			int selectedfromlist = GetSelectedItemFromList(source_owner_list);
			targetGirl = GirlSelectedFromList(source_owner, selectedfromlist);

			HateLove = g_Girls.GetStat(targetGirl, STAT_PCLOVE) - g_Girls.GetStat(targetGirl, STAT_PCHATE);

			// take items and transfer to shop, giving money to player
			int pos = 0;
			int selection = GetNextSelectedItemFromList(source_list, 0, pos);
			while (selection != -1)
			{
				if (targetGirl->m_EquipedItems[selection] == 1)	// unequip item if it is equiped
					g_InvManager.Unequip(targetGirl, selection);
				if (targetGirl->m_Inventory[selection]->m_Badness >= 20)	// happy to get rid of bad items
					g_Girls.UpdateStat(targetGirl, STAT_HAPPINESS, 5);
				else	// sad to see good items go
				{
					int happiness = g_InvManager.HappinessFromItem(targetGirl->m_Inventory[selection]);
					g_Girls.UpdateStat(targetGirl, STAT_HAPPINESS, -happiness);
				}

				// add the gold
				long gold = (int)(targetGirl->m_Inventory[selection]->m_Cost*0.5f);
				g_Gold.item_sales(gold);

				// since items sold to shop are simply destroyed, no selection to track here
				//				*item_name = targetGirl->m_Inventory[selection]->m_Name;  // note name of item, for selection tracking in target list

				// remove the item
				targetGirl->m_Inventory[selection] = 0;
				targetGirl->m_EquipedItems[selection] = 0;
				targetGirl->m_NumInventory--;

				selection = GetNextSelectedItemFromList(source_list, pos + 1, pos);
			}
		}
		else	// player selling to shop
		{
			// sell to shop, giving money to player
			int pos = 0;
			int selection = GetNextSelectedItemFromList(source_list, 0, pos);
			while (selection != -1)
			{
				// since items sold to shop are simply destroyed, no selection to track here
				//				*item_name = g_Brothels.m_Inventory[selection]->m_Name;  // note name of item, for selection tracking in target list

				long gold = (int)((float)g_Brothels.m_Inventory[selection]->m_Cost*0.5f);
				g_Gold.item_sales(gold);
				g_Brothels.m_NumItem[selection]--;
				if (g_Brothels.m_NumItem[selection] == 0)
				{
					g_Brothels.m_Inventory[selection] = 0;
					g_Brothels.m_EquipedItems[selection] = 0;
					g_Brothels.m_NumInventory--;
				}
				selection = GetNextSelectedItemFromList(source_list, pos + 1, pos);
			}
		}
	}
	else if (target_owner == 0) // target is player
	{
		if (source_owner == 1)	// buying from shop
		{
			int pos = 0;
			int selection = GetNextSelectedItemFromList(source_list, 0, pos);
			while (selection != -1)
			{
				int cost = g_InvManager.GetShopItem(selection)->m_Cost;
				if (g_Gold.afford(cost))
				{
					*item_name = g_InvManager.GetShopItem(selection)->m_Name;  // note name of item, for selection tracking in target list

					if (g_Brothels.AddItemToInventory(g_InvManager.GetShopItem(selection)))
					{
						long gold = (int)(g_InvManager.GetShopItem(selection)->m_Cost);
						g_InvManager.BuyShopItem(selection);
						g_Gold.item_cost(gold);
					}
					else
					{
						*item_name = "";
						g_MessageQue.AddToQue(gettext("Your inventory is full."), 1);
						break;
					}
				}
				else
				{
					g_MessageQue.AddToQue(gettext("You don't have enough gold."), 1);
					break;
				}

				selection = GetNextSelectedItemFromList(source_list, pos + 1, pos);
			}
		}
		else	// taking from girl
		{
			// take items from girl and give to player
			sGirl* targetGirl = 0;
			int selectedfromlist = GetSelectedItemFromList(source_owner_list);
			targetGirl = GirlSelectedFromList(source_owner, selectedfromlist);

			HateLove = g_Girls.GetStat(targetGirl, STAT_PCLOVE) - g_Girls.GetStat(targetGirl, STAT_PCHATE);

			int pos = 0;
			int selection = GetNextSelectedItemFromList(source_list, 0, pos);
			while (selection != -1)
			{
				// Add the item to players inventory
				if (g_Brothels.AddItemToInventory(targetGirl->m_Inventory[selection]) == false)
				{
					g_MessageQue.AddToQue(gettext("Your inventory is full."), 1);
					break;
				}

				if (targetGirl->m_EquipedItems[selection] == 1)	// unequip item if it is equiped
					g_InvManager.Unequip(targetGirl, selection);
				if (targetGirl->m_Inventory[selection]->m_Badness >= 20)	// happy to get rid of bad items
					g_Girls.UpdateStat(targetGirl, STAT_HAPPINESS, 5);
				else	// sad to see good items go
				{
					int happiness = g_InvManager.HappinessFromItem(targetGirl->m_Inventory[selection]);
					g_Girls.UpdateStat(targetGirl, STAT_HAPPINESS, -happiness);
				}

				*item_name = targetGirl->m_Inventory[selection]->m_Name;  // note name of item, for selection tracking in target list

				// remove the item from the girl
				targetGirl->m_Inventory[selection] = 0;
				targetGirl->m_EquipedItems[selection] = 0;
				targetGirl->m_NumInventory--;

				selection = GetNextSelectedItemFromList(source_list, pos + 1, pos);
			}
		}
	}
	else	// target is girl
	{
		sGirl* targetGirl = 0;
		int selectedfromlist = GetSelectedItemFromList(target_owner_list);
		targetGirl = GirlSelectedFromList(target_owner, selectedfromlist);

		HateLove = g_Girls.GetStat(targetGirl, STAT_PCLOVE) - g_Girls.GetStat(targetGirl, STAT_PCHATE);

		if (source_owner == 0)	// player giving gift to the girl
		{
			int pos = 0;
			int selection = GetNextSelectedItemFromList(source_list, 0, pos);
			while (selection != -1)
			{
				if (g_Girls.IsInvFull(targetGirl))
				{
					g_MessageQue.AddToQue(gettext("Her inventory is full"), 0);
					break;
				}

				int goodbad = g_Brothels.m_Inventory[selection]->m_Badness;
				*item_name = g_Brothels.m_Inventory[selection]->m_Name;  // note name of item, for selection tracking in target list
				u_int type = g_Brothels.m_Inventory[selection]->m_Type;
				g_MessageQue.AddToQue(GiveItemText(goodbad, HateLove), 0);

				if (goodbad < 20)
				{
					int happiness = g_InvManager.HappinessFromItem(g_Brothels.m_Inventory[selection]);

					g_Girls.UpdateStat(targetGirl, STAT_OBEDIENCE, 1);
					g_Girls.UpdateStat(targetGirl, STAT_HAPPINESS, happiness);
					g_Girls.UpdateStat(targetGirl, STAT_PCHATE, -2);
					g_Girls.UpdateStat(targetGirl, STAT_PCLOVE, happiness - 1);
					g_Girls.UpdateStat(targetGirl, STAT_PCFEAR, -1);
				}

				if (!AutoUseItems && (type == INVFOOD || type == INVMAKEUP)) 
					g_Girls.AddInv(targetGirl, g_Brothels.m_Inventory[selection]);
				else 
					g_InvManager.Equip(targetGirl, g_Girls.AddInv(targetGirl, g_Brothels.m_Inventory[selection]), false);

				g_Brothels.m_NumItem[selection]--;
				if (g_Brothels.m_NumItem[selection] == 0)
				{
					g_Brothels.m_Inventory[selection] = 0;
					g_Brothels.m_EquipedItems[selection] = 0;
					g_Brothels.m_NumInventory--;
				}

				selection = GetNextSelectedItemFromList(source_list, pos + 1, pos);
			}
		}
		else if (source_owner == 1)	// player giving gift straight from the shop
		{
			int pos = 0;
			int selection = GetNextSelectedItemFromList(source_list, 0, pos);
			while (selection != -1)
			{
				if (g_Girls.IsInvFull(targetGirl))
				{
					g_MessageQue.AddToQue(gettext("Her inventory is full"), 0);
					break;
				}

				sInventoryItem *ShopItem = g_InvManager.GetShopItem(selection);
				int cost = ShopItem->m_Cost;

				// can player afford the item
				if (!g_Gold.item_cost(cost)) {
					g_MessageQue.AddToQue(gettext("You don't have enough money."), 0);
					break;
				}

				*item_name = ShopItem->m_Name;  // note name of item, for selection tracking in target list
				int goodbad = ShopItem->m_Badness;
				u_int type = ShopItem->m_Type;
				g_MessageQue.AddToQue(GiveItemText(goodbad, HateLove), 0);

				if (ShopItem->m_Badness < 20)
				{
					int happiness = g_InvManager.HappinessFromItem(ShopItem);
					g_Girls.UpdateStat(targetGirl, STAT_OBEDIENCE, 1);
					g_Girls.UpdateStat(targetGirl, STAT_HAPPINESS, happiness);
					g_Girls.UpdateStat(targetGirl, STAT_PCHATE, -2);
					g_Girls.UpdateStat(targetGirl, STAT_PCLOVE, happiness - 1);
					g_Girls.UpdateStat(targetGirl, STAT_PCFEAR, -1);
				}

				if (!AutoUseItems && (type == INVFOOD || type == INVMAKEUP)) g_Girls.AddInv(targetGirl, ShopItem);
				else g_InvManager.Equip(targetGirl, g_Girls.AddInv(targetGirl, ShopItem), false);
				g_InvManager.BuyShopItem(selection);

				selection = GetNextSelectedItemFromList(source_list, pos + 1, pos);
			}
		}
		else	// player forcing a girl to give to another girl
		{
			sGirl* fromGirl = 0;
			int selectedfromlist = GetSelectedItemFromList(source_owner_list);
			fromGirl = GirlSelectedFromList(source_owner, selectedfromlist);

			int HateLove = g_Girls.GetStat(targetGirl, STAT_PCLOVE) - g_Girls.GetStat(targetGirl, STAT_PCHATE);

			int pos = 0;
			int selection = GetNextSelectedItemFromList(source_list, 0, pos);
			while (selection != -1)
			{
				if (g_Girls.IsInvFull(targetGirl))
				{
					g_MessageQue.AddToQue(gettext("Her inventory is full"), 0);
					break;
				}

				if (fromGirl->m_EquipedItems[selection] == 1) g_InvManager.Unequip(fromGirl, selection);	// unequip item if it is equiped

				int happiness = g_InvManager.HappinessFromItem(fromGirl->m_Inventory[selection]);
				*item_name = fromGirl->m_Inventory[selection]->m_Name;  // note name of item, for selection tracking in target list
				int goodbad = fromGirl->m_Inventory[selection]->m_Badness;
				u_int type = fromGirl->m_Inventory[selection]->m_Type;
				g_MessageQue.AddToQue(GiveItemText(goodbad, HateLove), 0);

				// add to target Girls inventory
				if (goodbad < 20)
				{
					g_Girls.UpdateStat(fromGirl, STAT_HAPPINESS, -happiness);  // previous owner sad to lose it

					g_Girls.UpdateStat(targetGirl, STAT_OBEDIENCE, 1);
					g_Girls.UpdateStat(targetGirl, STAT_HAPPINESS, happiness);
					g_Girls.UpdateStat(targetGirl, STAT_PCHATE, -2);
					g_Girls.UpdateStat(targetGirl, STAT_PCLOVE, happiness - 1);
					g_Girls.UpdateStat(targetGirl, STAT_PCFEAR, -1);
				}
				else
				{
					g_Girls.UpdateStat(fromGirl, STAT_HAPPINESS, 5);  // previous owner happy to see it go
				}

				if (!AutoUseItems && (type == INVFOOD || type == INVMAKEUP)) g_Girls.AddInv(targetGirl, fromGirl->m_Inventory[selection]);
				else g_InvManager.Equip(targetGirl, g_Girls.AddInv(targetGirl, fromGirl->m_Inventory[selection]), false);

				// remove the item from the girl
				fromGirl->m_Inventory[selection] = 0;
				fromGirl->m_EquipedItems[selection] = 0;
				fromGirl->m_NumInventory--;

				selection = GetNextSelectedItemFromList(source_list, pos + 1, pos);
			}
		}
	}

	//set these for re-selection of same items if they're still there
	int pos = 0;
	sel_pos_l = GetNextSelectedItemFromList(items_l_id, 0, pos);
	sel_pos_r = GetNextSelectedItemFromList(items_r_id, 0, pos);
	sel_next_l = GetAfterSelectedItemFromList(items_l_id);
	sel_next_r = GetAfterSelectedItemFromList(items_r_id);

	// update the item lists
	SetSelectedItemInList(source_owner_list, source_owner);
	SetSelectedItemInList(target_owner_list, target_owner);
}

string cScreenItemManagement::GiveItemText(int goodbad, int HateLove, string ItemName)
{
	string message = "";
	if (goodbad < 20)
	{
		/* */if (HateLove < -80)	message = gettext("She grudgingly accepts the gift, but makes it clear that she still thinks that you rate slightly below a toad in her worldview.");
		else if (HateLove < -60)	message = gettext("She takes your gift in hand, looks at it, looks at you, than walks away without a word.");
		else if (HateLove < -40)	message = gettext("You know, if you wanted to fuck, you shoulda just said.  So that way I'd have had more fun saying no.");
		else if (HateLove < -20)	message = gettext("If you think giving me pretty things will get you between my legs, you're wrong!");
		else if (HateLove < 0)		message = gettext("She is shocked you would give her anything nice.");
		else if (HateLove < 20)		message = gettext("She is happy with the gift.");
		else if (HateLove < 40)		message = gettext("She is happy with the gift and thanks you.");
		else if (HateLove < 60)		message = gettext("She is happy with the gift and gives you a big hug.");
		else if (HateLove < 80)		message = gettext("She is happy with the gift and gives you a big hug and a kiss on the cheek.");
		else /*               */	message = gettext("She is happy with the gift and gives you a big hug and kisses you hard.  After the kiss she whispers to you to see her later so she can thank you \"properly\".");
	}
	else
	{
		/* */if (HateLove < -80)	message = gettext("'Wow, what's next?  A dead rat?  Your shit sucks almost as bad as your technique!");
		else if (HateLove < -60)	message = gettext("She takes your gift in hand, looks at it, looks at you, than walks away without a word.");
		else if (HateLove < -40)	message = gettext("Hey, present for you!' 'Fuck off and die!");
		else if (HateLove < -20)	message = gettext("'Fuck off, and take your cheap garbage with you!");
		else if (HateLove < 0)		message = gettext("She doesn't seem happy with the gift.  But has come to expect this kinda thing from you.");
		else if (HateLove < 20)		message = gettext("She doesn't seem happy with the gift.");
		else if (HateLove < 40)		message = gettext("She doesn't seem happy with the gift and looks a little sad.");
		else if (HateLove < 60)		message = gettext("She doesn't seem happy with the gift and looks sad.");
		else if (HateLove < 80)		message = gettext("She doesn't seem happy with the gift and tears can be seen in her eyes.");
		else /*              */		message = gettext("She can't belive you would give her such a gift and runs off crying.");
	}
	return message;
}

// `J` This if/else check was written out in several places - combined into one.
sGirl* cScreenItemManagement::GirlSelectedFromList(int owner, int selectedfromlist)
{
	sGirl* targetGirl;
	if (selectedfromlist == -100)	selectedfromlist = owner;

	if (owner <= NumBrothelGirls0 + 1)
		targetGirl = g_Brothels.GetGirl(0, selectedfromlist - NumberSpecialSlots);
	else if (owner <= NumBrothelGirls0 + NumBrothelGirls1 + 1)
		targetGirl = g_Brothels.GetGirl(1, (selectedfromlist - Brothel1Index) - NumberSpecialSlots);
	else if (owner <= NumBrothelGirls0 + NumBrothelGirls1 + NumBrothelGirls2 + 1)
		targetGirl = g_Brothels.GetGirl(2, (selectedfromlist - Brothel2Index) - NumberSpecialSlots);
	else if (owner <= NumBrothelGirls0 + NumBrothelGirls1 + NumBrothelGirls2 + NumBrothelGirls3 + 1)
		targetGirl = g_Brothels.GetGirl(3, (selectedfromlist - Brothel3Index) - NumberSpecialSlots);
	else if (owner <= NumBrothelGirls0 + NumBrothelGirls1 + NumBrothelGirls2 + NumBrothelGirls3 + NumBrothelGirls4 + 1)
		targetGirl = g_Brothels.GetGirl(4, (selectedfromlist - Brothel4Index) - NumberSpecialSlots);
	else if (owner <= NumBrothelGirls0 + NumBrothelGirls1 + NumBrothelGirls2 + NumBrothelGirls3 + NumBrothelGirls4 + NumBrothelGirls5 + 1)
		targetGirl = g_Brothels.GetGirl(5, (selectedfromlist - Brothel5Index) - NumberSpecialSlots);
	else if (owner <= NumBrothelGirls0 + NumBrothelGirls1 + NumBrothelGirls2 + NumBrothelGirls3 + NumBrothelGirls4 + NumBrothelGirls5 + NumBrothelGirls6 + 1)
		targetGirl = g_Brothels.GetGirl(6, (selectedfromlist - Brothel6Index) - NumberSpecialSlots);

	else if (owner <= NumBrothelGirls + NumStudioGirls + 1)
		targetGirl = g_Studios.GetGirl(0, (selectedfromlist - StudioIndex) - NumberSpecialSlots);
	else if (owner <= NumBrothelGirls + NumStudioGirls + NumArenaGirls + 1)
		targetGirl = g_Arena.GetGirl(0, (selectedfromlist - ArenaIndex) - NumberSpecialSlots);
	else if (owner <= NumBrothelGirls + NumStudioGirls + NumArenaGirls + NumCentreGirls + 1)
		targetGirl = g_Centre.GetGirl(0, (selectedfromlist - CentreIndex) - NumberSpecialSlots);
	else if (owner <= NumBrothelGirls + NumStudioGirls + NumArenaGirls + NumCentreGirls + NumClinicGirls + 1)
		targetGirl = g_Clinic.GetGirl(0, (selectedfromlist - ClinicIndex) - NumberSpecialSlots);
	else if (owner <= NumBrothelGirls + NumStudioGirls + NumArenaGirls + NumCentreGirls + NumClinicGirls + NumFarmGirls + 1)
		targetGirl = g_Farm.GetGirl(0, (selectedfromlist - FarmIndex) - NumberSpecialSlots);
	else if (owner <= NumBrothelGirls + NumStudioGirls + NumArenaGirls + NumCentreGirls + NumClinicGirls + NumFarmGirls + NumHouseGirls + 1)
		targetGirl = g_House.GetGirl(0, (selectedfromlist - HouseIndex) - NumberSpecialSlots);
	else
		targetGirl = (g_Brothels.GetDungeon()->GetGirl((selectedfromlist - DungeonIndex) - NumberSpecialSlots))->m_Girl;

	return targetGirl;
}