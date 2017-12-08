#pragma once

#include <vector>

#include "../scene/SpaceObject.h"
#include "CollisionManager.h"
#include "NBodyManager.h"

namespace pbs17 {

	/**
	 * \brief The manager to handle the simulation (within all needed calculations) to simulate one frame.
	 */
	class SimulationManager {
	public:
		/**
		 * \brief Constructor of the simulation-manager.
		 *
		 * \param spaceObjects
		 *      All space-objects in the scene.
		 */
		SimulationManager(std::vector<SpaceObject*> spaceObjects);


		/**
		 * \brief Simulate the scene.
		 *
		 * \param dt
		 *      Time difference since between the last frames.
		 */
		void simulate(double dt);

		static void setIsPaused(const bool isPaused) {
			IS_PAUSED = isPaused;
		}

		static bool getIsPaused() {
			return IS_PAUSED;
		}

		static void setSimulationDt(const double dt) {
			SIMULATION_DT = std::max(dt, 0.0);
			SIMULATION_DT = std::min(SIMULATION_DT, 1.0);
		}

		static void increaseSimulationDt(const double dt) {
			setSimulationDt(SIMULATION_DT + dt);
		}

		static void decreaseSimulationDt(const double dt) {
			setSimulationDt(SIMULATION_DT - dt);
		}

		static double getSimulationDt() {
			return SIMULATION_DT;
		}

	private:
		//! All space-objects in the scene
		std::vector<SpaceObject*> _spaceObjects;
		CollisionManager* _cManager;
		NBodyManager* _nManager;

		static bool IS_PAUSED;

		static double SIMULATION_DT;
	};
}
