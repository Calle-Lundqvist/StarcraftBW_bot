#include "StarterBot.h"
#include "Tools.h"
#include "MapTools.h"
#include "BWEM/src/bwem.h" 

//using namespace BWAPI;
//using namespace Filter;

using namespace BWEM;
using namespace BWEM::BWAPI_ext;
using namespace BWEM::utils;

namespace { auto& theMap = BWEM::Map::Instance(); }



StarterBot::StarterBot()
{
    
}

// Called when the bot starts!
void StarterBot::onStart()
{
    // Set our BWAPI options here    
	BWAPI::Broodwar->setLocalSpeed(10);
    BWAPI::Broodwar->setFrameSkip(0);
    
    // Enable the flag that tells BWAPI top let users enter input while bot plays
    BWAPI::Broodwar->enableFlag(BWAPI::Flag::UserInput);

    // Call MapTools OnStart
    m_mapTools.onStart();

    // ourBases.push_back(BWAPI::Broodwar->self()->getStartLocation());

    
    

    // BWEM
    try
    {
        BWAPI::Broodwar << "Map initialization..." << std::endl;

        theMap.Initialize();
        theMap.EnableAutomaticPathAnalysis();
        bool startingLocationsOK = theMap.FindBasesForStartingLocations();
        //assert(startingLocationsOK);



        
        BWAPI::Broodwar << "gg" << std::endl;

    }
    catch (const std::exception& e)
    {
        BWAPI::Broodwar << "EXCEPTION: " << e.what() << std::endl;
    }
    getChokePointNear();
    // getScoutLocations();

}

// Called whenever the game ends and tells you if you won or not
void StarterBot::onEnd(bool isWinner) 
{
    std::cout << "We " << (isWinner ? "won!" : "lost!") << "\n";
}

// Called on each frame of the game
void StarterBot::onFrame()
{
    // Update our MapTools information

    // std::cout << "Our base " << (ourBases.size()) << "\n";

    m_mapTools.onFrame();
    //auto curr_time = BWAPI::Broodwar->elapsedTime();
    //if (curr_time % 60 == 0){ std::cout << "Time is (min) " << (curr_time / 60) << "\n"; }

    // BWEM







    try
    {
        // BWEM::utils::gridMapExample(theMap);


        //BWAPI::Broodwar->drawTextScreen(200, 0, "FPS: %d", BWAPI::Broodwar->getFPS());
        //BWAPI::Broodwar->drawTextScreen(200, 20, "Average FPS: %f", BWAPI::Broodwar->getAverageFPS());
        // ...
    }
    catch (const std::exception& e)
    {
        BWAPI::Broodwar << "EXCEPTION: " << e.what() << std::endl;
    }


    // startingScout(); 
    startingScout();
    // const int supply = BWAPI::Broodwar->self()->supplyUsed();
    // std::cout << "Scout is " << (m_scout) << "\n";
    // Divide game into early game, mid game late game?

    // Early game, follow build order
    const int supply = BWAPI::Broodwar->self()->supplyUsed();
    int supplyCurrent = supply / 2;

    // std::cout << "Supply: " << (supply / 2) << "\n";

    // if (supply / 2 <= 50){ followBuildOrderDT(); }



    // **** SET BUILD ORDER HERE ****
    int buildOrderLimit = 30;
    if (supplyCurrent <= buildOrderLimit){ followBuildOrder3GateSpeed(); }
    
    // Check if we wanna attack
    const BWAPI::UnitType observer = BWAPI::UnitTypes::Protoss_Observer;
    const int observersOwned = Tools::CountUnitsOfType(observer, BWAPI::Broodwar->self()->getUnits());
    const BWAPI::UnitType dragoon = BWAPI::UnitTypes::Protoss_Dragoon;
    const int dragoonsOwned = Tools::CountUnitsOfType(dragoon, BWAPI::Broodwar->self()->getUnits());
    const BWAPI::UnitType darkTemplar = BWAPI::UnitTypes::Protoss_Dark_Templar;
    const int darkTemplarsOwned = Tools::CountUnitsOfType(darkTemplar, BWAPI::Broodwar->self()->getUnits());
    const BWAPI::UnitType zealot = BWAPI::UnitTypes::Protoss_Zealot;
    const int zealotsOwned = Tools::CountUnitsOfType(zealot, BWAPI::Broodwar->self()->getUnits());


    if ((zealotsOwned >= 4 || upgradeDone) && !rushCompleted) { attackOpponent(); }


    // When attack?

    if (dragoonsOwned >= 2 && !rushCompleted) { attackOpponentDragoons(); }

    if (totalZealots >= 30 && zealotsOwned < 5) { rushCompleted = true; }
    if (totalZealots >= 30) { earlyGameDone = true; }


    // Gather an army if rush failed
    if (rushCompleted)
    {
        for (auto& ourUnit : BWAPI::Broodwar->self()->getUnits())
        {
            if (!ourUnit->getType().isWorker() && !ourUnit->getType().isBuilding() && ourUnit->getDistance(closeChokePoint) < 300)
            {

                gatherUnits(ourUnit);
            }
        }
    }
    if (rushCompleted && supplyCurrent > 150)
    {
        attackOpponent();
        attackOpponentDragoons();
    }


    // To check if we are in midgame or smth
    int numForge = Tools::CountUnitsOfType(BWAPI::UnitTypes::Protoss_Forge, BWAPI::Broodwar->self()->getUnits());
    
    // Get enough zealots to survive opponent first push
 
    if (supplyCurrent > buildOrderLimit && !earlyGameDone)
    {
        // Prio 1, more zealots
        trainUnit(BWAPI::UnitTypes::Protoss_Zealot);
        // Prio 2, pylons!
        buildAdditionalSupply();
        // Get cyber
        if (Tools::CountUnitsOfType(BWAPI::UnitTypes::Protoss_Cybernetics_Core, BWAPI::Broodwar->self()->getUnits()) < 1)
        {
            Tools::BuildBuildingAtPosition(BWAPI::UnitTypes::Protoss_Cybernetics_Core, averagePosition);
        }
        // Get citadel
        if (Tools::CountUnitsOfType(BWAPI::UnitTypes::Protoss_Citadel_of_Adun, BWAPI::Broodwar->self()->getUnits()) < 1)
        {
            Tools::BuildBuilding(BWAPI::UnitTypes::Protoss_Citadel_of_Adun);
        }
        // Get third gateway
        if (Tools::CountUnitsOfType(BWAPI::UnitTypes::Protoss_Citadel_of_Adun, BWAPI::Broodwar->self()->getUnits()) >= 1 && Tools::CountUnitsOfType(BWAPI::UnitTypes::Protoss_Gateway, BWAPI::Broodwar->self()->getUnits()) < 3)
        {
            Tools::BuildBuilding(BWAPI::UnitTypes::Protoss_Gateway);
        }
        // Get zealot upgrade
        if (Tools::CountUnitsOfType(BWAPI::UnitTypes::Protoss_Citadel_of_Adun, BWAPI::Broodwar->self()->getUnits()) >= 1)
        {
            makeUpgrade(BWAPI::UpgradeTypes::Enum::Leg_Enhancements); 
            if (BWAPI::Broodwar->self()->isUpgrading(BWAPI::UpgradeTypes::Enum::Leg_Enhancements)) { upgradeInc = true; }
            if (upgradeInc && !BWAPI::Broodwar->self()->isUpgrading(BWAPI::UpgradeTypes::Enum::Leg_Enhancements)) { upgradeDone = true; }
        }



        //buildAdditionalSupply();
        // If we cant build more zealots but have money
        //if (!trainUnit(BWAPI::UnitTypes::Protoss_Zealot) && BWAPI::Broodwar->self()->minerals() > 100)
        //{
        //    getPhotonCannon();
        //}
    }
    if (earlyGameDone)
    {
        trainAdditionalWorkers();
        buildAdditionalSupply();
        trainUnits();
        // Get detection
        int numObservatory = Tools::CountUnitsOfType(BWAPI::UnitTypes::Protoss_Observatory, BWAPI::Broodwar->self()->getUnits());
        if (numObservatory < 1) { getTechBuildings(); }
        // Make sure we have observers
        int numObservers = Tools::CountUnitsOfType(BWAPI::UnitTypes::Protoss_Observer, BWAPI::Broodwar->self()->getUnits());
        if (numObservers < 2) { trainUnit(BWAPI::UnitTypes::Protoss_Observer); }


        if (BWAPI::Broodwar->self()->minerals() > 400) { spendMinerals(); }
    }

    // Make something with time instead of supply
    /*else if (numForge >= 1)
    {
        // Tech up! Maybe continue build order, get assim, cyber, robo, observ, etc.
        buildAdditionalSupply();
        

        // Train units first 
        if (numCybers >= 1)
        { 
            trainUnit(BWAPI::UnitTypes::Protoss_Dragoon);
        }
        else { trainUnit(BWAPI::UnitTypes::Protoss_Zealot); }



        // Make sure we have assimilator
        if (Tools::CountUnitsOfType(BWAPI::UnitTypes::Protoss_Assimilator, BWAPI::Broodwar->self()->getUnits()) < 1)
        {
        Tools::BuildBuildingAtPosition(BWAPI::UnitTypes::Protoss_Assimilator, averagePosition);
        }

        // Make sure we have cannons
        int numCannons = Tools::CountUnitsOfType(BWAPI::UnitTypes::Protoss_Photon_Cannon, BWAPI::Broodwar->self()->getUnits());
        if (numCannons < 2) { getPhotonCannon(); }
        int numObservatory = Tools::CountUnitsOfType(BWAPI::UnitTypes::Protoss_Observatory, BWAPI::Broodwar->self()->getUnits());
        if (numCannons > 1 && numObservatory < 1) { getTechBuildings(); }
        // Make sure we have observers
        int numObservers = Tools::CountUnitsOfType(BWAPI::UnitTypes::Protoss_Observer, BWAPI::Broodwar->self()->getUnits());
        if (numObservers < 2) { trainUnit(BWAPI::UnitTypes::Protoss_Observer); }


        

        if (numCybers >= 1 && BWAPI::Broodwar->self()->minerals() > 400) { spendMinerals(); }
        

    }*/

    // Pause making workers during the initial rush
    if (supplyCurrent > 16 && supplyCurrent < 29 || Tools::CountUnitsOfType(BWAPI::UnitTypes::Protoss_Citadel_of_Adun, BWAPI::Broodwar->self()->getUnits()) >= 1)
    {
        // Don't train workers
    }
    else
    {
        trainAdditionalWorkers();
    }

    // Max gas amount 250
    //if (BWAPI::Broodwar->self()->gatheredGas() <= 250) { maxWorkersGas = 3; }
    //else { maxWorkersGas = 0; }
    maxWorkersGas = 3;



    // Make workers always
    // trainAdditionalWorkers();
    /*if (supply / 2 > 60) 
    {
        // Build supply when needed


        

        buildAdditionalSupply();
        trainUnit(BWAPI::UnitTypes::Protoss_Dark_Templar);
        const BWAPI::UnitType gateway = BWAPI::UnitTypes::Protoss_Gateway;
        const int gatewaysOwned = Tools::CountUnitsOfType(gateway, BWAPI::Broodwar->self()->getUnits());
        if ((supply / 2) / gatewaysOwned > 15) { Tools::BuildBuilding(gateway); }
    }
    // Send our idle workers to mine minerals so they don't just stand there
    */

    // Make max 3 workers on each gas
    sendIdleWorkersToGas();

    sendIdleWorkersToMinerals();

    makeWorkersWork();
    
    // Make sure new base has important buildings, assimilator + pylon + photon cannon

    // defendBase();

    dragoonMicro();
    zealotMicro();

    observerMicro();
   

    // lateGameScout();



    // Draw unit health bars, which brood war unfortunately does not do
    Tools::DrawUnitHealthBars();

    // Draw some relevent information to the screen to help us debug the bot
    drawDebugInformation();
}


void StarterBot::trainUnits()
{
    const BWAPI::UnitType dragoon = BWAPI::UnitTypes::Protoss_Dragoon;
    const int dragoonsOwned = Tools::CountUnitsOfType(dragoon, BWAPI::Broodwar->self()->getUnits());
    const BWAPI::UnitType zealot = BWAPI::UnitTypes::Protoss_Zealot;
    const int zealotsOwned = Tools::CountUnitsOfType(zealot, BWAPI::Broodwar->self()->getUnits());
    
    if (dragoonsOwned >= zealotsOwned) { trainUnit(BWAPI::UnitTypes::Protoss_Zealot); }
    else { trainUnit(BWAPI::UnitTypes::Protoss_Dragoon); }


}

void StarterBot::lateGameScout()
{
    int frameNow = BWAPI::Broodwar->getFrameCount();
    if (frameNow < 5000) { return; }

    if (enemyBuildings.size() == 0)
    {
        scoutAll();
    }

}


void StarterBot::getScoutLocations()
{
    for (const Area& area : theMap.Areas())
    {
        for (const Base& base : area.Bases())
        {
            BWAPI::TilePosition tp = base.Location();
            scoutPositions.push_back(tp);
        }
    }
}

void StarterBot::getChokePointNear()
{
    BWAPI::TilePosition startPos = BWAPI::Broodwar->self()->getStartLocation();
    BWAPI::Position startPosition(startPos);
    float distance = 100000;
    BWAPI::Position closestPos;

    // Goes through ALL possible chokepoints
    for (const Area& area : theMap.Areas())
    {
        for (const auto& chokePoint : area.ChokePoints())
        {
            BWAPI::WalkPosition place = chokePoint->Center();
            BWAPI::Position pos(place);


            float distHere = startPosition.getDistance(pos);


            // std::cout << "Position " << (pos) << "\n";

            if (distHere < distance)
            {
                closestPos = pos;
                distance = distHere;
            }

        }
    }

    closeChokePoint = closestPos;

    return;
}



void StarterBot::scoutAll()
{
    if (!m_scout->exists()) { m_scout = nullptr; }
    if (!m_scout)
    {
        // Scout with first 
        m_scout = Tools::GetUnitOfType(BWAPI::UnitTypes::Protoss_Probe, nullptr);
    }

    for (auto& position : scoutPositions) 
    {
        BWAPI::Position pos(position);
        if (m_scout->getDistance(pos) < 50) { scoutPositions.remove(position); }
        BWAPI::Broodwar->drawCircleMap(pos, 32, BWAPI::Colors::Red, true);

        m_scout->move(pos);
        break;
    }


}




void StarterBot::spendMinerals()
{
    // Check if we want to expand
    const BWAPI::UnitType workerType = BWAPI::Broodwar->self()->getRace().getWorker();
    const int workersOwned = Tools::CountUnitsOfType(workerType, BWAPI::Broodwar->self()->getUnits());
    int basesOwned = ourBases.size();
    if (workersOwned / basesOwned >= 25)
    {
        BWAPI::TilePosition expandTo = findExpansions();

        BWAPI::Position pos(expandTo);

        BWAPI::Unitset thingsAtExpansion = BWAPI::Broodwar->getUnitsInRadius(pos, 300);

        bool nexusBeingBuilt = false;

        for (auto& thing : thingsAtExpansion)
        {
            if (thing->getType() == BWAPI::UnitTypes::Protoss_Nexus) { nexusBeingBuilt = true; }
        }

        if (!nexusBeingBuilt) { Tools::BuildBuildingAtPosition(BWAPI::UnitTypes::Protoss_Nexus, expandTo); }
        return; // Return ensures that we expand before moving down this function, ie that we have minerals to expand
    }

    

    // Build more gateways
    int numGateways = Tools::CountUnitsOfType(BWAPI::UnitTypes::Protoss_Gateway, BWAPI::Broodwar->self()->getUnits());
    const int supply = BWAPI::Broodwar->self()->supplyUsed();
    int supplyCurrent = supply / 2;
    if (supply / numGateways > 20) { Tools::BuildBuildingAtPosition(BWAPI::UnitTypes::Protoss_Gateway, averagePosition); return; }


    // Build key buildings
    // Citadel of adun for zealot upgrade etc
    int numAdun = Tools::CountUnitsOfType(BWAPI::UnitTypes::Protoss_Citadel_of_Adun, BWAPI::Broodwar->self()->getUnits());
    if (numAdun < 1) { Tools::BuildBuildingAtPosition(BWAPI::UnitTypes::Protoss_Citadel_of_Adun, averagePosition); }
    int numTemplar = Tools::CountUnitsOfType(BWAPI::UnitTypes::Protoss_Templar_Archives, BWAPI::Broodwar->self()->getUnits());
    if (numTemplar < 1) { Tools::BuildBuildingAtPosition(BWAPI::UnitTypes::Protoss_Templar_Archives, averagePosition); }


    // Get good upgrades
    // Get three forges and start pumping upgrades for ground units
    makeUpgrade(BWAPI::UpgradeTypes::Enum::Singularity_Charge);



    int numForge = Tools::CountUnitsOfType(BWAPI::UnitTypes::Protoss_Forge, BWAPI::Broodwar->self()->getUnits());
    if (numForge < 3) { Tools::BuildBuildingAtPosition(BWAPI::UnitTypes::Protoss_Forge, averagePosition); }
    makeUpgrade(BWAPI::UpgradeTypes::Enum::Protoss_Ground_Weapons);
    makeUpgrade(BWAPI::UpgradeTypes::Enum::Protoss_Ground_Armor);
    makeUpgrade(BWAPI::UpgradeTypes::Enum::Protoss_Plasma_Shields);

    makeUpgrade(BWAPI::UpgradeTypes::Enum::Leg_Enhancements);


    

    // build more supply
    // std::cout << "Supply thing " << (Tools::GetTotalSupply() / 2) << "\n";
    if (Tools::GetTotalSupply() / 2 < 190){ Tools::BuildBuildingAtPosition(BWAPI::UnitTypes::Protoss_Pylon, averagePosition); }

}

void StarterBot::defendBase()
{
    // Make our units attack enemy units near / in our base

    int numEnemies = BWAPI::Broodwar->enemy()->visibleUnitCount();
    if (numEnemies < 1) { return; }


    BWAPI::Unitset units = BWAPI::Broodwar->self()->getUnits();

    // Attack enemy units in our base
    for (auto& unit : units) 
    {
        if (!unit->getType().isWorker() && !unit->getType().isBuilding() && unit->getPlayer() == BWAPI::Broodwar->self())
        {
            BWAPI::Unitset enemies = BWAPI::Broodwar->enemy()->getUnits();

            for (auto& enemy : enemies)
            {
                if (!enemy->getType().isBuilding() && enemy->getDistance(first_gateway_position) < 1000)
                {
                    if (unit->getLastCommand().getTarget() == enemy) { continue; }
                    unit->attack(enemy);
                }
                // Attack first best enemy
                break;
            }



        }
    }

}



void StarterBot::getPhotonCannon()
{
    // Buildings for Photon cannon
    if (Tools::CountUnitsOfType(BWAPI::UnitTypes::Protoss_Forge, BWAPI::Broodwar->self()->getUnits()) < 1)
    {
        if (Tools::BuildBuilding(BWAPI::UnitTypes::Protoss_Forge)) { return; }
    }

    if (Tools::CountUnitsOfType(BWAPI::UnitTypes::Protoss_Photon_Cannon, BWAPI::Broodwar->self()->getUnits()) < 2)
    {
        if (Tools::BuildBuilding(BWAPI::UnitTypes::Protoss_Photon_Cannon)) { return; }
    }
}


void StarterBot::getTechBuildings()
{
    // Get to observer
    if (Tools::CountUnitsOfType(BWAPI::UnitTypes::Protoss_Cybernetics_Core, BWAPI::Broodwar->self()->getUnits()) < 1)
    {
        Tools::BuildBuilding(BWAPI::UnitTypes::Protoss_Cybernetics_Core);
        return;
    }
    if (Tools::CountUnitsOfType(BWAPI::UnitTypes::Protoss_Robotics_Facility, BWAPI::Broodwar->self()->getUnits()) < 1)
    {
        Tools::BuildBuilding(BWAPI::UnitTypes::Protoss_Robotics_Facility);
        return;
    }
    if (Tools::CountUnitsOfType(BWAPI::UnitTypes::Protoss_Observatory, BWAPI::Broodwar->self()->getUnits()) < 1)
    {
        Tools::BuildBuilding(BWAPI::UnitTypes::Protoss_Observatory);
        return;
    }

}


void StarterBot::zealotMicro()
{
    const int zealotsOwned = Tools::CountUnitsOfType(BWAPI::UnitTypes::Protoss_Zealot, BWAPI::Broodwar->self()->getUnits());
    if (zealotsOwned < 1) { return; }
    int numEnemies = BWAPI::Broodwar->enemy()->visibleUnitCount();
    if (numEnemies < 1) { return; }

    BWAPI::Unitset enemies = BWAPI::Broodwar->enemy()->getUnits();

    for (auto& unit : BWAPI::Broodwar->self()->getUnits())
    {
        
        if (unit->getType() != BWAPI::UnitTypes::Protoss_Zealot) { continue; }

        // If low shield, retreat
        //if (unit->getShields() < 10 && unit->isUnderAttack()) { unit->move(closeChokePoint); continue; }

        int enemyCounter = 0;
        int photonCounter = 0;

        BWAPI::Unit enemyProbe = nullptr;
        bool enemyHasCannon = false;
        

        for (auto& enemy : enemies)
        {
            BWAPI::Position ourPos = unit->getPosition();
            BWAPI::Position enemyPos = enemy->getPosition();
            if (enemy->getType().isWorker()) { enemyProbe = enemy; }
            if (!enemy) { continue; }
            if (enemy->getType().isWorker()) 
            {
                continue; 
            }
            if (enemy->getType() == BWAPI::UnitTypes::Protoss_Photon_Cannon) 
            { 
                enemyHasCannon = true;  
                if (unit->getDistance(enemy) < 300) { photonCounter++; }
            }
            if (enemy->getType().isBuilding())
            {
                continue;
            }
            if (enemy->getDistance(closeChokePoint) < 100) { unit->attack(enemy->getPosition()); return; }

            // if (enemy->getType() == BWAPI::UnitTypes::Protoss_Photon_Cannon) { enemyHasCannon = true; }

            //std::cout << "Enemy in range and adding to counter type is: " << (enemy->getType()) << "\n";
            if (unit->getDistance(enemy) < 200) { enemyCounter++; }
        }

        enemyCounter += photonCounter;

        //std::cout << "Number of enemies in range: " << (enemyCounter) << "\n";

        int friendCounter = 0;
        
        for (auto& friendly : BWAPI::Broodwar->self()->getUnits())
        {
            if (friendly->getType().isWorker() || friendly->getType().isBuilding()) { continue; }

            // If friendly has low shield, he will run back, so dont include in friendCounter
            //if (friendly->getShields() < 10 && friendly->isUnderAttack()) { continue; }
            if (unit->getDistance(friendly) < 200) { friendCounter++; }
        }

        if (enemyCounter + 1 > friendCounter) { unit->move(closeChokePoint); }
        
        // attack probes

     


        //if (enemyCounter == 0) 
        //{
        //    // if (unit->getLastCommand().getTarget()->getType() == BWAPI::UnitTypes::Protoss_Probe) { continue; }
        //    if (unit->getLastCommand().getTarget() == enemyProbe) { continue; }
        //    unit->attack(enemyProbe);
        //}

        
        

    }
}



void StarterBot::dragoonMicro()
{
    const int dragoonsOwned = Tools::CountUnitsOfType(BWAPI::UnitTypes::Protoss_Dragoon, BWAPI::Broodwar->self()->getUnits());
    if (dragoonsOwned < 1) { return; }
    int numEnemies = BWAPI::Broodwar->enemy()->visibleUnitCount();
    if (numEnemies < 1) { return;}

    BWAPI::Unitset enemies = BWAPI::Broodwar->enemy()->getUnits();

    for (auto& unit : BWAPI::Broodwar->self()->getUnits())
    {
        if (unit->getType() != BWAPI::UnitTypes::Protoss_Dragoon) { continue; }
        for (auto& enemy : enemies)
        {
            BWAPI::Position ourPos = unit->getPosition();
            BWAPI::Position enemyPos = enemy->getPosition();



            if (enemy->getType().isWorker() || enemy->getType().isBuilding()) { continue; }
            if (enemy->getDistance(first_gateway_position) < 100) { unit->attack(enemy->getPosition()); return; }
            if (unit->getDistance(enemy) < 2 * 32) { unit->move(first_gateway_position); }
        }
    }
}

void StarterBot::observerMicro()
{
    const int observersOwned = Tools::CountUnitsOfType(BWAPI::UnitTypes::Protoss_Observer, BWAPI::Broodwar->self()->getUnits());
    if (observersOwned < 1) { return; }
    
    BWAPI::Unitset enemies = BWAPI::Broodwar->enemy()->getUnits();

    for (auto& unit : BWAPI::Broodwar->self()->getUnits())
    {
        if (unit->getType() != BWAPI::UnitTypes::Protoss_Observer) { continue; }
        for (auto& enemy : enemies)
        {
            BWAPI::Position ourPos = unit->getPosition();
            BWAPI::Position enemyPos = enemy->getPosition();



            if (enemy->getType().isWorker() || enemy->getType().isBuilding()) { continue; }
            if (enemy->isCloaked()) { unit->move(enemyPos); return; }
        }
    }
}


void StarterBot::fixWorkerSaturation() 
{
    // Make sure we have a maximum of 30 workers at a base, (3 each mineral, 3 on gas)

    // Maybe change the row below, if our bases get destroyed this might need fixing
    if (ourBases.size() == 1) { return; }


    // Check how many minerals left
    BWAPI::Unitset allMinerals = BWAPI::Broodwar->getMinerals();

    // Check how many gas left
    const BWAPI::Unitset myUnits = BWAPI::Broodwar->self()->getUnits();
    const BWAPI::UnitType assimilator = BWAPI::UnitTypes::Protoss_Assimilator;

    

    int maxWorkersAtBase;
    BWAPI::TilePosition otherBase;
    for (auto const& base : ourBases)
    {
        int numMins = 0;
        int numGas = 0;
        for (auto const& possibleOtherBase : ourBases)
        {
            if (base != possibleOtherBase) { otherBase = possibleOtherBase; }
        }

        BWAPI::Position basePos(base);
        BWAPI::Position otherBasePos(otherBase);

        // Minerals
        for (auto const& mineral : allMinerals)
        {
            if (mineral->getDistance(basePos) < 800) { numMins++; }
        }
        // Gas
        BWAPI::Unit thisAssimilator = nullptr;
        for (auto unit : myUnits)
        {
            if (unit->getType() == assimilator && unit->getDistance(basePos) < 800 && unit->getResources() > 0) { numGas++; }
        }

        maxWorkersAtBase = (numGas + numMins) * 3; // 3 workers each thing



        int numWorkers = 0;
        for (auto& unit : BWAPI::Broodwar->self()->getUnits())
        {
            if (unit->getType().isWorker() && unit->getDistance(basePos) < 800)
            {
                numWorkers++;
                if (numWorkers > maxWorkersAtBase)
                {
                    unit->move(otherBasePos);
                }
            }
            
        }

    }


}

void StarterBot::buildAtExpansions()
{
    if (ourBases.size() == 1) { return; }
    for (auto const& existingBase : ourBases)
    {
        // std::cout << "Distance " << (existingBase.getDistance(place)) << "\n";
        
        
        // We want assimilator, pylon and photon cannon

        BWAPI::Position basePos(existingBase);

        const BWAPI::UnitType assimilator = BWAPI::UnitTypes::Protoss_Assimilator;
        const BWAPI::UnitType pylon = BWAPI::UnitTypes::Protoss_Pylon;
        const BWAPI::UnitType photonCannon = BWAPI::UnitTypes::Protoss_Photon_Cannon;

        BWAPI::Unit closestAssim = Tools::GetClosestUnitOfTypeBeingBuilt(assimilator, basePos);
        if (closestAssim && closestAssim->getDistance(basePos) > 400) { Tools::BuildBuildingAtPosition(assimilator, existingBase); }

        BWAPI::Unit closestPylon = Tools::GetClosestUnitOfTypeBeingBuilt(pylon, basePos);
        if (closestPylon && closestPylon->getDistance(basePos) > 400) { Tools::BuildBuildingAtPosition(pylon, existingBase); }

        BWAPI::Unit closestPhotonCannon = Tools::GetClosestUnitOfTypeBeingBuilt(photonCannon, basePos);
        if (closestPhotonCannon && closestPhotonCannon->getDistance(basePos) > 400) { Tools::BuildBuildingAtPosition(photonCannon, existingBase); }
    
    
    }
    


}


BWAPI::TilePosition StarterBot::findExpansions()
{
    BWAPI::TilePosition startPos = BWAPI::Broodwar->self()->getStartLocation();
    float distance = 100000;
    BWAPI::TilePosition closestPos;

    // Goes through ALL possible bases
    for (const Area& area : theMap.Areas()) 
    {
        for (const Base& base : area.Bases())
        {
            BWAPI::TilePosition place = base.Location();
            float distHere = startPos.getDistance(place);

            // Check if we have already expanded here
            bool baseAllowed = true;
            for (auto const& existingBase : ourBases) 
            {
                // std::cout << "Distance " << (existingBase.getDistance(place)) << "\n";
                if (existingBase.getDistance(place) < 20) { baseAllowed = false; }
            }



            if (baseAllowed && distHere < distance)
            {
                closestPos = place;
                distance = distHere;
            }

        }
    }

    // Set checkPoint to next
    BWAPI::Position startPositionChokePoint(closestPos);
    
    float distanceCP = 100000;
    BWAPI::Position closestChokePointNew;

    // Goes through ALL possible chokepoints
    for (const Area& area : theMap.Areas())
    {
        for (const auto& chokePoint : area.ChokePoints())
        {
            BWAPI::WalkPosition place = chokePoint->Center();
            BWAPI::Position posCP(place);


            float distHere = startPositionChokePoint.getDistance(posCP);


            // std::cout << "Position " << (pos) << "\n";

            if (distHere < distanceCP && posCP != closeChokePoint)
            {
                closestChokePointNew = posCP;
                distanceCP = distHere;
            }

        }
    }

    closeChokePoint = closestChokePointNew;


    return closestPos;
}

bool StarterBot::makeUpgrade(BWAPI::UpgradeType makeUpgrade)
{
    const BWAPI::UnitType building = makeUpgrade.whatUpgrades();
    const BWAPI::Unitset& units = BWAPI::Broodwar->self()->getUnits();
    // if we have a valid depot unit and it's currently not training something, train a worker
    // there is no reason for a bot to ever use the unit queueing system, it just wastes resources
    for (auto& currUnit : units)
    {
        if (currUnit->getType() == building && !currUnit->isUpgrading()) { return currUnit->upgrade(makeUpgrade); }
    }
}


void StarterBot::followBuildOrder3GateSpeed()
{
    // 3 Gate speedzeal https://liquipedia.net/starcraft/3_Gate_Speedzeal_(vs._Protoss))) 
    const int supply = BWAPI::Broodwar->self()->supplyUsed();
    int inGameSupply = supply / 2;

    // Always train zealot
    trainUnit(BWAPI::UnitTypes::Protoss_Zealot);
    if (inGameSupply >= 8 && Tools::CountUnitsOfType(BWAPI::UnitTypes::Protoss_Pylon, BWAPI::Broodwar->self()->getUnits()) < 1)
    {
        if (Tools::BuildBuilding(BWAPI::UnitTypes::Protoss_Pylon)) { return; }
    }
    if (inGameSupply >= 10 && Tools::CountUnitsOfType(BWAPI::UnitTypes::Protoss_Gateway, BWAPI::Broodwar->self()->getUnits()) < 1)
    {
        if (Tools::BuildBuilding(BWAPI::UnitTypes::Protoss_Gateway)) { return; }
    }
    if (inGameSupply >= 12 && Tools::CountUnitsOfType(BWAPI::UnitTypes::Protoss_Gateway, BWAPI::Broodwar->self()->getUnits()) < 2)
    {
        Tools::BuildBuildingAtPosition(BWAPI::UnitTypes::Protoss_Gateway, averagePosition);
    }
    if (inGameSupply >= 15 && Tools::CountUnitsOfType(BWAPI::UnitTypes::Protoss_Pylon, BWAPI::Broodwar->self()->getUnits()) < 2)
    {
        Tools::BuildBuilding(BWAPI::UnitTypes::Protoss_Pylon);
    }
    if (inGameSupply >= 21 && Tools::CountUnitsOfType(BWAPI::UnitTypes::Protoss_Pylon, BWAPI::Broodwar->self()->getUnits()) < 3)
    {
        Tools::BuildBuilding(BWAPI::UnitTypes::Protoss_Pylon);
    }
    if (inGameSupply >= 27 && Tools::CountUnitsOfType(BWAPI::UnitTypes::Protoss_Assimilator, BWAPI::Broodwar->self()->getUnits()) < 1)
    {
        Tools::BuildBuildingAtPosition(BWAPI::UnitTypes::Protoss_Assimilator, averagePosition);
    }
    

}


void StarterBot::followBuildOrder3GateRobo()
{
    // 3 Gate Robo https://liquipedia.net/starcraft/3_Gate_Robo_(vs._Protoss)) 
    const int supply = BWAPI::Broodwar->self()->supplyUsed();
    int inGameSupply = supply / 2;
    if (inGameSupply >= 8 && Tools::CountUnitsOfType(BWAPI::UnitTypes::Protoss_Pylon, BWAPI::Broodwar->self()->getUnits()) < 1)
    {
        if (Tools::BuildBuilding(BWAPI::UnitTypes::Protoss_Pylon)) { return; }
    }
    if (inGameSupply >= 10 && Tools::CountUnitsOfType(BWAPI::UnitTypes::Protoss_Gateway, BWAPI::Broodwar->self()->getUnits()) < 1)
    {
        if (Tools::BuildBuilding(BWAPI::UnitTypes::Protoss_Gateway)) { return; }
    }
    if (inGameSupply >= 12 && Tools::CountUnitsOfType(BWAPI::UnitTypes::Protoss_Assimilator, BWAPI::Broodwar->self()->getUnits()) < 1)
    {
        if (Tools::BuildBuildingAtPosition(BWAPI::UnitTypes::Protoss_Assimilator, averagePosition)) { return; }
    }
    if (inGameSupply >= 14 && Tools::CountUnitsOfType(BWAPI::UnitTypes::Protoss_Cybernetics_Core, BWAPI::Broodwar->self()->getUnits()) < 1)
    {
        if (Tools::BuildBuilding(BWAPI::UnitTypes::Protoss_Cybernetics_Core)) { return; }
    }
    if (inGameSupply >= 14 && Tools::CountUnitsOfType(BWAPI::UnitTypes::Protoss_Zealot, BWAPI::Broodwar->self()->getUnits()) < 1)
    {
        if (trainUnit(BWAPI::UnitTypes::Protoss_Zealot)) { return; }
    }
    if (inGameSupply >= 16 && Tools::CountUnitsOfType(BWAPI::UnitTypes::Protoss_Pylon, BWAPI::Broodwar->self()->getUnits()) < 2)
    {
        if (Tools::BuildBuilding(BWAPI::UnitTypes::Protoss_Pylon)) { return; }
    }
    if (inGameSupply >= 18 && Tools::CountUnitsOfType(BWAPI::UnitTypes::Protoss_Dragoon, BWAPI::Broodwar->self()->getUnits()) < 1)
    {
        if (trainUnit(BWAPI::UnitTypes::Protoss_Dragoon)) { return; }
    }
    if (inGameSupply >= 20) // Add something only upgrade once??
    {
        if (makeUpgrade(BWAPI::UpgradeTypes::Enum::Singularity_Charge)) { return; }
    }
    if (inGameSupply >= 21 && Tools::CountUnitsOfType(BWAPI::UnitTypes::Protoss_Pylon, BWAPI::Broodwar->self()->getUnits()) < 3)
    {
        if (Tools::BuildBuilding(BWAPI::UnitTypes::Protoss_Pylon)) { return; }
    }
    if (inGameSupply >= 22 && Tools::CountUnitsOfType(BWAPI::UnitTypes::Protoss_Dragoon, BWAPI::Broodwar->self()->getUnits()) < 2)
    {
        if (trainUnit(BWAPI::UnitTypes::Protoss_Dragoon)) { return; }
    }
    if (inGameSupply >= 26 && Tools::CountUnitsOfType(BWAPI::UnitTypes::Protoss_Robotics_Facility, BWAPI::Broodwar->self()->getUnits()) < 1)
    {
        if (Tools::BuildBuilding(BWAPI::UnitTypes::Protoss_Robotics_Facility)) { return; }
    }
    if (inGameSupply >= 29 && Tools::CountUnitsOfType(BWAPI::UnitTypes::Protoss_Gateway, BWAPI::Broodwar->self()->getUnits()) < 3)
    {
        if (Tools::BuildBuilding(BWAPI::UnitTypes::Protoss_Gateway)) { return; }
    }
    if (inGameSupply >= 30 && Tools::CountUnitsOfType(BWAPI::UnitTypes::Protoss_Dragoon, BWAPI::Broodwar->self()->getUnits()) < 3)
    {
        if (trainUnit(BWAPI::UnitTypes::Protoss_Dragoon)) { return; }
    }
    if (inGameSupply >= 30 && Tools::CountUnitsOfType(BWAPI::UnitTypes::Protoss_Pylon, BWAPI::Broodwar->self()->getUnits()) < 4)
    {
        // BWAPI::TilePosition posBuild(first_gateway_position);
        if (Tools::BuildBuilding(BWAPI::UnitTypes::Protoss_Pylon)) { return; }
        //std::cout << "Trying to build Pylon!: " << (inGameSupply) << "\n";
    }
    if (inGameSupply >= 31 && Tools::CountUnitsOfType(BWAPI::UnitTypes::Protoss_Dragoon, BWAPI::Broodwar->self()->getUnits()) < 4)
    {
        if (trainUnit(BWAPI::UnitTypes::Protoss_Dragoon)) { return; }
    }
    if (inGameSupply >= 33 && Tools::CountUnitsOfType(BWAPI::UnitTypes::Protoss_Observatory, BWAPI::Broodwar->self()->getUnits()) < 1)
    {
        if (Tools::BuildBuilding(BWAPI::UnitTypes::Protoss_Observatory)) { return; }
    }
    if (inGameSupply >= 35 && Tools::CountUnitsOfType(BWAPI::UnitTypes::Protoss_Observer, BWAPI::Broodwar->self()->getUnits()) < 1)
    {
        if (trainUnit(BWAPI::UnitTypes::Protoss_Observer)) { return; }
    }
    if (inGameSupply >= 36)
    {
        buildAdditionalSupply();
        trainUnit(BWAPI::UnitTypes::Protoss_Dragoon);

    }

}


void StarterBot::followBuildOrder2Gate()
{
    // 2 gateway: https://liquipedia.net/starcraft/2_Gate_(vs._Protoss)#10.2F12_Gateway))
    // Returns supply times 2, take divide 2 to get in game supply
    const int supply = BWAPI::Broodwar->self()->supplyUsed();
    //std::cout << "Supply: " << (supply / 2) << "\n";
    int inGameSupply = supply / 2;
    if (inGameSupply >= 8 && Tools::CountUnitsOfType(BWAPI::UnitTypes::Protoss_Pylon, BWAPI::Broodwar->self()->getUnits()) < 1)
    {
        if (Tools::BuildBuilding(BWAPI::UnitTypes::Protoss_Pylon)) { return; }
    }
    if (inGameSupply >= 10 && Tools::CountUnitsOfType(BWAPI::UnitTypes::Protoss_Gateway, BWAPI::Broodwar->self()->getUnits()) < 1)
    {
        if (Tools::BuildBuilding(BWAPI::UnitTypes::Protoss_Gateway)) { return; }
    }
    if (inGameSupply >= 12 && Tools::CountUnitsOfType(BWAPI::UnitTypes::Protoss_Gateway, BWAPI::Broodwar->self()->getUnits()) < 2)
    {
        if (Tools::BuildBuilding(BWAPI::UnitTypes::Protoss_Gateway)) { return; }

    }
    if (inGameSupply >= 13 && Tools::CountUnitsOfType(BWAPI::UnitTypes::Protoss_Zealot, BWAPI::Broodwar->self()->getUnits()) < 1)
    {
        if (trainUnit(BWAPI::UnitTypes::Protoss_Zealot)) { return; }

    }
    if (inGameSupply >= 16 && Tools::CountUnitsOfType(BWAPI::UnitTypes::Protoss_Pylon, BWAPI::Broodwar->self()->getUnits()) < 2)
    {
        if (Tools::BuildBuilding(BWAPI::UnitTypes::Protoss_Pylon)) { return; }
    }
    if (inGameSupply >= 18)
    {
        buildAdditionalSupply();
        trainUnit(BWAPI::UnitTypes::Protoss_Zealot);
    }

}

void StarterBot::followBuildOrderDT() 
{
    // 2 gateway DT: https://liquipedia.net/starcraft/2_Gateway_Dark_Templar_(vs._Protoss))
    // Returns supply times 2, take divide 2 to get in game supply
    const int supply = BWAPI::Broodwar->self()->supplyUsed();
    //std::cout << "Supply: " << (supply / 2) << "\n";
    int inGameSupply = supply / 2;
    
    if (inGameSupply >= 8 && Tools::CountUnitsOfType(BWAPI::UnitTypes::Protoss_Pylon, BWAPI::Broodwar->self()->getUnits()) < 1)
    { 
        Tools::BuildBuilding(BWAPI::UnitTypes::Protoss_Pylon); 
        return; 
    }
    if (inGameSupply >= 10 && Tools::CountUnitsOfType(BWAPI::UnitTypes::Protoss_Gateway, BWAPI::Broodwar->self()->getUnits()) < 1)
    {
        Tools::BuildBuilding(BWAPI::UnitTypes::Protoss_Gateway);
        return;
    }
    if (inGameSupply >= 12 && Tools::CountUnitsOfType(BWAPI::UnitTypes::Protoss_Assimilator, BWAPI::Broodwar->self()->getUnits()) < 1)
    {
        Tools::BuildBuilding(BWAPI::UnitTypes::Protoss_Assimilator);
        return;
    }
    if (inGameSupply >= 13 && Tools::CountUnitsOfType(BWAPI::UnitTypes::Protoss_Zealot, BWAPI::Broodwar->self()->getUnits()) < 1)
    {
        trainUnit(BWAPI::UnitTypes::Protoss_Zealot);
        return;
    }
    if (inGameSupply >= 16 && Tools::CountUnitsOfType(BWAPI::UnitTypes::Protoss_Pylon, BWAPI::Broodwar->self()->getUnits()) < 2)
    {
        Tools::BuildBuilding(BWAPI::UnitTypes::Protoss_Pylon);
        return;
    }
    if (inGameSupply >= 18 && Tools::CountUnitsOfType(BWAPI::UnitTypes::Protoss_Cybernetics_Core, BWAPI::Broodwar->self()->getUnits()) < 1)
    {
        Tools::BuildBuilding(BWAPI::UnitTypes::Protoss_Cybernetics_Core);
        return;
    }
    if (inGameSupply >= 19 && Tools::CountUnitsOfType(BWAPI::UnitTypes::Protoss_Zealot, BWAPI::Broodwar->self()->getUnits()) < 2)
    {
        trainUnit(BWAPI::UnitTypes::Protoss_Zealot);
        return;
    }
    if (inGameSupply >= 22 && Tools::CountUnitsOfType(BWAPI::UnitTypes::Protoss_Pylon, BWAPI::Broodwar->self()->getUnits()) < 3)
    {
        Tools::BuildBuilding(BWAPI::UnitTypes::Protoss_Pylon);
        return;
    }
    if (inGameSupply >= 23 && Tools::CountUnitsOfType(BWAPI::UnitTypes::Protoss_Dragoon, BWAPI::Broodwar->self()->getUnits()) < 1)
    {
        trainUnit(BWAPI::UnitTypes::Protoss_Dragoon);
        return;
    }
    if (inGameSupply >= 26 && Tools::CountUnitsOfType(BWAPI::UnitTypes::Protoss_Citadel_of_Adun, BWAPI::Broodwar->self()->getUnits()) < 1)
    {
        Tools::BuildBuilding(BWAPI::UnitTypes::Protoss_Citadel_of_Adun);
        return;
    }
    if (inGameSupply >= 27 && Tools::CountUnitsOfType(BWAPI::UnitTypes::Protoss_Dragoon, BWAPI::Broodwar->self()->getUnits()) < 2)
    {
        trainUnit(BWAPI::UnitTypes::Protoss_Dragoon);
        return;
    }
    if (inGameSupply >= 29 && Tools::CountUnitsOfType(BWAPI::UnitTypes::Protoss_Gateway, BWAPI::Broodwar->self()->getUnits()) < 2)
    {
        Tools::BuildBuilding(BWAPI::UnitTypes::Protoss_Gateway);
        return;
    }
    if (inGameSupply >= 29 && Tools::CountUnitsOfType(BWAPI::UnitTypes::Protoss_Pylon, BWAPI::Broodwar->self()->getUnits()) < 4)
    {
        Tools::BuildBuilding(BWAPI::UnitTypes::Protoss_Pylon);
        return;
    }
    if (inGameSupply >= 29 && Tools::CountUnitsOfType(BWAPI::UnitTypes::Protoss_Templar_Archives, BWAPI::Broodwar->self()->getUnits()) < 1)
    {
        Tools::BuildBuilding(BWAPI::UnitTypes::Protoss_Templar_Archives);
        return;
    }
    if (inGameSupply >= 30 && Tools::CountUnitsOfType(BWAPI::UnitTypes::Protoss_Zealot, BWAPI::Broodwar->self()->getUnits()) < 4)
    {
        trainUnit(BWAPI::UnitTypes::Protoss_Zealot);
        buildAdditionalSupply();
        return;

    }
    if (inGameSupply >= 31 && Tools::CountUnitsOfType(BWAPI::UnitTypes::Protoss_Dark_Templar, BWAPI::Broodwar->self()->getUnits()) < 4)
    {
        trainUnit(BWAPI::UnitTypes::Protoss_Dark_Templar);
        buildAdditionalSupply();
        return;
    }
    if (inGameSupply >= 32 && Tools::CountUnitsOfType(BWAPI::UnitTypes::Protoss_Forge, BWAPI::Broodwar->self()->getUnits()) < 1)
    {
        Tools::BuildBuilding(BWAPI::UnitTypes::Protoss_Forge);
        return;
    }
    if (inGameSupply >= 33 && Tools::CountUnitsOfType(BWAPI::UnitTypes::Protoss_Photon_Cannon, BWAPI::Broodwar->self()->getUnits()) < 4)
    {
        Tools::BuildBuilding(BWAPI::UnitTypes::Protoss_Photon_Cannon);
        return;
    }
    if (inGameSupply > 35)
    {
        buildAdditionalSupply();
    }





}


bool StarterBot::trainUnit(BWAPI::UnitType unit)
{
    
    // get the unit pointer to zealot builder
    const BWAPI::UnitType building = unit.whatBuilds().first;;
    const BWAPI::Unitset& units = BWAPI::Broodwar->self()->getUnits();
    // if we have a valid depot unit and it's currently not training something, train a worker
    // there is no reason for a bot to ever use the unit queueing system, it just wastes resources
    for (auto& currUnit : units)
    {
        if (currUnit->getType() == building && !currUnit->isTraining()) { return currUnit->train(unit); }
    }

}

void StarterBot::startingScout()
{
    const BWAPI::UnitType pylon = BWAPI::UnitTypes::Protoss_Pylon;

    const int pylonAmount = Tools::CountUnitsOfType(pylon, BWAPI::Broodwar->self()->getUnits());

    const BWAPI::UnitType probe = BWAPI::UnitTypes::Protoss_Probe;

    if (pylonAmount < 1) { return; }

    if (!enemy_base)
    {
        

        if (!m_scout)
        {
            // Scout with first 
            m_scout = Tools::GetUnitOfType(BWAPI::UnitTypes::Protoss_Probe, nullptr);
        }

        auto& startLocations = BWAPI::Broodwar->getStartLocations();

        for (BWAPI::TilePosition tp : startLocations)
        {
            BWAPI::Unitset unitsOnTile = BWAPI::Broodwar->getUnitsOnTile(tp);
            BWAPI::Unitset enemyUnits = BWAPI::Broodwar->enemy()->getUnits();

            for (auto& unit : unitsOnTile)
            {
                for (auto& enemy : enemyUnits)
                {
                    if (enemy == unit)
                    {
                        BWAPI::Position enemy_pos(tp);
                        enemy_base = true;
                        enemy_base_position = enemy_pos;
                    }
                }

            }
            if (BWAPI::Broodwar->isExplored(tp)) { continue; }

            BWAPI::Position pos(tp);

            BWAPI::Broodwar->drawCircleMap(pos, 32, BWAPI::Colors::Red, true);

            m_scout->move(pos);
            break;
        }
        return;
    }



    BWAPI::Position moveTo(BWAPI::Broodwar->self()->getStartLocation());

    if (m_scout->getDistance(moveTo) > 1500){ m_scout->move(moveTo); }

    
}


// Send our idle workers to mine minerals so they don't just stand there
void StarterBot::sendIdleWorkersToMinerals()
{
    // Let's send all of our starting workers to the closest mineral to them
    // First we need to loop over all of the units that we (BWAPI::Broodwar->self()) own
    const BWAPI::Unitset& myUnits = BWAPI::Broodwar->self()->getUnits();
    for (auto& unit : myUnits)
    {
        // Check the unit type, if it is an idle worker, then we want to send it somewhere
        if (unit->getType().isWorker() && unit->isIdle())
        {
            // Get the closest mineral to this worker unit
            BWAPI::TilePosition desiredPos = BWAPI::Broodwar->self()->getStartLocation();
            BWAPI::Position pos(desiredPos);

            BWAPI::Unit closestMineral = Tools::GetClosestUnitTo(unit, BWAPI::Broodwar->getMinerals());

            // If a valid mineral was found, right click it with the unit in order to start harvesting
            if (closestMineral) { unit->rightClick(closestMineral); }
        }
    }
}

void StarterBot::sendIdleWorkersToGas()
{
    // Send workers to gas
    // First we need to loop over all of the units that we (BWAPI::Broodwar->self()) own

    // Check this for closest assimilator

    const BWAPI::Unitset myUnits = BWAPI::Broodwar->self()->getUnits();
    const BWAPI::UnitType assimilator = BWAPI::UnitTypes::Protoss_Assimilator;

    BWAPI::Unit closestAssim = nullptr;

    // Used to be 3 below
    int maxWorkers = maxWorkersGas;
    

    // Make sure no more than 3 workers on a assimilator
    for (auto& thisUnit : myUnits)
    {
        if (thisUnit->getType().isWorker() && thisUnit->isIdle()) // Loop through idle workers
        {
            closestAssim = Tools::GetClosestUnitOfType(assimilator, thisUnit->getPosition());
            if (!closestAssim) { return; } // If we haven't built an assimilator
            if (closestAssim->getResources() == 0) { return; } // If assimilator is empty
            // Loop through units again, check if more than 3
            int currWorkers = 0;
            for (auto& otherUnit : myUnits)
            {
                if (otherUnit->getType().isWorker())
                {
                    if (otherUnit->getLastCommand().getTarget() == closestAssim) { currWorkers += 1; }
                }
            }
            if (currWorkers >= maxWorkers) { return; } // If already 3 workers
            // If a valid gas was found, right click it with the unit in order to start harvesting
            thisUnit->rightClick(closestAssim);

        }
    }
    

   
}

// Train more workers so we can gather more income
void StarterBot::trainAdditionalWorkers()
{
    const BWAPI::UnitType workerType = BWAPI::Broodwar->self()->getRace().getWorker();
    const int workersWanted = 75;
    const int workersOwned = Tools::CountUnitsOfType(workerType, BWAPI::Broodwar->self()->getUnits());
    if (workersOwned < workersWanted)
    {


        for (auto& unit : BWAPI::Broodwar->self()->getUnits())
        {
            if (unit->getType() == BWAPI::UnitTypes::Protoss_Nexus && !unit->isTraining()) { unit->train(workerType); }
        }

    }
}

void StarterBot::makeWorkersWork()
{
    // Fix weird bug
    for (auto& unit : BWAPI::Broodwar->self()->getUnits())
    {
        if (unit->getType().isWorker())
        
        {
            
            BWAPI::Position targetPos = unit->getLastCommand().getTargetPosition();
            BWAPI::UnitType targetType = unit->getLastCommand().getUnitType();
            
            if (!targetPos) { continue; }
            if (!targetType.isBuilding()) { continue; }

            BWAPI::Position moveTo(BWAPI::Broodwar->self()->getStartLocation());

            int lastOkCommand = unit->getLastCommandFrame();
            int currFrame = BWAPI::Broodwar->getFrameCount();
            if (targetType != BWAPI::UnitTypes::Protoss_Nexus && currFrame - lastOkCommand > 200){ unit->move(targetPos); }
            if (targetType == BWAPI::UnitTypes::Protoss_Nexus && currFrame - lastOkCommand > 1500) { unit->move(targetPos); }

            // unit->move(targetPos);

            // std::cout << "Test: " << (targetPos) << "\n";

            
        }
    }
}

// Build more supply if we are going to run out soon
void StarterBot::buildAdditionalSupply()
{
    
    

    // std::cout << "Tilepos: " << (tilePos) << "\n";
    // std::cout << "Startpos: " << (BWAPI::Broodwar->self()->getStartLocation()) << "\n";

    // Get the amount of supply supply we currently have unused
    const int unusedSupply = Tools::GetTotalSupply(true) - BWAPI::Broodwar->self()->supplyUsed();

    // If we have a sufficient amount of supply, we don't need to do anything
    if (unusedSupply >= 8) { return; }

    // Otherwise, we are going to build a supply provider
    const BWAPI::UnitType supplyProviderType = BWAPI::Broodwar->self()->getRace().getSupplyProvider();

    const bool startedBuilding = Tools::BuildBuildingAtPosition(supplyProviderType, averagePosition);
  
}



void StarterBot::attackOpponent()
{

    const BWAPI::Unitset& units = BWAPI::Broodwar->self()->getUnits();
    


    for (auto& unit : units)
    {
        if (!unit->getType().isWorker() && !unit->getType().isBuilding())
        {

            
            
            // Loop through enemy buildings
            BWAPI::Position attackThis;
            for (BWAPI::Position enemy : enemyBuildings)
            {
                attackThis = enemy;
                break;
            }
            if (unit->getLastCommand().getTargetPosition() == attackThis) { continue; }
            unit->attack(attackThis);
            
        }
    }

}


void StarterBot::attackOpponentDragoons()
{

    const BWAPI::Unitset& units = BWAPI::Broodwar->self()->getUnits();
    for (auto& unit : units)
    {
        if (unit->getType() == BWAPI::UnitTypes::Protoss_Dragoon)
        {

            // Loop through enemy buildings
            BWAPI::Position attackThis;
            for (BWAPI::Position enemy : enemyBuildings)
            {
                attackThis = enemy;
                break;
            }


            if (unit->getLastCommand().getTargetPosition() == attackThis) { continue; }
            unit->attack(attackThis);
        }
    }

}




// Draw some relevent information to the screen to help us debug the bot
void StarterBot::drawDebugInformation()
{
    // BWAPI::Broodwar->drawTextScreen(BWAPI::Position(10, 10), "Hello, World!\n");

    // BWAPI::Broodwar->drawCircleMap(closeChokePoint, 10, BWAPI::Colors::Red, true);

    Tools::DrawUnitCommands();
    Tools::DrawUnitBoundingBoxes();
}

// Called whenever a unit is destroyed, with a pointer to the unit
void StarterBot::onUnitDestroy(BWAPI::Unit unit)
{

    int numEnemies = BWAPI::Broodwar->enemy()->visibleUnitCount();

    if (numEnemies < 1) {

        const BWAPI::Unitset& units = BWAPI::Broodwar->self()->getUnits();
        for (auto& ourUnit : units)
        {
            if (!ourUnit->getType().isWorker() && !ourUnit->getType().isBuilding())
            {

                gatherUnits(ourUnit);
            }
        }
    }


    // Add that we remove base from here if all mined out ??
    if (unit->getType().isMineralField()) 
    { 
        fixWorkerSaturation(); 
    }

    // Add if one of our bases is destroyed.
    if (unit->getPlayer() == BWAPI::Broodwar->self() && unit->getType() == BWAPI::UnitTypes::Protoss_Nexus)
    {
        ourBases.remove(unit->getTilePosition());
    }


    // BWEM
    try
    {
        if (unit->getType().isMineralField())    theMap.OnMineralDestroyed(unit);
        else if (unit->getType().isSpecialBuilding()) theMap.OnStaticBuildingDestroyed(unit);
    }
    catch (const std::exception& e)
    {
        BWAPI::Broodwar << "EXCEPTION: " << e.what() << std::endl;
    }


    if (unit->getPlayer() == BWAPI::Broodwar->enemy()) { updateEnemyPositions(); }


}

// Called whenever a unit is morphed, with a pointer to the unit
// Zerg units morph when they turn into other units
void StarterBot::onUnitMorph(BWAPI::Unit unit)
{
	
}

// Called whenever a text is sent to the game by a user
void StarterBot::onSendText(std::string text) 
{ 
    if (text == "/map")
    {
        m_mapTools.toggleDraw();
    }
}

// Called whenever a unit is created, with a pointer to the create unit
// Units are created in buildings like barracks before they are visible, 
// so this will trigger when you issue the build command for most units
void StarterBot::onUnitCreate(BWAPI::Unit unit)
{ 
    BWAPI::UnitType gateway = BWAPI::UnitTypes::Protoss_Gateway;
    const int gatewayAmount = Tools::CountUnitsOfType(gateway, BWAPI::Broodwar->self()->getUnits());
    if (unit->getType() == gateway && unit->getPlayer() == BWAPI::Broodwar->self() && gatewayAmount <= 2)
    {
        first_gateway_position = unit->getPosition();

    }

    /*if (unit->getType() == BWAPI::UnitTypes::Protoss_Probe && ourBases.size() > 0) 
    {

        // Check if we want to expand
        const BWAPI::UnitType workerType = BWAPI::Broodwar->self()->getRace().getWorker();
        const int workersOwned = Tools::CountUnitsOfType(workerType, BWAPI::Broodwar->self()->getUnits());
        int basesOwned = ourBases.size();
        if (workersOwned / basesOwned >= 25)
        {
            BWAPI::TilePosition expandTo = findExpansions();

            BWAPI::Position pos(expandTo);

            BWAPI::Unitset thingsAtExpansion = BWAPI::Broodwar->getUnitsInRadius(pos, 300);

            bool nexusBeingBuilt = false;

            for (auto& thing : thingsAtExpansion)
            {
                if (thing->getType() == BWAPI::UnitTypes::Protoss_Nexus) { nexusBeingBuilt = true; }
            }

            if (!nexusBeingBuilt){ Tools::BuildBuildingAtPosition(BWAPI::UnitTypes::Protoss_Nexus, expandTo); }
            
        }
    }*/
    if (unit->getType() == BWAPI::UnitTypes::Protoss_Probe){ buildAtExpansions(); }

}

void StarterBot::gatherUnits(BWAPI::Unit unit)
{
    // std::cout << "Gather units here: " << (first_gateway_position) << "\n";
    unit->move(closeChokePoint);
}

// Called whenever a unit finished construction, with a pointer to the unit
void StarterBot::onUnitComplete(BWAPI::Unit unit)
{
    
    // Attack enemy units in our base
    if (!unit->getType().isWorker() && !unit->getType().isBuilding() && unit->getPlayer() == BWAPI::Broodwar->self())
    {
        BWAPI::Unitset enemies = BWAPI::Broodwar->enemy()->getUnits();

        int numEnemies = 0;

        for (auto& enemy : enemies)
        {
            if (!enemy->getType().isBuilding() && unit->getDistance(enemy) < 1000)
            {

                unit->attack(enemy->getPosition());
                numEnemies++;
            }
        }
        if (numEnemies == 0)
        {
            if (first_gateway_position && !unit->getType().isWorker() && !unit->getType().isBuilding() && unit->getPlayer() == BWAPI::Broodwar->self()) { gatherUnits(unit); }
        }



    }
    
    if (unit->getPlayer() == BWAPI::Broodwar->self() && unit->getType() == BWAPI::UnitTypes::Protoss_Zealot) { totalZealots++; }
    // std::cout << "Total zealots: " << (totalZealots) << "\n";
    
    
    
    if (unit->getPlayer() == BWAPI::Broodwar->self() && unit->getType() == BWAPI::UnitTypes::Protoss_Nexus) 
    { 
        ourBases.push_back(unit->getTilePosition()); 

        int x = 0;
        int y = 0;
        for (auto& base : ourBases)
        {
            x += base.x;
            y += base.y;
        }


        x = x / ourBases.size();
        y = y / ourBases.size();
        BWAPI::TilePosition tilePos(x, y);
        averagePosition = tilePos;


        // Gather units at new position
        const BWAPI::Unitset& units = BWAPI::Broodwar->self()->getUnits();
        for (auto& ourUnit : units)
        {
            if (!ourUnit->getType().isWorker() && !ourUnit->getType().isBuilding())
            {
                // if (ourUnit->isAttacking()) { continue; }
                // if (ourUnit->getLastCommand().getTargetPosition() == unit->getPosition()) { continue; }
                gatherUnits(ourUnit);
            }
        }
    }

    if (unit->getType() == BWAPI::UnitTypes::Protoss_Nexus && ourBases.size() > 0) 
    {

        //if (workersOwned / basesOwned >= 25)
        //{
        fixWorkerSaturation();
        //}
    }

    //if (unit->getType() == BWAPI::UnitTypes::Protoss_Probe) { fixWorkerSaturation(); }



    



    


}


void StarterBot::attackNearbyEnemy(BWAPI::Unit unit)
{
    const BWAPI::Unitset& units = BWAPI::Broodwar->self()->getUnits();
    for (auto& ourUnit : units)
    {
        if (!ourUnit->getType().isWorker() && !ourUnit->getType().isBuilding())
        {
            // if (ourUnit->isAttacking()) { continue; }
            // if (ourUnit->getLastCommand().getTargetPosition() == unit->getPosition()) { continue; }
            ourUnit->attack(unit->getPosition());
        }
    }
}


// Called whenever a unit appears, with a pointer to the destroyed unit
// This is usually triggered when units appear from fog of war and become visible
void StarterBot::onUnitShow(BWAPI::Unit unit)
{ 



    float distance = unit->getDistance(first_gateway_position); // Closest to what???
    if (unit->getPlayer() == BWAPI::Broodwar->enemy() && !unit->getType().isWorker() && !unit->getType().isBuilding() && distance < 1000)
    {
        attackNearbyEnemy(unit);
    }
    

    if (unit->getPlayer() == BWAPI::Broodwar->enemy()) { updateEnemyPositions(); }



}


void StarterBot::updateEnemyPositions()
{

    //std::cout << "How many elements in enemy list: " << (enemyBuildings.size()) << "\n";

    // Loop through enemy buildings


    // always loop over all currently visible enemy units(even though this set is usually empty)
        for (BWAPI::Unit unit : BWAPI::Broodwar->enemy()->getUnits()) 
        {
            //if this unit is in fact a building
            if (unit->getType().isBuilding()) 
            {
                //check if we have it's position in memory and add it if we don't
                

                bool enemyNotInList = true;
                for (auto const& enemyAlreadyInList : enemyBuildings)
                {
                    // std::cout << "Distance " << (existingBase.getDistance(place)) << "\n";
                    if (enemyAlreadyInList == unit->getPosition()) { enemyNotInList = false; }
                }
                
                if (enemyNotInList) { enemyBuildings.push_back(unit->getPosition()); }
            }
        }

    //loop over all the units that we remember
    for (BWAPI::Position unitPos : enemyBuildings) {
        // compute the TilePosition corresponding to our remembered Position p



        BWAPI::TilePosition tileCorrespondingToP(unitPos);

        //if that tile is currently visible to us...
        if (BWAPI::Broodwar->isVisible(tileCorrespondingToP)) 
        {

            //loop over all the visible enemy buildings and find out if at least
            //one of them is still at that remembered position
            bool buildingStillThere = false;
            for (BWAPI::Unit u2 : BWAPI::Broodwar->enemy()->getUnits()) {
                if ((u2->getType().isBuilding()) && (u2->getPosition() == unitPos)) 
                {
                    buildingStillThere = true;
                    break;
                }
            }

            //if there is no more any building, remove that position from our memory
            if (buildingStillThere == false) 
            {
                enemyBuildings.remove(unitPos);
                break;
            }
        }
    }
}


// Called whenever a unit gets hidden, with a pointer to the destroyed unit
// This is usually triggered when units enter the fog of war and are no longer visible
void StarterBot::onUnitHide(BWAPI::Unit unit)
{ 
	
}

// Called whenever a unit switches player control
// This usually happens when a dark archon takes control of a unit
void StarterBot::onUnitRenegade(BWAPI::Unit unit)
{ 
	
}