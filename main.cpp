#include <CGAL/Simple_cartesian.h>
#include <CGAL/draw_polyhedron.h>
#include <CGAL/Surface_mesh_default_triangulation_3.h>
#include <CGAL/Complex_2_in_triangulation_3.h>
#include <CGAL/make_surface_mesh.h>
#include <CGAL/Implicit_surface_3.h>
#include <CGAL/IO/facets_in_complex_2_to_triangle_mesh.h>
#include <CGAL/Polyhedron_3.h>
#include <CGAL/Polyhedron_items_with_id_3.h>
#include <CGAL/Surface_mesh_deformation.h>
#include <fstream>
#include <string>

typedef CGAL::Simple_cartesian<double>                       Kernel;
typedef CGAL::Surface_mesh_default_triangulation_3           Tr;
typedef CGAL::Complex_2_in_triangulation_3<Tr>               C2t3;

typedef Tr::Geom_traits                                      GT;
typedef GT::Sphere_3                                         Sphere;
typedef GT::Point_3                                          Point;
typedef GT::FT                                               FT;


typedef FT(*Function)(Point);
typedef CGAL::Implicit_surface_3<GT, Function>              Surface;

typedef CGAL::Polyhedron_3<Kernel, CGAL::Polyhedron_items_with_id_3> Polyhedron;
typedef boost::graph_traits<Polyhedron>::vertex_descriptor  vertex_descriptor;
typedef boost::graph_traits<Polyhedron>::vertex_iterator    vertex_iterator;
typedef CGAL::Surface_mesh_deformation<Polyhedron>          Surface_mesh_deformation;


FT sphere_function(Point p) {
    const FT x2 = p.x() * p.x(), y2 = p.y() * p.y(), z2 = p.z() * p.z();
    return x2 + y2 + z2 - 1;
}

int main(int argc, char* argv[])
{
    //SPHERE 1
    Tr tr;
    C2t3 c2t3(tr);
    // defining the surface
    Surface surface(sphere_function,
        Sphere(CGAL::ORIGIN, 4.));
    // defining meshing criteria
    CGAL::Surface_mesh_default_criteria_3<Tr> criteria(30.,
        0.1,
        0.1);
    // meshing surface
    CGAL::make_surface_mesh(c2t3, surface, criteria, CGAL::Non_manifold_tag());
    Polyhedron sm;
    CGAL::facets_in_complex_2_to_triangle_mesh(c2t3, sm);
    CGAL::draw(sm);
    std::ofstream out("sphere_R2.off");
    out << sm << std::endl;

    //SPHERE 2
    Tr tr2;
    C2t3 c2t3_2(tr2);
    Surface surface2(sphere_function,
        Sphere(CGAL::ORIGIN, 9.));
    // defining meshing criteria
    CGAL::Surface_mesh_default_criteria_3<Tr> criteria2(30.,
        0.1,
        0.1);
    // meshing surface
    CGAL::make_surface_mesh(c2t3_2, surface2, criteria2, CGAL::Non_manifold_tag());
    Polyhedron sm2;
    CGAL::facets_in_complex_2_to_triangle_mesh(c2t3_2, sm2);
    CGAL::draw(sm2);
    std::ofstream out2("sphere_R3.off");
    out2 << sm2 << std::endl;

    /* Loading Brains
    //Brain 1
    Polyhedron brainmesh1;
    std::ifstream brain1((argc > 1) ? argv[1] : "D:/BEPdata/133019/Segmentation.off");
    brain1 >> brainmesh1;
    CGAL::draw(brainmesh1);

    //Brain 2
    Polyhedron brainmesh2;
    std::ifstream brain2((argc>2) ? argv[2] : "D:/BEPdata/133625/Segmentation.off");
    brain2 >> brainmesh2;
    CGAL::draw(brainmesh2);
    */

    // Init the indices of the halfedges and the vertices.
    set_halfedgeds_items_id(sm);
    set_halfedgeds_items_id(sm2);

    // Create a deformation object
    Surface_mesh_deformation deform_mesh(sm);

    vertex_iterator vb, ve;
    vertex_iterator vb2, ve2;
    boost::tie(vb, ve) = vertices(sm);     // the whole mesh is going to deform
    boost::tie(vb2, ve2) = vertices(sm2);
    deform_mesh.insert_roi_vertices(vb, ve);

    std::ifstream input("sphere_R3.off");

    for (int i = 0; i < 5; ++i) {
        vertex_descriptor control_1 = *std::next(vb, i);
        deform_mesh.insert_control_vertex(control_1);
        bool is_matrix_factorization_OK = deform_mesh.preprocess();
        if (!is_matrix_factorization_OK) {
            std::cerr << "Error in preprocessing, check documentation of preprocess()" << std::endl;
            return 1;
        }
        // Pos(i) op mesh1 
        // Normaal van Pos(i) lopen we naar mesh2 -> kleinste afstand is punt voor constrained_pos
        vertex_descriptor control_end1 = *std::next(vb, i);
        Surface_mesh_deformation::Point constrained_pos(0,0,0);    // Change position of the vertix
        deform_mesh.set_target_position(control_1, constrained_pos);
        deform_mesh.deform();
    }

    // Save the deformed mesh into a file
    std::ofstream output("Brain_1_deform.off");
    output << sm;

    CGAL::draw(sm);
    return EXIT_SUCCESS;
}