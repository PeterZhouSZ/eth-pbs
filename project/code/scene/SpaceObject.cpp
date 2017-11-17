#include "SpaceObject.h"

#include <osgDB/ReadFile>
#include <osg/PolygonMode>
#include <osg/ShapeDrawable>

using namespace pbs17;


//! ID's start from 0.
long SpaceObject::RunningId = 0;


/**
* \brief Constructor of SpaceObject.
*
* \param filename
*      Relative location to the object-file. (Relative from the data-directory in the source).
*/
SpaceObject::SpaceObject(std::string filename)
	: SpaceObject(filename, "") {}


/**
* \brief Constructor of SpaceObject.
*
* \param filename
*      Relative location to the object-file. (Relative from the data-directory in the source).
* \param textureName
*      Relative location to the texture-file. (Relative from the data-directory in the source).
*/
SpaceObject::SpaceObject(std::string filename, std::string textureName)
	: _filename(filename), _textureName(textureName) {
	_id = RunningId;
	++RunningId;

	_position = Eigen::Vector3d(0, 0, 0);
	_orientation = Eigen::Vector3d(0, 0, 0);


	osg::ref_ptr<osg::Geode> geode = new osg::Geode;
	_aabbShape = new osg::ShapeDrawable(new osg::Box(osg::Vec3(), 1.0f));
	_aabbShape->setColor(osg::Vec4(1.0, 0, 0, 1.0));
	geode->addDrawable(_aabbShape);
	
	_aabbRendering = new osg::MatrixTransform;
	_aabbRendering->setNodeMask(0x1);
	_aabbRendering->addChild(geode.get());
	osg::StateSet* ss = _aabbRendering->getOrCreateStateSet();
	ss->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	ss->setAttributeAndModes(new osg::PolygonMode(
		osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE));
}


/**
 * \brief Destructor of SpaceObject.
 */
SpaceObject::~SpaceObject() {
	if (_model) {
		_model = nullptr;
	}
}


/**
 * \brief Initialize the space-object for physics.
 *
 * \param mass
 *      Mass: unit = kg
 * \param linearVelocity
 *      Linear velocity: unit = m/s
 * \param angularVelocity
 *      Angular velocity: unit = rad/s
 * \param force
 *      Global force: unit = vector with norm equals to N
 * \param torque
 *      Global torque: unit = vector with norm equals to N*m (newton metre)
 */
void SpaceObject::initPhysics(double mass, Eigen::Vector3d linearVelocity, Eigen::Vector3d angularVelocity, Eigen::Vector3d force, Eigen::Vector3d torque) {
	_mass = mass;
	_linearVelocity = linearVelocity;
	_angularVelocity = angularVelocity;
	_force = force;
	_torque = torque;
}


/**
 * \brief Set the local rotation of the model. (Around it's local axis).
 *
 * \param angle
 *      Angle of the rotation.
 * \param axis
 *      Axis of the rotation.
 */
void SpaceObject::setLocalRotation(double angle, osg::Vec3d axis) const {
	osg::Matrix rotationMatrix = osg::Matrix::rotate(angle, axis);
	_rotation->setMatrix(rotationMatrix * _rotation->getMatrix());
}


void SpaceObject::calculateAABB() {
	CalculateBoundingBox bbox;
	_rotation->accept(bbox);
	_aabbLocal = bbox.getBoundBox();
	auto trans = osg::Matrix::translate(toOsg(_position));
	_aabbRendering->setMatrix(osg::Matrix::scale(_aabbLocal._max - _aabbLocal._min) * osg::Matrix::translate(toOsg(_position)));
}


void SpaceObject::repositionAABB() {
	osg::BoundingBox global;
	auto trans = osg::Matrix::translate(toOsg(_position));
	global.expandBy(_aabbLocal._min * trans);
	global.expandBy(_aabbLocal._max * trans);
	_aabbGlobal = global;
	_aabbRendering->setMatrix(osg::Matrix::scale(_aabbLocal._max - _aabbLocal._min) * osg::Matrix::translate(toOsg(_position)));
}
