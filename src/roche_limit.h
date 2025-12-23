#pragma once

namespace RocheLimit {
    // Constants moved from CONSTANTS.h
    inline constexpr double DIST_MULTIPLIER = 1.5;
    inline constexpr double SIZE_DIFFERENCE = 0.3;
    inline constexpr double MINIMUM_BREAKUP_SIZE = 4.0;

    /**
     * @brief Checks if the mass ratio is sufficient for disintegration.
     * 
     * @param victimMass The mass of the body being checked.
     * @param aggressorMass The mass of the other body.
     * @param aggressorIsBlackHole Is the aggressor a black hole?
     * @return true if the mass ratio allows for disintegration.
     */
    inline bool checkMassRatio(double victimMass, double aggressorMass, bool aggressorIsBlackHole) {
        return (aggressorIsBlackHole ? true : victimMass / aggressorMass < SIZE_DIFFERENCE);
    }

    /**
     * @brief Calculates the Roche limit radius distance.
     * 
     * @param radiusSum The sum of the radii of the two bodies.
     * @return The distance at which the Roche limit is considered breached.
     */
    inline double calculateLimitRadius(double radiusSum) {
        return DIST_MULTIPLIER * radiusSum;
    }

    /**
     * @brief Checks if the Roche limit is breached.
     * 
     * @param distance The distance between the two bodies.
     * @param radiusSum The sum of the radii of the two bodies.
     * @param victimMass The mass of the body being checked for disintegration.
     * @param aggressorMass The mass of the other body.
     * @param aggressorIsBlackHole Is the aggressor a black hole?
     * @return true if the Roche limit is breached, false otherwise.
     */
    inline bool isBreached(double distance, double radiusSum, double victimMass, double aggressorMass, bool aggressorIsBlackHole) {
        return distance < calculateLimitRadius(radiusSum) &&
               checkMassRatio(victimMass,aggressorMass, aggressorIsBlackHole);
    }

    /**
     * @brief Checks if a mass is large enough to be subject to Roche limit disintegration.
     * 
     * @param mass The mass to check.
     * @return true if it can disintegrate.
     */
    inline bool hasMinimumBreakupSize(double mass) {
        return mass >= MINIMUM_BREAKUP_SIZE;
    }
}