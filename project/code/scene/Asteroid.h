#pragma once

#include <osg/Node>
#include <osg/MatrixTransform>

#include "SpaceObject.h"

namespace pbs17 {

	/**
	 * \brief Class which represents any space-object with all relevant information needed for calculations
	 */
	class Asteroid : public SpaceObject {
	public:
		/**
		 * \brief Constructor of SpaceObject.
		 * 
		 * \param filename
		 *      Relative location to the object-file. (Relative from the data-directory in the source).
		 * \param center
		 *      Center of the global-rotation.
		 */
		Asteroid(std::string filename, Eigen::Vector3d center);


		~Asteroid();


		/**
		 * \brief Initialize the space-object for OSG.
		 * 
		 * \param position
		 *      Initial position of the object.
		 * \param scaling
		 *      Scaling of the model. (1.0 => not scaled, < 1.0 => smaller, > 1.0 => larger)
		 */
		void initOsg(Eigen::Vector3d position, double scaling) override;

	private:
		
	};

}
