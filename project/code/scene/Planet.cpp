#include "Planet.h"

#include <osg/ShapeDrawable>
#include <osg/Texture2D>
#include <osgDB/ReadFile>

#include "../config.h"
#include "../osg/OsgEigenConversions.h"
#include "../osg/ImageManager.h"
#include "../osg/ModelManager.h"

using namespace pbs17;


/**
 * \brief Constructor of Planet.
 *
 * \param size
 *      Size of the planet.
 */
Planet::Planet(double size)
	: SpaceObject(""), _size(size) {
}


/**
 * \brief Constructor of Planet.
 *
 * \param size
 *      Size of the planet.
 * \param textureName
 *      Relative location to the texture-file. (Relative from the data-directory in the source).
 */
Planet::Planet(double size, std::string textureName)
    : SpaceObject("", textureName), _size(size) {
}


/**
 * \brief Destructor of Planet.
 */
Planet::~Planet() {}


/**
 * \brief Initialize the space-object for OSG.
 *
 * \param position
 *      Initial position of the object.
 * \param ratio
 *      Ratio of the simplifier. (Supported values: [0..1])
 * \param scaling
 *      Scaling of the model. (1.0 => not scaled, < 1.0 => smaller, > 1.0 => larger)
 */
void Planet::initOsg(Eigen::Vector3d position, double ratio, double scaling) {
	// Set the position to the space-object
	_position = position;
	_scaling = _size;

	// Load the model
	std::string modelPath = DATA_PATH + "/sphere.obj";
	osg::ref_ptr<osg::Node> modelFile = ModelManager::Instance()->loadModel(modelPath, ratio, _size);
	//osg::ref_ptr<osg::ShapeDrawable> model = new osg::ShapeDrawable;
	//model->setShape(new osg::Sphere(osg::Vec3d(0.0f, 0.0f, 0.0f), _size));

	if (_textureName != "") {
		std::string texturePath = DATA_PATH + "/texture/" + _textureName;
		osg::ref_ptr<osg::Texture2D> myTex = ImageManager::Instance()->loadTexture(texturePath);
		modelFile->getOrCreateStateSet()->setTextureAttributeAndModes(0, myTex.get());
	}

	// First transformation-node to handle locale-rotations easier
	_rotation = new osg::MatrixTransform;
	_rotation->addChild(modelFile);

	// Second transformation-node for global rotations and translations
	_translation = new osg::MatrixTransform;
	_translation->setMatrix(osg::Matrix::translate(toOsg(position)));
	_translation->addChild(_rotation);

	calculateAABB();

	_model = new osg::Group;
	_model->addChild(_aabbRendering);
	_model->addChild(_translation);
}


void Planet::initPhysics(double mass, Eigen::Vector3d linearVelocity, Eigen::Vector3d angularVelocity, Eigen::Vector3d force, Eigen::Vector3d torque) {
	SpaceObject::initPhysics(mass, linearVelocity, angularVelocity, force, torque);

	_momentOfInertia = Eigen::Matrix3d();
	_momentOfInertia.setIdentity();
	_momentOfInertia *= mass;
	_momentOfInertia *= 0.4 * getRadius() * getRadius();
}