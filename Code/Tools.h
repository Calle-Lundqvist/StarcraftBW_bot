#pragma once

#include <BWAPI.h>

namespace Tools
{
    BWAPI::Unit GetClosestUnitTo(BWAPI::Position p, const BWAPI::Unitset& units);
    BWAPI::Unit GetClosestUnitTo(BWAPI::Unit unit, const BWAPI::Unitset& units);

    int CountUnitsOfType(BWAPI::UnitType type, const BWAPI::Unitset& units);

    BWAPI::Unit GetUnitOfType(BWAPI::UnitType type, BWAPI::Unit badBuilder);
    BWAPI::Unit GetClosestUnitOfType(BWAPI::UnitType type, BWAPI::Position pos);
    BWAPI::Unit GetClosestUnitOfTypeBeingBuilt(BWAPI::UnitType type, BWAPI::Position position);
    BWAPI::Unit GetDepot();

    bool BuildBuilding(BWAPI::UnitType type);
    bool BuildBuildingAtPosition(BWAPI::UnitType type, BWAPI::TilePosition position);
    void DrawUnitBoundingBoxes();
    void DrawUnitCommands();

    void SmartRightClick(BWAPI::Unit unit, BWAPI::Unit target);

    int GetTotalSupply(bool inProgress = false);

    void DrawUnitHealthBars();
    void DrawHealthBar(BWAPI::Unit unit, double ratio, BWAPI::Color color, int yOffset);
}