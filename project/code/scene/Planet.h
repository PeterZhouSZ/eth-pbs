#pragma once

#include "SpaceObject.h"

namespace pbs17 {

	/**
	 * \brief Class which represents any space-object with all relevant information needed for calculations
	 */
	class Planet : public SpaceObject {
	public:
		/**
		 * \brief Constructor of Planet.
		 *
		 * \param size
		 *      Size of the planet.
		 */
		Planet(double size);


		/**
		 * \brief Constructor of Planet.
		 *
		 * \param size
		 *      Size of the planet.
		 * \param textureName
		 *      Relative location to the texture-file. (Relative from the data-directory in the source).
		 */
		Planet(double size, std::string textureName);


		/**
		 * \brief Destructor of Planet.
		 */
		virtual ~Planet();


		/**
		 * \brief Initialize the space-object for OSG.
		 *
		 * \param position
		 *      Initial translation of the object.
		 * \param ratio
		 *      Ratio of the simplifier. (Supported values: [0..1])
		 * \param scaling
		 *      Scaling of the model. (1.0 => not scaled, < 1.0 => smaller, > 1.0 => larger)
		 */
		void initOsg(Eigen::Vector3d position, double ratio, double scaling) override;

		void initPhysics(double mass, Eigen::Vector3d linearVelocity, Eigen::Vector3d angularVelocity, Eigen::Vector3d force, Eigen::Vector3d torque);

		double getRadius() const {
			return _size;
		}

	private:
		//! Size of the planet
		double _size;

	};

}
