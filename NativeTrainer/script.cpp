
#include "script.h"
#include "keyboard.h"
#include <string>
#include <ctime>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <math.h>
//#include <stdarg.h>  // For va_start, etc.
#include <memory>    // For std::unique_ptr

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define TOGGLESTRING(x) ((x) == TRUE) ? "ON":"OFF"
#define TOGGLECOLOUR(x) ((x) == TRUE) ? "HUD_COLOUR_BLUE":"HUD_COLOUR_RED"

#pragma warning(disable : 4244 4305) // double <-> float conversions

#pragma  region vars
enum Controls{

	DPAD_DOWN = 187,
	DPAD_UP = 188,
	DPAD_RIGHT = 190,
	DPAD_LEFT = 189,
	START = 199,
	SELETE = 217,
	X = 201,
	Square = 203,
	Circle = 202,
	Tringle = 204,
	L1 = 205,
	R1 = 206,
	L2 = 207,
	R2 = 208,
	L3 = 209,
	R3 = 210,
	R3Down = 198,

	//Vehicle buttons
	keyDefbrakecontrol = R3Down,
	keyDefEngineSwitch = L1,
	keyDeleavevehicle = Tringle,
	keyDefCruiseControl = X,
	keyDefAlertLight = Square,
	keyDefInteriorSwitch = Circle,
	keyDeftrungsigsleftSwitch = DPAD_LEFT,
	keyDeftrungsigsrightSwitch = DPAD_RIGHT,
	keyDefHoodSwitch = L3,
	keyDefTrunkSwitch = R3,
	keyDefDoorswitch = R1,
	keyDefPressDelay = 500,
};

//Vehicle features
DWORD keyPressDelay = 0;
BOOL leftLight = FALSE;
BOOL rightLight = FALSE;
BOOL interiorLight = FALSE;
BOOL engineSwitch = FALSE;
BOOL hoodOpen = FALSE;
BOOL trunkOpen = FALSE;
BOOL cruising = FALSE;
FLOAT cruiseSpeed = 0.00f;
FLOAT lastSpeed = 0.00f;
BOOL DoorsOpen = FALSE;
BOOL bptyres = FALSE;
BOOL SuperBrake = FALSE;
float maxSpeedLeaveVehicle = 11.17;

//Crime
int onVehHit = 0;
int onHitPed = 0;
int onPavement = 0;
int InTraffic = 0;

// Fuel
int TimeDelay = 150;
int model = 0;
float MPH = 0;
float Knots = 0;
int MPG = 1000;//10000 Miles Per Gallon 
float Fuel = 0; //1000
int fuelup = 0;
int fuelin = 0;
float FuelMultiplier = 1;
bool InVehicle = false;
Vehicle MyVehicle = 0;
Vehicle Myplane = 0;
Vehicle Myboat = 0;
Blip BLIPID;
Vector3 MyLocation = {MyLocation.x, MyLocation.y, MyLocation.z};
#pragma  endregion
//pumps
int PumpIndex = 0;
bool InPumpArea = false;
float PetrolStation1Main_X[] = { 819.689, -525.790, -319.491, -2095.137, -725.480, -69.350, 265.827, 1208.911, 2681.797, -2554.940, 164.120, 2575.100, 1702, -1803, -95, 264, 50, 2537, 1179, 1208.268, 2004, 620.418, -1434.871, 1686 };//Center Of Station
float PetrolStation1Main_Y[] = { -1028.501, -1211.11, -1471.874, -319.594, -936.261, -1760.805, -1259.653, 2660.857, 3264.397, 2333.943, 6591, 359, 6418, 803, 6415, 2609, 2776, 2593, -333, -1402.594, 3771, 269.266, -274.544, 4930 };
float PetrolStation1Main_Z[] = { 26.404, 17.804, 30.168, 12.56, 18.740, 29.098, 28.820, 37.197, 55.054, 32.500, 12.15, 29.0, 25.12, 25.12, 21.2, 12.51, 22.12, 21.12, 21.15, 34.941, 20.15, 102.950, 45.847, 15.12 };
int NumberOfStations = 24;
int StationType[] = { 1, 2, 1 };//1 = 6 Pumps, 2 = 8 Pumps 3 = 4 Pumps , 4 = 9 pump  5 = 2 pumps

#pragma region ExtensionFunctions
BOOL isPressed(int Button)
{
	return CONTROLS::IS_CONTROL_JUST_PRESSED(2, Button);
}

BOOL isDown(int Button)
{
	return CONTROLS::IS_DISABLED_CONTROL_PRESSED(0, Button);
}

bool get_key_pressed(int nVirtKey){

	return (isDown(nVirtKey) || (GetAsyncKeyState(nVirtKey) & 0x8000) != 0);
}

void setKeyPressDelay(){
	keyPressDelay = GetTickCount() + keyDefPressDelay;
}

void NotifyAboveMap(char* Messg){
	UI::_0x202709F4C58A0424("STRING");
	UI::_ADD_TEXT_COMPONENT_STRING(Messg);
	UI::_0x2ED7843F8F801023(0, 1);
}

std::string string_format(const std::string fmt_str, ...)
{
	int final_n, n = ((int)fmt_str.size()) * 2; /* Reserve two times as much as the length of the fmt_str */
	std::string str;
	std::unique_ptr<char[]> formatted;
	va_list ap;
	while (1) {
		formatted.reset(new char[n]); /* Wrap the plain char array into the unique_ptr */
		strcpy(&formatted[0], fmt_str.c_str());
		va_start(ap, fmt_str);
		final_n = vsnprintf(&formatted[0], n, fmt_str.c_str(), ap);
		va_end(ap);
		if (final_n < 0 || final_n >= n)
			n += abs(final_n - n + 1);
		else
			break;
	}
	return std::string(formatted.get());
}

bool PLAY_ANIM(int Entity, char* AnimDict, char* Anim){

	STREAMING::REQUEST_ANIM_DICT(AnimDict);
	if (STREAMING::HAS_ANIM_DICT_LOADED(AnimDict)){

		AI::TASK_PLAY_ANIM(Entity, AnimDict, Anim, 8, -8, -1, 0, 0, 0, 0, true);
		return true;
	}
	else
		return false;
}

void DRAW_TEXT(char* Text, float X, float Y, float S_X, float S_Y, int Font, bool Outline, bool Shadow, bool Center, bool RightJustify, int R, int G, int B, int A)
{
	UI::SET_TEXT_FONT(Font);
	UI::SET_TEXT_SCALE(S_X, S_Y);
	UI::SET_TEXT_COLOUR(R, G, B, A);
	if (Outline)
	{
		UI::SET_TEXT_OUTLINE();
	}
	if (Shadow)
	{
		UI::SET_TEXT_DROP_SHADOW();
	}
	if (Center)
	{
		UI::SET_TEXT_CENTRE(1);
	}
	if (RightJustify)
	{
		UI::SET_TEXT_RIGHT_JUSTIFY(1);
	}
	UI::_SET_TEXT_ENTRY("STRING");
	UI::_ADD_TEXT_COMPONENT_STRING(Text);
	UI::_DRAW_TEXT(X, Y);
}

void draw_rect(float posX, float posY, float scaleX, float scaleY, int R, int B, int G, int A){

	GRAPHICS::DRAW_RECT((posX + (scaleX * 0.5f)), (posY + (scaleY * 0.5f)), scaleX, scaleY, R, B, G, A);
}

unsigned int getrandomnumb(int nlow, int nhigh){
	return (rand() % (nhigh - nlow + 1)) + nlow;
}

std::string statusText;
DWORD statusTextDrawTicksMax;
bool statusTextGxtEntry;
void update_status_text(){

	if (GetTickCount() < statusTextDrawTicksMax){
		UI::SET_TEXT_FONT(0);
		UI::SET_TEXT_SCALE(0.55, 0.55);
		UI::SET_TEXT_COLOUR(255, 255, 255, 255);
		UI::SET_TEXT_WRAP(0.0, 1.0);
		UI::SET_TEXT_CENTRE(1);
		UI::SET_TEXT_DROPSHADOW(0, 0, 0, 0, 0);
		UI::SET_TEXT_EDGE(1, 0, 0, 0, 205);
		if (statusTextGxtEntry){
			UI::_SET_TEXT_ENTRY((char *)statusText.c_str());
		}
		else{
			UI::_SET_TEXT_ENTRY("STRING");
			UI::_ADD_TEXT_COMPONENT_STRING((char *)statusText.c_str());
		}
		UI::_DRAW_TEXT(0.5, 0.5);
	}
}

void set_status_text(std::string str, DWORD time = 2500, bool isGxtEntry = false){
	statusText = str;
	statusTextDrawTicksMax = GetTickCount() + time;
	statusTextGxtEntry = isGxtEntry;
}
#pragma endregion

void FixnWashVehicle(UINT Vehicle){

	VEHICLE::SET_VEHICLE_FIXED(Vehicle);
	VEHICLE::SET_VEHICLE_DEFORMATION_FIXED(Vehicle);
	VEHICLE::SET_VEHICLE_DIRT_LEVEL(Vehicle, 0.0f);
}

void Draw(){
	
	if (PED::IS_PED_IN_ANY_VEHICLE(PLAYER::PLAYER_PED_ID(), false)){
#pragma region Fuel
		float FuelX = 0.0625 + 0.005;
		float Width = 0.150;

		GRAPHICS::DRAW_RECT(FuelX, 0.8, Width + 0.001, 0.011, 199, 85, 94, 255);
		GRAPHICS::DRAW_RECT(FuelX - (Width - ((Width / 1000) * Fuel)), 0.8, (Width / 1000) * Fuel, 0.01, 46, 204, 113, 255);
		
		/*char Distance2[100];
		sprintf_s(Distance2, sizeof(Distance2), "%s%f", "Test: ", maxtraction);
		DRAW_TEXT(Distance2, FuelX - 0.065, 0.65, 0.35, 0.35, 7, false, false, false, false, 95, 180, 255, 255);*/
	
		char xyzCoordsOut[100];// Show Coords
		sprintf_s(xyzCoordsOut, "X= %.3f / Y= %.3f / Z= %.3f", MyLocation.x, MyLocation.y, MyLocation.z);
		DRAW_TEXT(xyzCoordsOut, FuelX - 0.065, 0.69, 0.45, 0.45, 6, false, false, false, false, 95, 180, 255, 255);
		set_status_text(xyzCoordsOut);


		char Speed[100];
		sprintf_s(Speed, sizeof(Speed), "%s%f", "MPH: ", MPH);
		DRAW_TEXT(Speed, FuelX - 0.067, 0.726, 0.35, 0.35, 8, false, false, false, false, 75, 204, 255, 255);
	

		char msg[100];
		sprintf_s(msg, sizeof(msg), "%s%f", "Fuel: ", Fuel);
		DRAW_TEXT(msg, FuelX-0.013, 0.75, 0.35, 0.35, 7, true, false, true, false, 95, 180, 113, 255);


		float MilesRemaining = Fuel * MPG;
		char MilesRemaining1[100];
		sprintf_s(MilesRemaining1, sizeof(MilesRemaining1), "%s%f", "Fuel Miles Left: ", MilesRemaining);
		DRAW_TEXT(MilesRemaining1, FuelX+0.032, 0.77, 0.35, 0.35, 7, true, false, true, false, 199, 204, 113, 255);
#pragma endregion	
		if (InPumpArea){
			DRAW_TEXT("Press Left on Remote or 4 to Refill Vehicle", 0.0625 + 0.005, 0.74, 0.35, 0.35, 7, true, false, true, false, 199, 204, 113, 255);
		}
	}
	else{
		if (InPumpArea){
			DRAW_TEXT("Press Left on Remote or 4 to Refill Petrol Can", 0.0625 + 0.005, 0.74, 0.35, 0.35, 7, true, false, true, false, 199, 204, 113, 255);
		}
	}
}

bool CheckIndividualPump(int StationIndex, float Offset_X, float Offset_Y)
{
	if (ENTITY::IS_ENTITY_IN_AREA(PLAYER::PLAYER_PED_ID(), PetrolStation1Main_X[StationIndex] + Offset_X + 2, PetrolStation1Main_Y[StationIndex] + Offset_Y + 2, PetrolStation1Main_Z[StationIndex] + 5, PetrolStation1Main_X[StationIndex] + Offset_X - 2, PetrolStation1Main_Y[StationIndex] + Offset_Y - 2, PetrolStation1Main_Z[StationIndex] - 5, false, true, false))//Is at station
		return true;
	else
		return false;
}
bool CheckPetrolPumps(int StationIndex, int& PumpIndex)
{
	if (ENTITY::IS_ENTITY_IN_AREA(PLAYER::PLAYER_PED_ID(), PetrolStation1Main_X[StationIndex] + 20, PetrolStation1Main_Y[StationIndex] + 20, PetrolStation1Main_Z[StationIndex] + 5, PetrolStation1Main_X[StationIndex] - 20, PetrolStation1Main_Y[StationIndex] - 20, PetrolStation1Main_Z[StationIndex] - 5, false, true, false))//Is at station
	{
		//Check if in area of a specific pump
		if (CheckIndividualPump(StationIndex, 0, 2))//Center Pump 1
		{
			PumpIndex = 1;
			return true;
		}
		else if (CheckIndividualPump(StationIndex, 0, -2))//Center Pump 2
		{
			PumpIndex = 2;
			return true;
		}
		else if (CheckIndividualPump(StationIndex, 7, 2))//Back Pump 1
		{
			PumpIndex = 3;
			return true;
		}
		else if (CheckIndividualPump(StationIndex, 7, -2))//Back Pump 2
		{
			PumpIndex = 4;
			return true;
		}
		else if (CheckIndividualPump(StationIndex, -7, 2))//Front Pump 1
		{
			PumpIndex = 5;
			return true;
		}
		else if (CheckIndividualPump(StationIndex, -7, -2))//Front Pump 2
		{
			PumpIndex = 6;
			return true;
		}
		else
			return false;
	}
}

void drivingupdates(){

#pragma region Drivesafe

	Ped MyPed = PLAYER::PLAYER_PED_ID();
	Vehicle Current = PED::GET_VEHICLE_PED_IS_IN(MyPed, false);
	BOOL invehicle = PED::IS_PED_IN_ANY_VEHICLE(MyPed, 0);
	unsigned int modelId = ENTITY::GET_ENTITY_MODEL(Current);
	BOOL isPlayerDriver = (VEHICLE::GET_PED_IN_VEHICLE_SEAT(Current, -1) == MyPed);

	Player ped = PLAYER::GET_PLAYER_PED(MyPed);
	onVehHit = PLAYER::GET_TIME_SINCE_PLAYER_HIT_VEHICLE(ped);
	onHitPed = PLAYER::GET_TIME_SINCE_PLAYER_HIT_PED(ped);
	onPavement = PLAYER::GET_TIME_SINCE_PLAYER_DROVE_ON_PAVEMENT(ped);
	InTraffic = PLAYER::GET_TIME_SINCE_PLAYER_DROVE_AGAINST_TRAFFIC(ped);

	if (onVehHit < 1000 || InTraffic < 1000 < onPavement < 1000){
		PLAYER::SET_PLAYER_WANTED_LEVEL(ped, 1, TRUE);
		PLAYER::SET_PLAYER_WANTED_LEVEL_NOW(ped, TRUE);
	}
	if (PLAYER::IS_PLAYER_PRESSING_HORN(ped) || get_key_pressed(VK_NUMPAD4) || isDown(DPAD_RIGHT)){
		PLAYER::SET_PLAYER_WANTED_LEVEL(ped, 0, TRUE);
		PLAYER::SET_PLAYER_WANTED_LEVEL_NOW(ped, TRUE);
	}
	#pragma endregion
#pragma  region  vehicle controls
	//Kill vehicle engine.
	if (engineSwitch)
		VEHICLE::SET_VEHICLE_ENGINE_ON(Current, FALSE, FALSE);
	//Cruising control.
	if (cruiseSpeed != 0.00f){
		float actualSpeed = ENTITY::GET_ENTITY_SPEED(Current);
		if (lastSpeed == 0.0f)
			lastSpeed = actualSpeed;
		if (invehicle && (get_key_pressed(R2))){
			// Player is controlling the vehicle, lay down for a second.
		}
		else if (invehicle && get_key_pressed(keyDefCruiseControl)){
			cruiseSpeed = 0.00f;
			lastSpeed = 0.00f;
			NotifyAboveMap((char *)string_format("Cruise Control - <C>~%s~%s</C>", TOGGLECOLOUR(cruising), TOGGLESTRING(cruising)).c_str());
		}
		else if ((abs(lastSpeed) - abs(actualSpeed)) >= MIN(10, abs(lastSpeed * 0.2))){

			cruiseSpeed = 0.00f;
			lastSpeed = 0.00f;
			NotifyAboveMap((char *)string_format("Cruise Control - <C>~%s~%s</C>", TOGGLECOLOUR(cruising), TOGGLESTRING(cruising)).c_str());
		}
		else if ((cruiseSpeed != actualSpeed) && (ENTITY::GET_ENTITY_HEIGHT_ABOVE_GROUND(Current) < 1)){

			int increment = cruiseSpeed - actualSpeed;
			increment = MAX(MIN(increment, 1), -1);
			lastSpeed = actualSpeed;
			VEHICLE::SET_VEHICLE_FORWARD_SPEED(Current, actualSpeed + increment);
		}
	}

	//Vehicle turning indicators controller.
	if (get_key_pressed(keyDefAlertLight)){
		setKeyPressDelay();
		if (leftLight != rightLight){

			leftLight = rightLight = TRUE;
		}
		else if (leftLight == TRUE){
			leftLight = rightLight = FALSE;
		}
		else{
			leftLight = rightLight = TRUE;
		}
		VEHICLE::SET_VEHICLE_INDICATOR_LIGHTS(Current, TRUE, leftLight);
		VEHICLE::SET_VEHICLE_INDICATOR_LIGHTS(Current, FALSE, rightLight);
	}
	else if (get_key_pressed(keyDeftrungsigsleftSwitch) || get_key_pressed(keyDeftrungsigsrightSwitch)){
		setKeyPressDelay();
		rightLight = get_key_pressed(keyDeftrungsigsrightSwitch) ? !rightLight : FALSE;
		leftLight = get_key_pressed(keyDeftrungsigsleftSwitch) ? !leftLight : FALSE;
		VEHICLE::SET_VEHICLE_INDICATOR_LIGHTS(Current, TRUE, leftLight);
		VEHICLE::SET_VEHICLE_INDICATOR_LIGHTS(Current, FALSE, rightLight);
	}

	//Vehicle Brake controller
	if (get_key_pressed(keyDefbrakecontrol)){
		setKeyPressDelay();
		SuperBrake = !SuperBrake;
		if (SuperBrake)
			VEHICLE::SET_VEHICLE_FORWARD_SPEED(Current, 0.0f);
	}

	//Vehicle door controller
	if (get_key_pressed(keyDefDoorswitch)){
		setKeyPressDelay();
		DoorsOpen = !DoorsOpen;
		if (DoorsOpen){
			VEHICLE::SET_VEHICLE_DOOR_OPEN(Current, 1, FALSE, FALSE);
			VEHICLE::SET_VEHICLE_DOOR_OPEN(Current, 2, FALSE, FALSE);
			VEHICLE::SET_VEHICLE_DOOR_OPEN(Current, 3, FALSE, FALSE);
		}
		else{
			VEHICLE::SET_VEHICLE_DOOR_SHUT(Current, 1, TRUE);
			VEHICLE::SET_VEHICLE_DOOR_SHUT(Current, 2, TRUE);
			VEHICLE::SET_VEHICLE_DOOR_SHUT(Current, 3, TRUE);
		}
	}
	//Vehicle hood controller
	if (get_key_pressed(keyDefHoodSwitch)){
		setKeyPressDelay();
		hoodOpen = !hoodOpen;
		if (hoodOpen)
			VEHICLE::SET_VEHICLE_DOOR_OPEN(Current, 4, FALSE, FALSE);
		else
			VEHICLE::SET_VEHICLE_DOOR_SHUT(Current, 4, TRUE);
		NotifyAboveMap((char *)string_format("Hood - <C>~%s~%s</C>", TOGGLECOLOUR(hoodOpen), TOGGLESTRING(hoodOpen)).c_str());
	}
	//Vehicle trunk controller.
	if (get_key_pressed(keyDefTrunkSwitch)){
		setKeyPressDelay();
		trunkOpen = !trunkOpen;
		BOOL force = !VEHICLE::IS_THIS_MODEL_A_CAR(modelId);

		if (trunkOpen)
			VEHICLE::SET_VEHICLE_DOOR_OPEN(Current, 5, TRUE, force);
		else
			VEHICLE::SET_VEHICLE_DOOR_SHUT(Current, 5, TRUE);
		NotifyAboveMap((char *)string_format("Trunk - <C>~%s~%s</C>", TOGGLECOLOUR(trunkOpen), TOGGLESTRING(trunkOpen)).c_str());
	}

	//Vehicle interior lights controller.
	if (get_key_pressed(keyDefInteriorSwitch)){
		setKeyPressDelay();
		interiorLight = !interiorLight;
		VEHICLE::SET_VEHICLE_INTERIORLIGHT(Current, interiorLight);

		NotifyAboveMap((char *)string_format("Interior Light - <C>~%s~%s</C>", TOGGLECOLOUR(interiorLight), TOGGLESTRING(interiorLight)).c_str());
	}

	//Vehicle engine controller.
	if (get_key_pressed(keyDefEngineSwitch)){
		setKeyPressDelay();
		engineSwitch = !engineSwitch;
		// The player AI will turn on the engines alone in cars.
		if (!engineSwitch || !VEHICLE::IS_THIS_MODEL_A_CAR(modelId) || !VEHICLE::IS_THIS_MODEL_A_QUADBIKE(modelId))
			VEHICLE::SET_VEHICLE_ENGINE_ON(Current, engineSwitch, engineSwitch);
		NotifyAboveMap((char *)string_format("Engine - <C>~%s~%s</C>", TOGGLECOLOUR(engineSwitch), TOGGLESTRING(engineSwitch)).c_str());
	}

	//Vehicle cruise control.
	if (get_key_pressed(keyDefCruiseControl)){
		setKeyPressDelay();
		cruising = !cruising;
		if (cruiseSpeed == 0.00f){
			cruiseSpeed = ENTITY::GET_ENTITY_SPEED(Current);
			lastSpeed = cruiseSpeed;
			NotifyAboveMap((char *)string_format("Cruise Control - <C>~%s~%s</C>", TOGGLECOLOUR(cruising), TOGGLESTRING(cruising)).c_str());
		}
		else{
			cruiseSpeed = 0.00f;
			lastSpeed = 0.00f;
			NotifyAboveMap((char *)string_format("Cruise Control - <C>~%s~%s</C>", TOGGLECOLOUR(cruising), TOGGLESTRING(cruising)).c_str());
		}
	}
	//Vehicle leaving features.
	if (get_key_pressed(keyDeleavevehicle) && ENTITY::GET_ENTITY_SPEED(Current) < maxSpeedLeaveVehicle){

		int flags = 0;
		//If the player is just a passenger, it will let the door open.
		if (!isPlayerDriver || get_key_pressed(keyDeleavevehicle))
			flags = 1 << 8;
		AI::TASK_LEAVE_VEHICLE(MyPed, Current, flags);

		//If the player is the vehicle driver, let the engine running.
		if (isPlayerDriver){
			WAIT(5);
			VEHICLE::SET_VEHICLE_ENGINE_ON(Current, TRUE, TRUE);
		}
	}

	//Mods 
	//Bullet proof tires
	if (get_key_pressed(keyDefEngineSwitch && keyDefCruiseControl)){
		setKeyPressDelay();
		bptyres = !bptyres;
		if (bptyres)
			VEHICLE::SET_VEHICLE_TYRES_CAN_BURST(MyPed, FALSE);
		else
			VEHICLE::SET_VEHICLE_TYRES_CAN_BURST(MyPed, TRUE);
		NotifyAboveMap((char *)string_format("Bulletprooftyres - <C>~%s~%s</C>", TOGGLECOLOUR(bptyres), TOGGLESTRING(bptyres)).c_str());
	}

	// Fix n Wash
	if (get_key_pressed(keyDeleavevehicle && keyDefCruiseControl)){
		setKeyPressDelay();
		FixnWashVehicle(Current);
	}

	// veh godmode
	if (get_key_pressed(keyDefCruiseControl)){
		setKeyPressDelay();
		//if (VehGod)
		ENTITY::SET_ENTITY_INVINCIBLE(Current, TRUE);
		//else
		//	ENTITY::SET_ENTITY_INVINCIBLE(veh, FALSE);
	}
#pragma  endregion
}

void FuelCore(){

	char* dict = "misscarsteal2peeing";
	char* anim = "peeing_loop";

	Ped MyPed = PLAYER::PLAYER_PED_ID();	
	if (PED::IS_PED_IN_ANY_VEHICLE(MyPed, false)){
		Vehicle Current = PED::GET_VEHICLE_PED_IS_IN(MyPed, false);
		
#pragma region Fuel
		if (VEHICLE::IS_THIS_MODEL_A_CAR(ENTITY::GET_ENTITY_MODEL(Current)) || VEHICLE::IS_THIS_MODEL_A_BIKE(ENTITY::GET_ENTITY_MODEL(Current)) || VEHICLE::IS_THIS_MODEL_A_QUADBIKE(ENTITY::GET_ENTITY_MODEL(Current)))
			if (Current != MyVehicle){
				MyVehicle = Current;
				VEHICLE::SET_VEHICLE_ENGINE_ON(Current, true, false);
				Fuel = VEHICLE::GET_VEHICLE_PETROL_TANK_HEALTH(Current);//1000
			}

		drivingupdates();
		
		// Speed To Miles
		MPH = ENTITY::GET_ENTITY_SPEED(Current) * 2.25;
		float Distancetmp = MPH / (MPG * FuelMultiplier);
		Fuel -= Distancetmp;

		//Health Affect Fuel
		float carHealth = VEHICLE::GET_VEHICLE_ENGINE_HEALTH(Current);//1000
		if (carHealth < 1000)
			Fuel -= MPH / ((MPG * FuelMultiplier) - (2 / carHealth));

		if (Fuel < 0){
			VEHICLE::SET_VEHICLE_PETROL_TANK_HEALTH(Current, Fuel);
			VEHICLE::SET_VEHICLE_ENGINE_ON(Current, false, false);
			VEHICLE::SET_VEHICLE_UNDRIVEABLE(Current, true);
			Fuel = 0;
		}
#pragma endregion
	}

#pragma region Pumps
	int Rand = rand();
	MyLocation = ENTITY::GET_ENTITY_COORDS(MyPed, false);
	float DistanceFromPetrolStation = 1000;
	int ClosestStation = 0;
	for (int i = BLIPID; i < NumberOfStations; i++)
	{
		float CurrentDistance = GAMEPLAY::GET_DISTANCE_BETWEEN_COORDS(MyLocation.x, MyLocation.y, MyLocation.z, PetrolStation1Main_X[i], PetrolStation1Main_Y[i], PetrolStation1Main_Z[i], true);
		if (CurrentDistance < DistanceFromPetrolStation)
		{
			DistanceFromPetrolStation = CurrentDistance;
			ClosestStation = i;
			UI::SET_BLIP_SPRITE(i, 0.10);
			UI::SET_BLIP_COLOUR(i, Rand);
			UI::SET_BLIP_DISPLAY(i, (char)"START");
			UI::SET_BLIP_AS_SHORT_RANGE(i, true);
		}
	}

	int PumpIndex = 0;
	if (CheckPetrolPumps(ClosestStation, PumpIndex)){//Check if your in area of a petrol pump
		InPumpArea = true;
		if (PED::IS_PED_IN_ANY_VEHICLE(MyPed, false)){//Petrol Pump
			if (get_key_pressed(VK_NUMPAD4) || isDown(DPAD_RIGHT))
			{
				if (PED::IS_PED_MODEL(MyPed, GAMEPLAY::GET_HASH_KEY("player_zero")))
				{
					model = 0;
				}
				if (PED::IS_PED_MODEL(MyPed, GAMEPLAY::GET_HASH_KEY("player_one")))
				{
					model = 1;
				}
				if (PED::IS_PED_MODEL(MyPed, GAMEPLAY::GET_HASH_KEY("player_two")))
				{
					model = 2;
				}
				char statNameFull[32];
				sprintf_s(statNameFull, "SP%d_TOTAL_CASH", model);
				Hash hash = GAMEPLAY::GET_HASH_KEY(statNameFull);
				int val;
				fuelup = getrandomnumb(100, 1000) * 2.45;
				fuelin = fuelup / 2.45;
				STATS::STAT_GET_INT(hash, &val, -1);
				val -= fuelup;
				STATS::STAT_SET_INT(hash, val, 1);
				Fuel = fuelin;

				/*AI::TASK_LEAVE_VEHICLE(MyPed, MyVehicle, 0);

				PLAY_ANIM(MyPed, dict, anim);

				WAIT(10000);
				Fuel = fuelin;
				AI::CLEAR_PED_TASKS_IMMEDIATELY(MyPed);
				AI::TASK_ENTER_VEHICLE(MyPed, MyVehicle, 100000, -1, 2, 1, 0);*/
			}
		}
		else{

			Hash PetrolCan = GAMEPLAY::GET_HASH_KEY("WEAPON_PETROLCAN");
			if (WEAPON::GET_CURRENT_PED_WEAPON(MyPed, &PetrolCan, true)){
				if (get_key_pressed(VK_NUMPAD4) || isDown(DPAD_RIGHT))
				{
					if (PED::IS_PED_MODEL(MyPed, GAMEPLAY::GET_HASH_KEY("player_zero"))){
						model = 0;
					}
					if (PED::IS_PED_MODEL(MyPed, GAMEPLAY::GET_HASH_KEY("player_one"))){
						model = 1;
					}
					if (PED::IS_PED_MODEL(MyPed, GAMEPLAY::GET_HASH_KEY("player_two"))){
						model = 2;
					}

					char statNameFull[32];
					printf_s(statNameFull, "SP%d_TOTAL_CASH", model);
					Hash hash = GAMEPLAY::GET_HASH_KEY(statNameFull);
					int val;
					int fuelup = getrandomnumb(100, 1000) * 2.45;
					int fuelin = fuelup / 2.45;
					STATS::STAT_GET_INT(hash, &val, -1);
					val -= fuelup;
					STATS::STAT_SET_INT(hash, val, 1);
					Fuel = fuelin;
					WEAPON::SET_PED_AMMO(MyPed, PetrolCan, fuelin);

					/*PLAY_ANIM(MyPed, dict, anim);*/
				}
			}
		}
	}
	else
		InPumpArea = false;
#pragma endregion
}

void ConstantCall(){

	Draw();
	FuelCore();
}
void Fuelmain(){
	
	int Rand = rand();
	for (int i = 0; i < NumberOfStations; i++){
		BLIPID = UI::ADD_BLIP_FOR_COORD(PetrolStation1Main_X[i], PetrolStation1Main_Y[i], PetrolStation1Main_Z[i]);
		UI::SET_BLIP_SPRITE(i, 0.10);
		UI::SET_BLIP_COLOUR(i, Rand);
	}

	while (true){
		DWORD maxTickCount = GetTickCount() + TimeDelay;
		do{
			ConstantCall();
			WAIT(0);
		} while (GetTickCount() < maxTickCount);
		TimeDelay = 0;
	}
}

void ScriptMain()
{
	srand(GetTickCount());
	Fuelmain();
}