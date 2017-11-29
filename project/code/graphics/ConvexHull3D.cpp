#include "ConvexHull3D.h"

#include <CGAL/convex_hull_3.h>

#include <CGAL/Surface_mesh_simplification/edge_collapse.h>
#include <CGAL/Surface_mesh_simplification/Policies/Edge_collapse/Count_stop_predicate.h>
#include <CGAL/Surface_mesh_simplification/Policies/Edge_collapse/Edge_length_cost.h>
#include <CGAL/Surface_mesh_simplification/Policies/Edge_collapse/Midpoint_placement.h>

using namespace pbs17;
namespace SMS = CGAL::Surface_mesh_simplification;


/**
 * \brief Constructor which initializes the convex-hull based on the given vertices.
 *
 * \param vertices
 *      All vertices for which the bounding-box has to be calculated.
 */
ConvexHull3D::ConvexHull3D(osg::Vec3Array* vertices) {
	init(vertices);
}


/**
 * \brief Initialize the convex-hull based on the given vertices.
 *
 * \param vertices
 *      All vertices for which the bounding-box has to be calculated.
 */
void ConvexHull3D::init(osg::Vec3Array* vertices) {
	std::vector<Point_3> points(vertices->size());

	// Transform the osg-vertices into cgal-points
	for (unsigned int i = 0; i < vertices->size(); ++i) {
		osg::Vec3 vertex_at_i = (*vertices)[i];
		points[i] = Point_3(vertex_at_i[0], vertex_at_i[1], vertex_at_i[2]);
	}

	// Define polyhedron to hold convex hull and compute convex hull of non-collinear points
	CGAL::convex_hull_3(points.begin(), points.end(), _cgalModel);
	
	std::cout << "\nStart...\n"
		<< (_cgalModel.size_of_halfedges() / 2) << " final edges.\n";
	
	SMS::Count_stop_predicate<Polyhedron_3> stop(100);
	int r = SMS::edge_collapse (_cgalModel
		, stop
		, CGAL::parameters::vertex_index_map(get(CGAL::vertex_external_index, _cgalModel))
		.halfedge_index_map(get(CGAL::halfedge_external_index, _cgalModel))
		.get_cost(SMS::Edge_length_cost <Polyhedron_3>())
		.get_placement(SMS::Midpoint_placement<Polyhedron_3>())
	);

	std::cout << "\nFinished...\n" << r << " edges removed.\n"
		<< (_cgalModel.size_of_halfedges() / 2) << " final edges.\n";
	_cgalNefModel = Nef_Polyhedron_3(_cgalModel);

	// Vectors to store the vertices and faces of the convex-hull
	osg::ref_ptr<osg::Vec3Array> convexVertices = new osg::Vec3Array;
	osg::ref_ptr<osg::DrawElementsUInt> convexFaces = new osg::DrawElementsUInt(GL_TRIANGLES);
	_faces.resize(_cgalModel.size_of_facets(), 3);

	// Set the index of each vertex and add it to the convex-hull-vertices (osg)
	unsigned int index = 0;
	for (Vertex_iterator v = _cgalModel.vertices_begin(); v != _cgalModel.vertices_end(); ++v, ++index) {
		v->id() = index;
		Point_3 vertex = v->point();
		convexVertices->push_back(osg::Vec3(CGAL::to_double(vertex[0]), CGAL::to_double(vertex[1]), CGAL::to_double(vertex[2])));
		_vertices.push_back(Eigen::Vector3d(CGAL::to_double(vertex[0]), CGAL::to_double(vertex[1]), CGAL::to_double(vertex[2])));
	}

	// Store each face of the convex-hull
	unsigned int row = 0;
	for (Facet_iterator pFacet = _cgalModel.facets_begin(); pFacet != _cgalModel.facets_end(); ++pFacet) {
		unsigned int col = 0;
		Halfedge_around_facet_circulator pHalfedge = pFacet->facet_begin();

		do {
			_faces(row, col) = pHalfedge->vertex()->id();
			convexFaces->push_back(pHalfedge->vertex()->id());
			++col;
		} while (++pHalfedge != pFacet->facet_begin());

		++row;
	}

	// Create the osg-entity which represents the convex-hull
	_osgModel = new osg::Geometry;
	_osgModel->setVertexArray(convexVertices);
	_osgModel->addPrimitiveSet(convexFaces);
}