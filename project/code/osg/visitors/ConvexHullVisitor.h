﻿/**
* \brief Functionality for managing loaded models to prevent loading multiple times the same model.
* The code is copied from http://www.vis-sim.com/osg/code/osgcode_bbox1.htm and adapted to our use.
*
* \Author: Alexander Lelidis (14-907-562), Andreas Emch (08-631-384), Uroš Tešić (17-950-346)
* \Date:   2017-11-11
*/

#pragma once

#include <osg/NodeVisitor>
#include <osg/Billboard>
#include <osg/Geometry>

namespace pbs17 {

	/**
	 * \brief Calculate the bounding-box of anode
	 */
	class  ConvexHullVisitor : public osg::NodeVisitor {

	public:

		/**
		 * \brief Constructor. Initialize the visitor to traverse all children.
		 */
		ConvexHullVisitor() : NodeVisitor(TRAVERSE_ALL_CHILDREN) {
			_vertices = new osg::Vec3Array;
			_convexHull = NULL;
			_transformMatrix.makeIdentity();
			_isCalculated = false;
		}


		/**
		 * \brief Destructor.
		 */
		virtual ~ConvexHullVisitor() {}


		/**
		 * \brief Calculate the bounding-box for the type osg::Geode.
		 *
		 * \param geode
		 *      Current geode-child.
		 */
		void apply(osg::Geode &geode) override;


		/**
		 * \brief Calculate the bounding-box for the type osg::MatrixTransform.
		 *
		 * \param node
		 *      Current matrix-transform-child.
		 */
		void apply(osg::MatrixTransform &node) override;


		/**
		 * \brief Calculate the bounding-box for the type osg::Billboard.
		 * important to handle billboard so that its size will not affect the geode size continue traversing the graph
		 *
		 * \param node
		 *      Current billboard-child.
		 */
		void apply(osg::Billboard &node) override;
		

		osg::ref_ptr<osg::Geometry> getConvexHull();


	protected:

		//! All the vertices found of the subtree
		osg::Vec3Array* _vertices;

		//! The current transform matrix.
		osg::Matrix _transformMatrix;

		//! The convex hull.
		osg::Geometry* _convexHull;

		bool _isCalculated;
	
	};
}