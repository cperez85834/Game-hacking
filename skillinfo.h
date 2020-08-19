#include <winsock2.h>
//#include <dinput.h>
#include "dinput.h"
#include <Windows.h>
#include <fstream>
//#include <windows.h>
#include <iostream>
#include <string>
#include <Psapi.h>
#include <SetupAPI.h>
#include <initguid.h>

union IDtoBYTE
{
	short id;
	BYTE bytes[2];
};


LONGLONG globalCooldown = 0;
LONGLONG buffsCooldown = 0;
LONGLONG hpstoneCooldown = 0;
LONGLONG spstoneCooldown = 0;
LONGLONG potionCooldown = 0;

struct skill{
	float cooldown; //seconds
	BYTE id[2];
	LONGLONG tickAtCast = 0;
};
skill PowerShotr1;
skill MultiShotr1;
skill MultiShotr6;
skill MultiShot;
skill NatureMist;
skill PierceShot;
skill Bash;
skill Heal;
skill Protect;
skill Resist;
skill Bleed;
skill DB;
skill Endure;
skill Immune;
skill MC;
skill Extinguish;
skill Stoneskin;

//mage
skill MagicBolt;
skill FireBolt;
skill IceBolt;
skill LightBolt;
skill MagicBurst;
void initializeSkills_Mage(){
	IDtoBYTE magicbolty;

	magicbolty.id = 6000;

	MagicBolt.id[0] = magicbolty.bytes[0];
	MagicBolt.id[1] = magicbolty.bytes[1];
	MagicBolt.cooldown = 1.8;

	IDtoBYTE firebolty;

	firebolty.id = 6040;

	FireBolt.id[0] = firebolty.bytes[0];
	FireBolt.id[1] = firebolty.bytes[1];
	FireBolt.cooldown = 4.6;

	IDtoBYTE icebolty;

	icebolty.id = 6023;

	IceBolt.id[0] = icebolty.bytes[0];
	IceBolt.id[1] = icebolty.bytes[1];
	IceBolt.cooldown = 15.1;

	IDtoBYTE lightbolty;

	lightbolty.id = 6063;

	LightBolt.id[0] = lightbolty.bytes[0];
	LightBolt.id[1] = lightbolty.bytes[1];
	LightBolt.cooldown = 15.1;

	IDtoBYTE magicbursty;

	//magicbursty.id = 6146;
	magicbursty.id = 6142;

	MagicBurst.id[0] = magicbursty.bytes[0];
	MagicBurst.id[1] = magicbursty.bytes[1];
	MagicBurst.cooldown = 5.0;
}

void initializeSkills_Arch(){
	IDtoBYTE naturemisty;

	naturemisty.id = 4223;

	NatureMist.id[0] = naturemisty.bytes[0];
	NatureMist.id[1] = naturemisty.bytes[1];
	NatureMist.cooldown = 4.2;

	IDtoBYTE pierceshoty;

	pierceshoty.id = 4320;

	PierceShot.id[0] = pierceshoty.bytes[0];
	PierceShot.id[1] = pierceshoty.bytes[1];
	PierceShot.cooldown = 4.8;

	IDtoBYTE multishoty;

	multishoty.id = 4287;

	MultiShot.id[0] = multishoty.bytes[0];
	MultiShot.id[1] = multishoty.bytes[1];
	MultiShot.cooldown = 6.8;

	IDtoBYTE multishotr6y;

	multishotr6y.id = 4285;

	MultiShotr6.id[0] = multishotr6y.bytes[0];
	MultiShotr6.id[1] = multishotr6y.bytes[1];
	MultiShotr6.cooldown = 6.8;
}

//healbot
skill healbot_Heal;
skill healbot_Bash;
skill healbot_Restore;
skill healbot_Protect;
skill healbot_Resist;
skill healbot_Cure;
skill healbot_Endure;

void InitializeSkills_Healbot(){
	IDtoBYTE healy;
	healy.id = 1556;

	healbot_Heal.id[0] = healy.bytes[0];
	healbot_Heal.id[1] = healy.bytes[1];
	healbot_Heal.cooldown = 3.2;

	IDtoBYTE bashy;
	bashy.id = 1501;

	healbot_Bash.id[0] = bashy.bytes[0];
	healbot_Bash.id[1] = bashy.bytes[1];
	healbot_Bash.cooldown = 4;

	IDtoBYTE restorey;
	restorey.id = 1700;

	healbot_Restore.id[0] = restorey.bytes[0];
	healbot_Restore.id[1] = restorey.bytes[1];
	healbot_Restore.cooldown = 14;

	IDtoBYTE protecty;
	protecty.id = 1570;

	healbot_Protect.id[0] = protecty.bytes[0];
	healbot_Protect.id[1] = protecty.bytes[1];
	healbot_Protect.cooldown = 2.2;

	IDtoBYTE resisty;
	resisty.id = 1768;

	healbot_Resist.id[0] = resisty.bytes[0];
	healbot_Resist.id[1] = resisty.bytes[1];
	healbot_Resist.cooldown = 2.2;

	IDtoBYTE curey;
	curey.id = 1660;

	healbot_Cure.id[0] = curey.bytes[0];
	healbot_Cure.id[1] = curey.bytes[1];
	healbot_Cure.cooldown = 10;

	IDtoBYTE endurey;
	endurey.id = 1586;

	healbot_Endure.id[0] = endurey.bytes[0];
	healbot_Endure.id[1] = endurey.bytes[1];
	healbot_Endure.cooldown = 10;
}

void initializeSkills(){
	//skill PowerShotr1;
	PowerShotr1.id[0] = 0xb4;
	PowerShotr1.id[1] = 0x0f;
	PowerShotr1.cooldown = 10;

	MultiShotr1.id[0] = 0xb8;
	MultiShotr1.id[1] = 0x10;
	MultiShotr1.cooldown = 8;

	IDtoBYTE bashy;
	bashy.id = 1513;

	Bash.id[0] = bashy.bytes[0];
	Bash.id[1] = bashy.bytes[1];
	Bash.cooldown = 2.2;

	IDtoBYTE extinguishy;
	bashy.id = 1980;

	Extinguish.id[0] = extinguishy.bytes[0];
	Extinguish.id[1] = extinguishy.bytes[1];
	Extinguish.cooldown = 8;

	IDtoBYTE MCy;
	MCy.id = 2001;

	MC.id[0] = MCy.bytes[0];
	MC.id[1] = MCy.bytes[1];
	MC.cooldown = 10.5;

	IDtoBYTE bleedy;
	bleedy.id = 1647;

	Bleed.id[0] = bleedy.bytes[0];
	Bleed.id[1] = bleedy.bytes[1];
	Bleed.cooldown = 5.5;

	IDtoBYTE healy;
	healy.id = 1556;

	Heal.id[0] = healy.bytes[0];
	Heal.id[1] = healy.bytes[1];
	Heal.cooldown = 3.5;

	IDtoBYTE endurey;
	endurey.id = 1586;

	Endure.id[0] = endurey.bytes[0];
	Endure.id[1] = endurey.bytes[1];
	Endure.cooldown = 3;

	IDtoBYTE immuney;
	immuney.id = 1682;

	Immune.id[0] = immuney.bytes[0];
	Immune.id[1] = immuney.bytes[1];
	Immune.cooldown = 3;

	IDtoBYTE protecty;
	protecty.id = 1570;

	Protect.id[0] = protecty.bytes[0];
	Protect.id[1] = protecty.bytes[1];
	Protect.cooldown = 30;

	IDtoBYTE resisty;
	resisty.id = 1768;

	Resist.id[0] = resisty.bytes[0];
	Resist.id[1] = resisty.bytes[1];
	Resist.cooldown = 30;

	IDtoBYTE Stoneskiny;
	Stoneskiny.id = 1725;

	Stoneskin.id[0] = Stoneskiny.bytes[0];
	Stoneskin.id[1] = Stoneskiny.bytes[1];
	Stoneskin.cooldown = 30;

	IDtoBYTE DBy;
	DBy.id = 2070;

	DB.id[0] = DBy.bytes[0];
	DB.id[1] = DBy.bytes[1];
	DB.cooldown = 42;
}

skill HeroicStrike;
skill LightBlast;
skill Lifeline;
skill Advent;
skill LightStrike;
skill LightTouch;
skill Sunlight;
skill Moonlight;
skill Lightjump;

void initializeSkills_Crus(){
	int bot = 1;


	IDtoBYTE heroicstrikey;

	//bot 1
	if(bot == 1) heroicstrikey.id = 7356;

	//bot 2
	if (bot == 2) heroicstrikey.id = 7353;

	HeroicStrike.id[0] = heroicstrikey.bytes[0];
	HeroicStrike.id[1] = heroicstrikey.bytes[1];
	HeroicStrike.cooldown = 1.6;

	IDtoBYTE lightblasty;

	//bot 1
	if (bot == 1) lightblasty.id = 7405;

	//bot 2
	if (bot == 2) lightblasty.id = 7402;

	LightBlast.id[0] = lightblasty.bytes[0];
	LightBlast.id[1] = lightblasty.bytes[1];
	LightBlast.cooldown = 3;

	IDtoBYTE lifeliney;

	//bot 1
	if (bot == 1) lifeliney.id = 7441;

	//bot 2
	if (bot == 2) lifeliney.id = 7438;

	Lifeline.id[0] = lifeliney.bytes[0];
	Lifeline.id[1] = lifeliney.bytes[1];
	Lifeline.cooldown = 12;

	IDtoBYTE adventy;

	//bot 1
	if (bot == 1) adventy.id = 7369;

	//bot 2
	if (bot == 2) adventy.id = 7366;

	Advent.id[0] = adventy.bytes[0];
	Advent.id[1] = adventy.bytes[1];
	Advent.cooldown = 14;

	IDtoBYTE lightstrikey;

	//bot 1
	if (bot == 1) lightstrikey.id = 7380;

	//bot 2
	if (bot == 2) lightstrikey.id = 7377;

	LightStrike.id[0] = lightstrikey.bytes[0];
	LightStrike.id[1] = lightstrikey.bytes[1];
	LightStrike.cooldown = 3.5;

	IDtoBYTE lighttouchy;

	//bot 1
	if (bot == 1) lighttouchy.id = 7452;

	//bot 2
	if (bot == 2) lighttouchy.id = 7449;

	LightTouch.id[0] = lighttouchy.bytes[0];
	LightTouch.id[1] = lighttouchy.bytes[1];
	LightTouch.cooldown = 15;

	IDtoBYTE sunlighty;
	sunlighty.id = 7479;

	Sunlight.id[0] = sunlighty.bytes[0];
	Sunlight.id[1] = sunlighty.bytes[1];
	Sunlight.cooldown = 30;

	IDtoBYTE moonlighty;
	moonlighty.id = 7478;

	Moonlight.id[0] = moonlighty.bytes[0];
	Moonlight.id[1] = moonlighty.bytes[1];
	Moonlight.cooldown = 180;

	IDtoBYTE lightjumpy;
	lightjumpy.id = 7413;

	Lightjump.id[0] = lightjumpy.bytes[0];
	Lightjump.id[1] = lightjumpy.bytes[1];
	Lightjump.cooldown = 30;
}