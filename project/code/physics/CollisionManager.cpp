#include <iostream>
#include "CollisionManager.h"

using namespace pbs17;
using namespace Eigen;

CollisionManager::CollisionManager(std::vector<SpaceObject*> spaceObjects) {
    for (std::vector<SpaceObject*>::iterator it = spaceObjects.begin(); it != spaceObjects.end(); ++it) {
        SpaceObject* spaceObject = *it;

        xList.push_back(spaceObject);
        yList.push_back(spaceObject);
        zList.push_back(spaceObject);
    }

    // sort the lists
    insertionSort(xList, 0);
    insertionSort(yList, 1);
    insertionSort(zList, 2);
}
/*
 *
 *
 *
 */
void CollisionManager::insertionSort(std::vector<SpaceObject*> &A, int dim) {
    int i = 0;
    while(i < A.size()) {
        int j = 0;
        while(j > 0 && A[j-1]->getAABB()._min[dim] > A[j]->getAABB()._min[dim]) {
            std::swap(A[j]->getAABB()._min[dim], A[j-1]->getAABB()._min[dim]);
            j--;
        }
        i++;
    }
}

void CollisionManager::handleCollisions(double dt, std::vector<SpaceObject *> _spaceObjects) {
    std::vector<std::pair<SpaceObject *, SpaceObject *>> collision;
    this->broadPhase(collision);
    this->narrowPhase(collision);
}


void CollisionManager::pruneAndSweep(std::vector<SpaceObject*> &A, int dim, std::vector<std::pair<SpaceObject *, SpaceObject *>> &res) {
    for (int i = 0; i < A.size() - 1; ++i) {
        double aabbMax = A[i]->getAABB()._max[dim];
        for (int j = i + 1; j < A.size(); ++j) {
            if(aabbMax > A[j]->getAABB()._min[dim]) {
                // allways have the object with the smaller id first
                if(A[i]->getId() < A[j]->getId()) {
                    res.push_back(std::make_pair(A[i], A[j]));
                } else {
                    res.push_back(std::make_pair(A[j], A[i]));
                }
            } else {
                break;
            }
        }
    }
}

void CollisionManager::broadPhase(std::vector<std::pair<SpaceObject *, SpaceObject *>> &res) {
    // sort the list
    insertionSort(xList, 0);
    insertionSort(yList, 1);
    insertionSort(zList, 2);

    std::vector<std::pair<SpaceObject *, SpaceObject *>> resX;
    std::vector<std::pair<SpaceObject *, SpaceObject *>> resY;
    std::vector<std::pair<SpaceObject *, SpaceObject *>> resZ;

    pruneAndSweep(xList, 0, resX);
    pruneAndSweep(yList, 1, resY);
    pruneAndSweep(zList, 2, resZ);
    std::cout << "resX = " << resX.size() << std::endl;
    std::cout << "resY = " << resY.size() << std::endl;
    std::cout << "resZ = " << resZ.size() << std::endl;

    std::vector<std::pair<SpaceObject *, SpaceObject *>> resXY;
    // get only the instersections
    std::set_intersection(resX.begin(), resX.end(), resY.begin(), resY.end(),
                          std::back_inserter(resXY));

    std::set_intersection(resXY.begin(), resXY.end(), resZ.begin(), resZ.end(),
                          std::back_inserter(res));
}

bool CollisionManager::checkIntersection(Planet *p1, Planet *p2) {
    double d = (p1->getPosition() - p2->getPosition()).norm();
    return (d - p1->getRadius() - p2->getRadius()) < 0.0;
}

void CollisionManager::narrowPhase(std::vector<std::pair<SpaceObject *, SpaceObject *>> &collision) {
    for (int i = 0; i < collision.size(); ++i) {
        Planet* p1 = dynamic_cast<Planet*>(collision[i].first);
        Planet* p2 = dynamic_cast<Planet*>(collision[i].second);
        if(p1 != NULL && p2 != NULL) {
            // we have a possible collision between 2 spheres.

            if(checkIntersection(p1, p2)) {
                std::cout << "INTERSECTION DETECTED" << std::endl;
            }
        } else {
            std::cout << "TBD: impl narrow collision check between aribary space objects" << std::endl;
        }
    }
}
