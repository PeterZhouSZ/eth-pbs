#include "CollisionManager.h"

#include <iostream>

#include <Eigen/Core>
// ReSharper disable CppUnusedIncludeDirective
#include <Eigen/LU>
#include <Eigen/Dense>
// ReSharper restore CppUnusedIncludeDirective

#include "../graphics/GjkAlgorithm.h"


using namespace pbs17;
using namespace Eigen;

void print(std::string name, Eigen::Vector3d &v) {
	std::cout << name << "= [" << v.x() << " " << v.y() << " " << v.z() << "]; " << std::endl;
}

void print(std::string name, Eigen::Matrix3d &m) {
	std::cout << name << "= [" << m(0, 0) << " " << m(0, 1) << " " << m(0, 2) << ";" << std::endl
		<< m(1, 0) << " " << m(1, 1) << " " << m(1, 2) << ";" << std::endl
		<< m(2, 0) << " " << m(2, 1) << " " << m(2, 2) << "];" << std::endl;
}

CollisionManager::CollisionManager(std::vector<SpaceObject*> spaceObjects) {
	_xList = std::vector<SpaceObject*>(spaceObjects);
	_yList = std::vector<SpaceObject*>(spaceObjects);
	_zList = std::vector<SpaceObject*>(spaceObjects);

	// sort the lists
	insertionSort(_xList, 0);
	insertionSort(_yList, 1);
	insertionSort(_zList, 2);
}


/*
 * \brief Sort the space-objects based on the minimum coordinate on the given axis.
 * For this, the insertion-sort is used, since the relative order should normaly not change a lot.
 * Therefore it is mostly in O(n) to go through the list.
 *
 * \param A
 *      Output-parameter:
 *			Input:	Vector with all possibly unsorted space-objects.
 *			Output: Vector with all space-objects sorted so that: for each i => i.min[dim] <= i+1.min[dim].
 * \param dim
 *	    Current axis for which the sort-criteria is checked.
 */
void CollisionManager::insertionSort(std::vector<SpaceObject*> &A, int dim) const {
	for (unsigned int i = 1; i < A.size(); ++i) {
		SpaceObject* tmp = A[i];
		int j = i - 1;

		while (j >= 0 && A[j]->getAABB()._min[dim] > A[j + 1]->getAABB()._min[dim]) {
			std::swap(A[j], A[j + 1]);
			j--;
		}

		A[j + 1] = tmp;
	}
}


/**
 *
 */
void CollisionManager::handleCollisions(double dt, std::vector<SpaceObject *> &spaceObjects) {
	std::vector<std::pair<SpaceObject *, SpaceObject *>> collision;

	for (unsigned int i = 0; i < spaceObjects.size(); ++i) {
		spaceObjects[i]->resetCollisionState();
	}

	this->broadPhase(collision);
	this->narrowPhase(collision);
	this->respondToCollisions();
}


/**
 * \brief Find possible collisions based on the bounding-boxes of the objects.
 * The algorithm runs in O(n + #collisions), but the objects need to be sorted based on the min-edge.
 *
 * \param A
 *      Vector with all space-objects. Needs to be sorted based on i.min[dim] <= i+1.min[dim].
 * \param dim
 *	    Current axis for which the collisions checked.
 * \param res
 *	    Output-parameter: Vector with possible collisions. The value is a pair with the two objects which possibly colided.
 *	                      By convention, on the first position the object with the smaller id is stored (key.first.Id < key.second.Id)
 */
void CollisionManager::pruneAndSweep(std::vector<SpaceObject*> &A, int dim, std::vector<std::pair<SpaceObject *, SpaceObject *>> &res) {
	for (unsigned int i = 0; i < A.size() - 1; ++i) {
		double aabbMax = A[i]->getAABB()._max[dim];

		for (unsigned int j = i + 1; j < A.size(); ++j) {
			// If the left edge of the neighbor is smaller than the right edge of the current, a possible collision exists.
			if (aabbMax >= A[j]->getAABB()._min[dim]) {
				// always have the object with the smaller id first
				if (A[i]->getId() < A[j]->getId()) {
					res.push_back(std::make_pair(A[i], A[j]));
				} else {
					res.push_back(std::make_pair(A[j], A[i]));
				}
			} else {
				// Because the list is ordered for the min-edge (left), as soon as the neigbour is greater, we can
				// stop the search for possible collisions.
				break;
			}
		}
	}
}

void CollisionManager::broadPhase(std::vector<std::pair<SpaceObject *, SpaceObject *>> &res) {
	// Resort the list
	insertionSort(_xList, 0);
	insertionSort(_yList, 1);
	insertionSort(_zList, 2);

	// Vectors which contains the possible collisions per axis
	std::vector<std::pair<SpaceObject *, SpaceObject *>> resX;
	std::vector<std::pair<SpaceObject *, SpaceObject *>> resY;
	std::vector<std::pair<SpaceObject *, SpaceObject *>> resZ;

	pruneAndSweep(_xList, 0, resX);
	pruneAndSweep(_yList, 1, resY);
	pruneAndSweep(_zList, 2, resZ);

	std::sort(resX.begin(), resX.end());
	std::sort(resY.begin(), resY.end());
	std::sort(resZ.begin(), resZ.end());
	//std::cout << "Collisions: " << resX.size()
	//	<< ", " << resY.size()
	//	<< ", " << resZ.size() << std::endl;

	std::vector<std::pair<SpaceObject *, SpaceObject *>> resXY;

	// get only the instersections
	std::set_intersection(resX.begin(), resX.end(), resY.begin(), resY.end(),
		std::back_inserter(resXY));

	std::set_intersection(resXY.begin(), resXY.end(), resZ.begin(), resZ.end(),
		std::back_inserter(res));

	if (res.size() > 0) {
		//std::cout << "Possible collisions: " << res.size() << std::endl;
	}
}

bool CollisionManager::checkIntersection(Planet *p1, Planet *p2) {
	double d = (p1->getPosition() - p2->getPosition()).norm();
	//std::cout << d << " " << p1->getRadius() << " " << p2->getRadius() << std::endl;
	return d < p1->getRadius() + p2->getRadius();
}

void CollisionManager::response(Planet *p1, Planet *p2) {
	Vector3d pos1 = p1->getPosition();
	Vector3d pos2 = p2->getPosition();

	// First, find the vector which will serve as a basis vector (x-axis),
	// in an arbitrary direction. It has to be normalized to get realistic results.
	Vector3d x = (pos1 - pos2).normalized();

	// Then we calculate the x-direction velocity vector and the perpendicular y-vector.
	Vector3d v1 = p1->getLinearVelocity();
	double x1 = x.dot(v1);
	Vector3d v1x = x * x1;
	Vector3d v1y = v1 - v1x;
	double m1 = p1->getMass();

	// Same procedure for the other sphere.
	Vector3d v2 = p2->getLinearVelocity();
	double x2 = x.dot(v2);
	Vector3d v2x = x * x2;
	Vector3d v2y = v1 - v2x;
	double m2 = p2->getMass();

	p1->setLinearVelocity(v1x * (m1 - m2) / (m1 + m2) + v2x * (2. * m2) / (m1 + m2) + v1y);
	p2->setLinearVelocity(v1x * (2. * m1) / (m1 + m2) + v2x * (m2 - m1) / (m1 + m2) + v2y);
}

void CollisionManager::narrowPhase(std::vector<std::pair<SpaceObject *, SpaceObject *>> &collisions) {
	for (unsigned int i = 0; i < collisions.size(); ++i) {
		SpaceObject* o1 = collisions[i].first;
		SpaceObject* o2 = collisions[i].second;

		Planet* p1 = dynamic_cast<Planet*>(collisions[i].first);
		Planet* p2 = dynamic_cast<Planet*>(collisions[i].second);

		if (p1 != nullptr && p2 != nullptr) {
			// we have a possible collision between 2 spheres.

			if (checkIntersection(p1, p2)) {
				Collision sphereCollision;
				sphereCollision.setFirstObject(p1);
				sphereCollision.setSecondObject(p2);
				sphereCollision.setUnitNormal((p1->getPosition() - p2->getPosition()).normalized());
				sphereCollision.setFirstPOC(p1->getPosition() - p1->getRadius() * sphereCollision.getUnitNormal());
				sphereCollision.setSecondPOC(p2->getPosition() + p2->getRadius() * sphereCollision.getUnitNormal());
				sphereCollision.setIntersectionVector(sphereCollision.getUnitNormal() * (((p1->getPosition() - p2->getPosition()).norm()) - p1->getRadius() - p2->getRadius()));
				_collisionQueue.push(sphereCollision);
				//std::cout << "INTERSECTION DETECTED" << std::endl;

				p1->setCollisionState(2);
				p2->setCollisionState(2);
			} else {
				p1->setCollisionState(1);
				p2->setCollisionState(1);
			}
		} else {
			std::vector<Eigen::Vector3d> convexHullP1 = o1->getConvexHull();
			std::vector<Eigen::Vector3d> convexHullP2 = o2->getConvexHull();
			Collision sphereCollision(o1, o2);

			if (GjkAlgorithm::intersect(convexHullP1, convexHullP2, sphereCollision)) {
				//sphereCollision.setUnitNormal((o1->getPosition() - o2->getPosition()).normalized());
				_collisionQueue.push(sphereCollision);
				std::cout << "INTERSECTION DETECTED" << std::endl;

				o1->setCollisionState(2);
				o2->setCollisionState(2);
			} else {
				o1->setCollisionState(1);
				o2->setCollisionState(1);
			}
		}
	}
}


Matrix3d CollisionManager::getOrthonormalBasis(Vector3d v) {
	Vector3d firstTangent;
	Vector3d secondTangent;

	if (fabs(v[0]) > fabs(v[1])) {
		firstTangent[0] = v[2];
		firstTangent[1] = 0;
		firstTangent[2] = -v[0];

		firstTangent.normalize();

		secondTangent[0] = v[1] * firstTangent[0];
		secondTangent[1] = v[2] * firstTangent[0] - v[0] * firstTangent[2];
		secondTangent[2] = -v[1] * firstTangent[0];
	} else {
		firstTangent[0] = 0;
		firstTangent[1] = -v[2];
		firstTangent[2] = v[1];

		firstTangent.normalize();

		secondTangent[0] = v[1] * firstTangent[2] - v[2] * firstTangent[1];
		secondTangent[1] = -v[0] * firstTangent[2];
		secondTangent[2] = v[0] * firstTangent[1];
	}

	Matrix3d result;
	result << v[0], firstTangent[0], secondTangent[0],
		v[1], firstTangent[1], secondTangent[1],
		v[2], firstTangent[2], secondTangent[2];

	return result;
}


void CollisionManager::respondToCollisions() {
	while (!_collisionQueue.empty()) {
		Collision currentCollision = _collisionQueue.top();
		_collisionQueue.pop();

		SpaceObject* object1 = currentCollision.getFirstObject();
		SpaceObject* object2 = currentCollision.getSecondObject();

		Matrix3d contactBasis = getOrthonormalBasis(currentCollision.getUnitNormal()).inverse();

		Matrix3d foOrientationX, foOrientationY, foOrientationZ;
		Matrix3d soOrientationX, soOrientationY, soOrientationZ;

		osg::Quat orientation1 = object1->getOrientation();
		osg::Quat orientation2 = object2->getOrientation();

		/*foOrientationX << 1., 0., 0.,
			0., cos(orientation1[0]), -sin(orientation1[0]),
			0., sin(orientation1[0]), cos(orientation1[0]);

		foOrientationY << cos(orientation1[1]), 0, sin(orientation1[1]),
			0, 1, 0,
			-sin(orientation1[1]), 0, cos(orientation1[1]);

		foOrientationZ << cos(orientation1[2]), -sin(orientation1[2]), 0,
			sin(orientation1[2]), cos(orientation1[2]), 0,
			0, 0, 1;

		soOrientationX << 1., 0., 0.,
			0., cos(orientation2[0]), -sin(orientation2[0]),
			0., sin(orientation2[0]), cos(orientation2[0]);

		soOrientationY << cos(orientation2[1]), 0, sin(orientation2[1]),
			0, 1, 0,
			-sin(orientation2[1]), 0, cos(orientation2[1]);

		soOrientationZ << cos(orientation2[2]), -sin(orientation2[2]), 0,
			sin(orientation2[2]), cos(orientation2[2]), 0,
			0, 0, 1;*/

		Eigen::Matrix3d foRotationMatrix = fromOsg(osg::Matrix::rotate(orientation1)).block(0,0,3,3);
		Eigen::Matrix3d soRotationMatrix = fromOsg(osg::Matrix::rotate(orientation2)).block(0,0,3,3);

		Eigen::Matrix3d foWorldInertia = foRotationMatrix.inverse().transpose() * object1->getMomentOfInertia() * foRotationMatrix.inverse();
		Eigen::Matrix3d soWorldInertia = soRotationMatrix.inverse().transpose() * object2->getMomentOfInertia() * soRotationMatrix.inverse();

		Eigen::Vector3d velocityFirst = object1->getLinearVelocity() + object1->getAngularVelocity().cross(currentCollision.getFirstPOC() - object1->getPosition());
		Eigen::Vector3d velocitySecond = object2->getLinearVelocity() + object2->getAngularVelocity().cross(currentCollision.getSecondPOC() - object2->getPosition());

		double closingVelocity = (velocityFirst - velocitySecond).dot(currentCollision.getUnitNormal());
		Vector3d contactVelocity = contactBasis * (velocityFirst - velocitySecond);

		double finalVelocity = -COEF_RESTITUTION * closingVelocity;

		double deltaVelocity = finalVelocity - closingVelocity;

		Vector3d velocityToKill;
		velocityToKill << deltaVelocity, -contactVelocity[1], -contactVelocity[2];

		Matrix3d deltaVelWorld;
		Matrix3d linearResponsePerUnitImpulse = (1. / object1->getMass())*Matrix3d::Identity();
		linearResponsePerUnitImpulse += (1. / object2->getMass())*Matrix3d::Identity();

		Vector3d skewOriginMatrix1 = (currentCollision.getFirstPOC() - object1->getPosition());
		Matrix3d skewSymmetricMatrix1;
		skewSymmetricMatrix1 << 0, -skewOriginMatrix1(2), skewOriginMatrix1(1),
			skewOriginMatrix1(2), 0, -skewOriginMatrix1(0),
			-skewOriginMatrix1(1), skewOriginMatrix1(0), 0;

		Vector3d skewOriginMatrix2 = (currentCollision.getSecondPOC() - object2->getPosition());
		Matrix3d skewSymmetricMatrix2;
		skewSymmetricMatrix2 << 0, -skewOriginMatrix2(2), skewOriginMatrix2(1),
			skewOriginMatrix2(2), 0, -skewOriginMatrix2(0),
			-skewOriginMatrix2(1), skewOriginMatrix2(0), 0;

		deltaVelWorld = (-1) * skewSymmetricMatrix1 * foWorldInertia.inverse() * skewSymmetricMatrix1;
		deltaVelWorld += (-1) * skewSymmetricMatrix2 * soWorldInertia.inverse() * skewSymmetricMatrix2;
		
		Matrix3d deltaVelocityMatrix = contactBasis * deltaVelWorld * contactBasis.inverse();
		deltaVelocityMatrix += linearResponsePerUnitImpulse;
		/*Eigen::Matrix3d rotationalResponsePerUnitImpulse = (-1. * (foWorldInertia.inverse() * skewSymmetricMatrix1 * contactBasis) * skewSymmetricMatrix1);
		rotationalResponsePerUnitImpulse += (-1. * (soWorldInertia.inverse() * skewSymmetricMatrix2 * contactBasis) * skewSymmetricMatrix2);
		Eigen::Matrix3d deltaVelocityMatrix = (linearResponsePerUnitImpulse + contactBasis.transpose() * rotationalResponsePerUnitImpulse * contactBasis);
		Eigen::Vector3d impulseResponse = (deltaVelocityMatrix.inverse() * velocityToKill);*/

		Vector3d contactImpulseResponse = deltaVelocityMatrix.inverse() * velocityToKill;
		double planarImpulse = sqrt(contactImpulseResponse.y()*contactImpulseResponse.y() + contactImpulseResponse.z()*contactImpulseResponse.z());
		if (planarImpulse > contactImpulseResponse.x() * COEF_FRICTION) {
			// We need to use dynamic friction.
			contactImpulseResponse.y() /= planarImpulse;
			contactImpulseResponse.z() /= planarImpulse;
			contactImpulseResponse.x() = deltaVelocityMatrix(0, 0) +
				deltaVelocityMatrix(1, 0) * COEF_FRICTION * contactImpulseResponse.y() +
				deltaVelocityMatrix(2, 0) * COEF_FRICTION * contactImpulseResponse.z();
			contactImpulseResponse.x() = deltaVelocity / contactImpulseResponse.x();
			contactImpulseResponse.y() *= COEF_FRICTION * contactImpulseResponse.x();
			contactImpulseResponse.z() *= COEF_FRICTION * contactImpulseResponse.x();
		}

		Vector3d impulseResponse = contactBasis.inverse() * contactImpulseResponse;

		object1->setPosition(object1->getPosition() - currentCollision.getIntersectionVector());
		object2->setPosition(object2->getPosition() + currentCollision.getIntersectionVector());

		object1->setLinearVelocity(object1->getLinearVelocity() + impulseResponse / object1->getMass());
		object2->setLinearVelocity(object2->getLinearVelocity() - impulseResponse / object2->getMass());

		object1->setAngularVelocity(object1->getAngularVelocity() - foRotationMatrix * foWorldInertia.inverse() * (impulseResponse).cross(currentCollision.getFirstPOC() - object1->getPosition()));
		object2->setAngularVelocity(object2->getAngularVelocity() - soRotationMatrix * soWorldInertia.inverse() * (impulseResponse).cross(currentCollision.getSecondPOC() - object2->getPosition()));

		//print("contactBasis", contactBasis);
		//print("orientation1", orientation1);
		//print("orientation2", orientation2);
		//print("foOrientationX", foOrientationX);
		//print("foOrientationY", foOrientationY);
		//print("foOrientationZ", foOrientationZ);
		//print("soOrientationX", soOrientationX);
		//print("soOrientationY", soOrientationY);
		//print("soOrientationZ", soOrientationZ);
		//print("foRotationMatrix", foRotationMatrix);
		//print("soRotationMatrix", soRotationMatrix);
		//print("momentOfInertia1", object1->getMomentOfInertia());
		//print("momentOfInertia2", object2->getMomentOfInertia());
		//print("foWorldInertia", foWorldInertia);
		//print("soWorldInertia", soWorldInertia);
		//print("velocityFirst", velocityFirst);
		//print("velocitySecond", velocitySecond);
		//print("contactVelocity", contactVelocity);
		//print("velocityToKill", velocityToKill);
		//print("linearResponsePerUnitImpulse", linearResponsePerUnitImpulse);
		//print("skewOriginMatrix1", skewOriginMatrix1);
		//print("skewSymmetricMatrix1", skewSymmetricMatrix1);
		//print("skewOriginMatrix2", skewOriginMatrix2);
		//print("skewSymmetricMatrix2", skewSymmetricMatrix2);
		//print("rotationalResponsePerUnitImpulse", rotationalResponsePerUnitImpulse);
		//print("deltaVelocityMatrix", deltaVelocityMatrix);
		//print("impulseResponse", impulseResponse);
		//print("contactImpulseResponse", contactImpulseResponse);
		//print("position1", object1->getPosition());
		//print("position2", object2->getPosition());
		//print("linearVelocity1", object1->getLinearVelocity());
		//print("linearVelocity2", object2->getLinearVelocity());
		//print("angularVelocity1", object1->getAngularVelocity());
		//print("angularVelocity2", object2->getAngularVelocity());
	}
}
