#pragma once

#include "MapTools.h"

#include <BWAPI.h>



class StarterBot
{
    MapTools m_mapTools;

	BWAPI::Unit m_scout = nullptr;
	BWAPI::Position enemy_base_position;
	bool enemy_base = false;
	BWAPI::Position first_gateway_position;
	std::list<BWAPI::TilePosition> ourBases;
	std::list<BWAPI::Position> enemyBuildings;

	int totalZealots = 0;
	int maxWorkersGas;

	BWAPI::Position closeChokePoint;

	BWAPI::TilePosition averagePosition;

	std::list<BWAPI::TilePosition> scoutPositions;

	bool rushCompleted = false;
	bool earlyGameDone = false;
	bool upgradeInc = false;
	bool upgradeDone = false;

public:

    StarterBot();

    // helper functions to get you started with bot programming and learn the API
    void sendIdleWorkersToMinerals();
    void trainAdditionalWorkers();
    void buildAdditionalSupply();
    void drawDebugInformation();
	void startingScout();
	void sendIdleWorkersToGas();
	void followBuildOrderDT();
	bool trainUnit(BWAPI::UnitType unit);
	void attackOpponent();
	void gatherUnits(BWAPI::Unit unit);
	void fixWorkerSaturation();
	BWAPI::TilePosition findExpansions();
	void updateEnemyPositions();
	void followBuildOrder3GateRobo();
	bool makeUpgrade(BWAPI::UpgradeType upgrade);
	void buildAtExpansions();
	void attackNearbyEnemy(BWAPI::Unit unit);
	void makeWorkersWork();
	void followBuildOrder2Gate();
	void dragoonMicro();
	void getPhotonCannon();
	void attackOpponentDragoons();
	void spendMinerals();
	void observerMicro();
	void defendBase();
	void getTechBuildings();
	void lateGameScout();
	void scoutAll();
	void getScoutLocations();
	void followBuildOrder3GateSpeed();
	void getChokePointNear();
	void trainUnits();
	void zealotMicro();


    // functions that are triggered by various BWAPI events from main.cpp
	void onStart();
	void onFrame();
	void onEnd(bool isWinner);
	void onUnitDestroy(BWAPI::Unit unit);
	void onUnitMorph(BWAPI::Unit unit);
	void onSendText(std::string text);
	void onUnitCreate(BWAPI::Unit unit);
	void onUnitComplete(BWAPI::Unit unit);
	void onUnitShow(BWAPI::Unit unit);
	void onUnitHide(BWAPI::Unit unit);
	void onUnitRenegade(BWAPI::Unit unit);
};