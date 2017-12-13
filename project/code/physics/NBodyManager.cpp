#include "NBodyManager.h"

#include <math.h>
#if defined(_OPENMP)
#include <omp.h>
#endif

using namespace pbs17;
using namespace Eigen;
using namespace std;


NBodyManager::NBodyManager() { }


void NBodyManager::simulateStep(double dt, std::vector<SpaceObject *> &spaceObjects) {

    //CONST
	double G = 1.0; // 6.67408 * pow(10.0, -4.0);
    double EPS = 0.000000001;


    int cntSpaceObj = spaceObjects.size();


    // initialize the forces
    vector<Vector3d> forces(cntSpaceObj);
    for (int i = 0; i < cntSpaceObj; ++i) {
        forces[i] = Vector3d(0.0, 0.0, 0.0);
    }
#if defined(_OPENMP)
#pragma omp parallel for
#endif
    for (int i = 0; i < cntSpaceObj; ++i) {
        SpaceObject* curObject = spaceObjects[i];
        // Calculate a rotation around the rotation-center of the object
        Vector3d curCenter = curObject->getPosition();
        double m = curObject->getMass();

        // acceleration vector

        for (int j = 0; j < cntSpaceObj; ++j) {
            if (i == j) continue; // do not compare the object with it self

            // compare the objects based on the center of mass
            SpaceObject* compareObject = spaceObjects[j];
            Vector3d compareCenter = compareObject->getPosition();

            // get the distance
            Vector3d d = compareCenter - curCenter;

            // compute the square distance
            double r = (compareCenter - curCenter).norm();
            r *= r;

            // F is the force between the masses
            double f = (G * m * compareObject->getMass()) / (r + EPS);
            forces[i] += f * d.normalized();
        }
    }
#if defined(_OPENMP)
#pragma omp parallel for
#endif
    // update positions
    for (int i = 0; i < cntSpaceObj; ++i) {
        SpaceObject* spaceObject = spaceObjects[i];
        Vector3d a = forces[i] / spaceObject->getMass();
        Vector3d v = spaceObject->getLinearVelocity() + (dt * a);
        spaceObject->setLinearVelocity(v);

        Vector3d dtv = dt * v;
        Vector3d p = spaceObject->getPosition() + dtv;

		Vector3d dto = dt * spaceObject->getAngularVelocity();
		osg::Quat q;
        double sinQuat = sin(dto.norm() / 2);
        double cosQuat = cos(dto.norm() / 2);
        if(dto.norm() > 0) {
            q.set(sinQuat*dto(0) / dto.norm(), sinQuat*dto(1) / dto.norm(), sinQuat*dto(2) / dto.norm(), cosQuat);
        } else {
            q.set(0.0, 0.0, 0.0, 1.0);
        }
        osg::Quat newQ = q*spaceObject->getOrientation();


		spaceObject->updatePositionOrientation(p, dtv, newQ);
    }
    /*
    for (std::vector<SpaceObject*>::iterator it = _spaceObjects.begin(); it != _spaceObjects.end(); ++it) {
        for
        SpaceObject* spaceObject = *it;

        // Calculate a rotation around the rotation-center of the object
        osg::Vec3d toCenter = toOsg((*it)->getCenter());
        osg::Matrix rotationGlobal = osg::Matrix::rotate(dt, osg::Vec3d(0.0f, 0.0f, 1.0f));
        osg::Matrixd translate1 = osg::Matrixd::translate(-toCenter);
        osg::Matrixd translate2 = osg::Matrixd::translate(toCenter);

        // Apply rotation to the current position and also rotate the object localy (rotation around it's own axis)
        spaceObject->getModel()->setMatrix(translate1 * rotationGlobal * translate2 * spaceObject->getModel()->getMatrix());
        spaceObject->setLocalRotation(-dt * 10, osg::Vec3d(1.0f, 1.0f, 0.0f));
    }
    */
}
