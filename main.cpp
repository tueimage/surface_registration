#pragma region Include Libaries
#include <CGAL/Simple_cartesian.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/draw_polyhedron.h>
#include <CGAL/Surface_mesh_default_triangulation_3.h>
#include <CGAL/Complex_2_in_triangulation_3.h>
#include <CGAL/Surface_mesh/IO/PLY.h>
#include <CGAL/IO/PLY_reader.h>
#include <CGAL/IO/Polyhedron_iostream.h>
#include <CGAL/Polyhedron_3.h>
#include <CGAL/Polyhedron_items_with_id_3.h>
#include <CGAL/Surface_mesh_deformation.h>
#include <CGAL/boost/graph/copy_face_graph.h>
#include <CGAL/Polygon_mesh_processing/locate.h>
#include <CGAL/AABB_tree.h>
#include <CGAL/AABB_traits.h>
#include <CGAL/AABB_face_graph_triangle_primitive.h>    
#include <CGAL/Polygon_mesh_processing/corefinement.h>
#include <CGAL/Polygon_mesh_processing/measure.h>
#include <CGAL/Polygon_mesh_processing/locate.h>
#include <CGAL/Polygon_mesh_processing/triangulate_faces.h>
#include <CGAL/Polygon_mesh_processing/compute_normal.h>
#include <CGAL/Polygon_mesh_processing/repair.h>
#include <CGAL/Polygon_mesh_processing/internal/repair_extra.h>
#include <CGAL/Polygon_mesh_processing/repair_self_intersections.h>
#include <CGAL/centroid.h>
#include <CGAL/Aff_transformation_3.h>
#include <CGAL/aff_transformation_tags.h>
#include <CGAL/mesh_segmentation.h>
#include <CGAL/optimal_bounding_box.h>
#include <fstream>
#include <string>
#include <iostream>
#include <omp.h>
#include <direct.h>
#include <cmath>
#include <stdio.h>
#include <windows.h>
#include <utility>
#include <vector>
#include <filesystem>
#pragma endregion

#pragma region Typedefs
typedef CGAL::Exact_predicates_inexact_constructions_kernel                  Kernel;
typedef CGAL::Surface_mesh_default_triangulation_3                           Tr;
typedef Tr::Geom_traits                                                      GT;
typedef GT::Point_3                                                          Point;
typedef Kernel::Vector_3                                                     Vector;
typedef CGAL::Aff_transformation_3<Kernel>                                   Transformation;

typedef GT::FT                                                               FT;
typedef FT(*Function)(Point);
typedef CGAL::Surface_mesh<Point>                                            Mesh;
typedef CGAL::Polyhedron_3<Kernel, CGAL::Polyhedron_items_with_id_3>         Polyhedron;
typedef boost::graph_traits<Polyhedron>::vertex_descriptor                   vertex_descriptor;
typedef boost::graph_traits<Polyhedron>::vertex_iterator                     vertex_iterator;
typedef boost::graph_traits<Polyhedron>::halfedge_descriptor                 halfedge_descriptor;
typedef boost::graph_traits<Polyhedron>::edge_descriptor                     edge_descriptor;
typedef boost::graph_traits<Polyhedron>::face_descriptor                     face_descriptor;
typedef boost::graph_traits<Polyhedron>::face_iterator                       face_iterator;
typedef boost::graph_traits<Polyhedron>::out_edge_iterator                   out_edge_iterator;
typedef Eigen::Vector3d                                                      Vector3d;
typedef CGAL::Halfedge_around_target_iterator<Polyhedron>                    halfedge_around_target_iterator;
typedef CGAL::Surface_mesh_deformation<Polyhedron>                           Surface_mesh_deformation;
typedef boost::property_map<Polyhedron, CGAL::vertex_point_t>::type          PM;
namespace PMP = CGAL::Polygon_mesh_processing;
typedef PMP::Face_location<Polyhedron, FT>                                   Face_location;
typedef typename Polyhedron::Vertex_const_iterator                           VCI;
typedef typename Polyhedron::Facet_const_iterator                            FCI;
typedef typename Polyhedron::Halfedge_around_facet_const_circulator          HFCC;
typedef CGAL::Inverse_index<VCI>                                             Index;
typedef CGAL::AABB_face_graph_triangle_primitive<Polyhedron>                 Primitive;
typedef CGAL::AABB_traits<Kernel, Primitive>                                 Traits;
typedef CGAL::AABB_tree<Traits>                                              Tree;
#pragma endregion

template <typename OutputIterator>
OutputIterator
adjacent_vertices(const Polyhedron& g,
    vertex_descriptor vd,
    OutputIterator out)
{
    halfedge_around_target_iterator hi, he;
    for (boost::tie(hi, he) = halfedges_around_target(halfedge(vd, g), g); hi != he; ++hi)
    {
        *out++ = source(*hi, g);
    }
    return out;
}

inline std::string getCurrentDateTime(std::string s) {
    time_t now = time(0);
    struct tm  tstruct;
    char  buf[80];
    tstruct = *localtime(&now);
    if (s == "now")
        strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);
    else if (s == "date")
        strftime(buf, sizeof(buf), "%Y-%m-%d", &tstruct);
    return std::string(buf);
};

void del_line(const char* file_name, int n)
{
    std::ifstream fin(file_name);
    std::ofstream fout;
    fout.open("./Logs/temp.txt", std::ios::out);
    char ch;
    int line = 1;
    while (fin.get(ch)){
        if (ch == '\n')
            line++;

        if (line != n)      // content not to be deleted 
            fout << ch;
    }
    fout.close();
    fin.close();
    remove(file_name);
    rename("./Logs/temp.txt", file_name);
}

void PLY(Polyhedron mesh, std::string path) {
    std::filebuf fb;
    fb.open(path, std::ios::out);
    std::ostream os(&fb);
    os << "ply\n"
        << "format ascii 1.0\n"
        << "element vertex " << mesh.size_of_vertices() << "\n"
        << "property float x\n"
        << "property float y\n"
        << "property float z\n"
        << "element face " << mesh.size_of_facets() << "\n"
        << "property list uchar int vertex_index\n"
        << "end_header\n";
    for (auto it = mesh.vertices_begin(); it != mesh.vertices_end(); it++) {
        os << it->point().x() << " " << it->point().y() << " " << it->point().z() << std::endl;
    }
    Index index(mesh.vertices_begin(), mesh.vertices_end());
    for (FCI fi = mesh.facets_begin(); fi != mesh.facets_end(); ++fi) {
        HFCC hc = fi->facet_begin();
        HFCC hc_end = hc;
        os << circulator_size(hc) << " ";
        do {
            os << index[VCI(hc->vertex())] << " ";
            ++hc;
        } while (hc != hc_end);
        os << "\n";
    }
    fb.close();
}

inline void Logger(std::string logMsg, bool Skipline=false, bool Progress=false, int n=0) {
    std::string filePath = "./Logs/log.txt";
    std::string now = getCurrentDateTime("now");
    if (Skipline) {
        std::ofstream ofs(filePath.c_str(), std::ios_base::out | std::ios_base::app);
        ofs << "\n";
        ofs.close();
    }
    else if (Progress) {
        del_line(filePath.c_str(), n);
        std::ofstream ofs(filePath.c_str(), std::ios_base::out | std::ios_base::app);
        ofs << now << "\t" << logMsg << "\n";
        ofs.close();
    }
    else {
        std::ofstream ofs(filePath.c_str(), std::ios_base::out | std::ios_base::app);
        ofs << now << '\t' << logMsg << '\n';
        ofs.close();
    }
}

void Stealth()
{
    HWND Stealth;
    AllocConsole();
    Stealth = FindWindowA("ConsoleWindowClass", NULL);
    ShowWindow(Stealth, 0);
}

CGAL::Polyhedron_3<Kernel, CGAL::Polyhedron_items_with_id_3> Rotate(Polyhedron mesh, Vector normal, Vector axis) {
    double inproduct = normal[0] * axis[0] + normal[1] * axis[1] + normal[2] * axis[2];
    Vector crossproduct(normal[1] * axis[2] - normal[2] * axis[1], normal[2] * axis[0] - normal[0] * axis[2], normal[0] * axis[1] - normal[1] * axis[0]);

    double normal_len = pow(pow(normal[0], 2) + pow(normal[1], 2) + pow(normal[2], 2), 0.5);
    double axis_len = pow(pow(axis[0], 2) + pow(axis[1], 2) + pow(axis[2], 2), 0.5);
    double cosA = inproduct / (normal_len * axis_len);

    const float k = 1.0f / (1.0f + cosA);
    CGAL::Aff_transformation_3<Kernel> rotation((crossproduct.x() * crossproduct.x() * k) + cosA,
        (crossproduct.y() * crossproduct.x() * k) - crossproduct.z(),
        (crossproduct.z() * crossproduct.x() * k) + crossproduct.y(),
        (crossproduct.x() * crossproduct.y() * k) + crossproduct.z(),
        (crossproduct.y() * crossproduct.y() * k) + cosA,
        (crossproduct.z() * crossproduct.y() * k) - crossproduct.x(),
        (crossproduct.x() * crossproduct.z() * k) - crossproduct.y(),
        (crossproduct.y() * crossproduct.z() * k) + crossproduct.x(),
        (crossproduct.z() * crossproduct.z() * k) + cosA);

    std::transform(mesh.points_begin(), mesh.points_end(), mesh.points_begin(), rotation);
    return mesh;
}

void computefaces(Polyhedron p_box, face_descriptor& smallestface, face_descriptor& largestface) {
    face_iterator fcb, fce;
    boost::tie(fcb, fce) = faces(p_box);
    double largestarea = 0;
    double smallestarea = 1000000;
    for (int i = 0; i < p_box.size_of_facets(); i++) {
        face_descriptor face = *std::next(fcb, i);
        double newarea = CGAL::Polygon_mesh_processing::face_area(face, p_box);
        if (newarea > largestarea) {
            largestarea = newarea;
            largestface = face;
        }
        if (newarea < smallestarea) {
            smallestarea = newarea;
            smallestface = face;
        }
    }
}


int main(int argc, char* argv[])
{
    Stealth();
    int line = 0;
    _chdir("C:/Users/20181933/Documents/Jaar_3/Kwartiel_4/BEP_Medical_Imaging/MainProgram/build");
    std::string delpath = "\C:\\Users\\20181933\\Documents\\Jaar_3\\Kwartiel_4\\BEP_Medical_Imaging\\MainProgram\\build\\Logs";
    std::string a = "del /s /q " + delpath + "\\* && pushd " + delpath + " && (rd /s /q " + delpath + " 2>null)";
    std::string delpath2 = "\C:\\Users\\20181933\\Documents\\Jaar_3\\Kwartiel_4\\BEP_Medical_Imaging\\MainProgram\\build\\Output";
    std::string a2 = "del /s /q " + delpath2 + "\\* && pushd " + delpath2 + " && (rd /s /q " + delpath2 + " 2>null)";
    system(a.c_str());
    system(a2.c_str());
    std::string logpath = "./Logs";
    _mkdir(logpath.c_str());
    std::string pathname = "./Output";
    _mkdir(pathname.c_str());
    std::string pathname2 = "./Output/Result";
    _mkdir(pathname2.c_str());
    std::string pathname3 = "./Output/Plot";
    _mkdir(pathname3.c_str());
    std::string bbpath = "./Output/BoundingBoxes";
    _mkdir(bbpath.c_str());
    std::string pathname4 = "./Output/Transformation";
    _mkdir(pathname4.c_str());

    std::setprecision(17);
    line = line + 1;
    Logger("Starting Algorithm!");
    std::ifstream inFile;
    std::string file1, file2, sendEpoch, slearningrate, snumber_of_rings;
    int endEpoch, number_of_rings;
    double learningrate;
    inFile.open("Parameters.txt");
    getline(inFile, file1);
    getline(inFile, file2);
    getline(inFile, sendEpoch);
    endEpoch = stoi(sendEpoch);
    getline(inFile, snumber_of_rings);
    number_of_rings = stoi(snumber_of_rings);
    getline(inFile, slearningrate);
    learningrate = stoi(slearningrate);

    inFile.close();

    //Brain 1
    Mesh sm1;
    std::ifstream in1((argc > 1) ? argv[1] : file1);
    CGAL::read_ply(in1, sm1);
    Polyhedron brainmeshr1;
    CGAL::copy_face_graph(sm1, brainmeshr1);
    PLY(brainmeshr1, "./Output/BoundingBoxes/Brain1_1.ply");

    //Brain 2
    Mesh sm2;
    std::ifstream in2((argc > 2) ? argv[2] : file2);
    CGAL::read_ply(in2, sm2);
    Polyhedron brainmeshr2;
    CGAL::copy_face_graph(sm2, brainmeshr2);
    PLY(brainmeshr2, "./Output/BoundingBoxes/Brain2_1.ply");

    line = line + 2;
    Logger("skip", true);
    Logger("Rigid registration starting!");

    clock_t Rbegin = clock();
    //Make Bounding Boxes
    // Compute the extreme points of the mesh, and then a tightly fitted oriented bounding box
    std::array<Point, 8> obb_points;
    CGAL::oriented_bounding_box(sm1, obb_points,
        CGAL::parameters::use_convex_hull(true));
    Mesh obb_sm;
    CGAL::make_hexahedron(obb_points[0], obb_points[1], obb_points[2], obb_points[3],
        obb_points[4], obb_points[5], obb_points[6], obb_points[7], obb_sm);
    PMP::triangulate_faces(obb_sm);
    Polyhedron p_box;
    CGAL::copy_face_graph(obb_sm, p_box);
    PLY(p_box, "./Output/BoundingBoxes/Box1_1.ply");

    std::array<Point, 8> obb_points2;
    CGAL::oriented_bounding_box(sm2, obb_points2,
        CGAL::parameters::use_convex_hull(true));
    Mesh obb_sm2;
    CGAL::make_hexahedron(obb_points2[0], obb_points2[1], obb_points2[2], obb_points2[3],
        obb_points2[4], obb_points2[5], obb_points2[6], obb_points2[7], obb_sm2);
    PMP::triangulate_faces(obb_sm2);
    Polyhedron p_box2;
    CGAL::copy_face_graph(obb_sm2, p_box2);
    PLY(p_box2, "./Output/BoundingBoxes/Box2_1.ply");

    // CENTROID OF CUBE 1
    std::vector<Point> points_box1;
    points_box1.insert(points_box1.end(), obb_points.begin(), obb_points.end());
    Point centroidbox1 = CGAL::centroid(points_box1.begin(), points_box1.end(), CGAL::Dimension_tag<0>());

    // CENTROID OF CUBE 2
    std::vector<Point> points_box2;
    points_box2.insert(points_box2.end(), obb_points2.begin(), obb_points2.end());
    Point centroidbox2 = CGAL::centroid(points_box2.begin(), points_box2.end(), CGAL::Dimension_tag<0>());

    // Init the indices of the halfedges and the vertices BOX 1
    set_halfedgeds_items_id(p_box);
    Surface_mesh_deformation deform_meshp1(p_box);
    vertex_iterator vbp1, vep1;
    boost::tie(vbp1, vep1) = vertices(p_box);

    // Init the indices of the halfedges and the vertices BOX 2
    set_halfedgeds_items_id(p_box2);
    Surface_mesh_deformation deform_meshp2(p_box2);
    vertex_iterator vbp2, vep2;
    boost::tie(vbp2, vep2) = vertices(p_box2);

    set_halfedgeds_items_id(brainmeshr1);
    Surface_mesh_deformation deform_meshr1(brainmeshr1);
    vertex_iterator vbr1, ver1;
    boost::tie(vbr1, ver1) = vertices(brainmeshr1);

    set_halfedgeds_items_id(brainmeshr2);
    Surface_mesh_deformation deform_meshr2(brainmeshr2);
    vertex_iterator vbr2, ver2;
    boost::tie(vbr2, ver2) = vertices(brainmeshr2);

    // COMPUTE DIFFERENCE BETWEEN CENTROIDS AND ORIGIN OF THE BOXES
    Vector translation = Vector(centroidbox1, Point(0, 0, 0));
    Vector translation2 = Vector(centroidbox2, Point(0, 0, 0));

    // TRANSLATION BOX 1
    deform_meshp1.insert_control_vertices(vbp1, vep1);
    deform_meshp1.translate(vbp1, vep1, translation);
    deform_meshp1.deform();
    deform_meshp1.overwrite_initial_geometry();
    deform_meshp1.clear_roi_vertices();
    deform_meshp1.clear_control_vertices();
    
    // TRANSLATION BOX 2
    deform_meshp2.insert_control_vertices(vbp2, vep2);
    deform_meshp2.translate(vbp2, vep2, translation2);
    deform_meshp2.deform();
    deform_meshp2.overwrite_initial_geometry();
    deform_meshp2.clear_roi_vertices();
    deform_meshp2.clear_control_vertices();

    deform_meshr1.insert_control_vertices(vbr1, ver1);
    deform_meshr1.translate(vbr1, ver1, translation);
    deform_meshr1.deform();
    deform_meshr1.overwrite_initial_geometry();
    deform_meshr1.clear_roi_vertices();
    deform_meshr1.clear_control_vertices();

    deform_meshr2.insert_control_vertices(vbr2, ver2);
    deform_meshr2.translate(vbr2, ver2, translation2);
    deform_meshr2.deform();
    deform_meshr2.overwrite_initial_geometry();
    deform_meshr2.clear_roi_vertices();
    deform_meshr2.clear_control_vertices();

    // SAVE THE PLY FILE FOR BOX 1
    PLY(p_box, "./Output/BoundingBoxes/Box1_2.ply");
    PLY(brainmeshr1, "./Output/BoundingBoxes/Brain1_2.ply");

    //SAVE THE PLY FILE FOR BOX 2
    PLY(p_box2, "./Output/BoundingBoxes/Box2_2.ply");
    PLY(brainmeshr2, "./Output/BoundingBoxes/Brain2_2.ply");


    // ROTATION
    Vector xaxis(1, 0, 0); // Define x and y axis
    Vector yaxis(0, 1, 0);

    // BOX 1
    face_descriptor largestface;
    face_descriptor smallestface;
    computefaces(p_box, smallestface, largestface); // Compute largest and smallest face of box
    Vector normal = CGAL::Polygon_mesh_processing::compute_face_normal(largestface, p_box); // Compute normal of largest face
    Polyhedron p_boxA = Rotate(p_box, normal, xaxis);  // Rotate box1 to x-axis, using the normal of the largest face
    Polyhedron brainmeshr1A = Rotate(brainmeshr1, normal, xaxis);

    face_descriptor largestface3;
    face_descriptor smallestface3;
    computefaces(p_boxA, smallestface3, largestface3); // Update largest and smallest face after rotation to x-axis
    Vector normalsmall = CGAL::Polygon_mesh_processing::compute_face_normal(smallestface3, p_boxA); // Compute normal of smallest face
    Polyhedron p_boxB = Rotate(p_boxA, normalsmall, yaxis); // Rotate box again, now around x-axis, towards the y-axis
    Polyhedron brainmeshr1B = Rotate(brainmeshr1A, normalsmall, yaxis);
    PLY(p_boxB, "./Output/BoundingBoxes/Box1_3.ply");
    PLY(brainmeshr1B, "./Output/BoundingBoxes/Brain1_3.ply");


    // BOX 2
    face_descriptor largestface2;
    face_descriptor smallestface2;
    computefaces(p_box2, smallestface2, largestface2);
    Vector normal2 = CGAL::Polygon_mesh_processing::compute_face_normal(largestface2, p_box2);
    Polyhedron p_box2A = Rotate(p_box2, normal2, xaxis); // Rotation towards x-axis    
    Polyhedron brainmeshr2A = Rotate(brainmeshr2, normal2, xaxis);

    face_descriptor largestface4;
    face_descriptor smallestface4;
    computefaces(p_box2A, smallestface4, largestface4);
    Vector normalsmall2 = CGAL::Polygon_mesh_processing::compute_face_normal(smallestface4, p_box2A);
    Polyhedron p_box2B = Rotate(p_box2A, normalsmall2, yaxis); // Rotations around x-axis so that there is alignment with y-axis
    Polyhedron brainmeshr2B = Rotate(brainmeshr2A, normalsmall2, yaxis);
    PLY(p_box2B, "./Output/BoundingBoxes/Box2_3.ply");
    PLY(brainmeshr2B, "./Output/BoundingBoxes/Brain2_3.ply");

    set_halfedgeds_items_id(p_boxB);
    Surface_mesh_deformation deform_meshp1B(p_boxB);
    vertex_iterator vbp1B, vep1B;
    boost::tie(vbp1B, vep1B) = vertices(p_boxB);
    deform_meshp1B.insert_control_vertices(vbp1B, vep1B);
    deform_meshp1B.translate(vbp1B, vep1B, -translation);
    deform_meshp1B.deform();
    deform_meshp1B.overwrite_initial_geometry();
    deform_meshp1B.clear_roi_vertices();
    deform_meshp1B.clear_control_vertices();
    PLY(p_boxB, "./Output/BoundingBoxes/Box1_4.ply");

    set_halfedgeds_items_id(p_box2B);
    Surface_mesh_deformation deform_meshp2B(p_box2B);
    vertex_iterator vbp2B, vep2B;
    boost::tie(vbp2B, vep2B) = vertices(p_box2B);
    deform_meshp2B.insert_control_vertices(vbp2B, vep2B);
    deform_meshp2B.translate(vbp2B, vep2B, -translation);
    deform_meshp2B.deform();
    deform_meshp2B.overwrite_initial_geometry();
    deform_meshp2B.clear_roi_vertices();
    deform_meshp2B.clear_control_vertices();
    PLY(p_box2B, "./Output/BoundingBoxes/Box2_4.ply");

    set_halfedgeds_items_id(brainmeshr1B);
    Surface_mesh_deformation deform_meshr1B(brainmeshr1B);
    vertex_iterator vbr1B, ver1B;
    boost::tie(vbr1B, ver1B) = vertices(brainmeshr1B);
    deform_meshr1B.insert_control_vertices(vbr1B, ver1B);
    deform_meshr1B.translate(vbr1B, ver1B, -translation);
    deform_meshr1B.deform();
    deform_meshr1B.overwrite_initial_geometry();
    deform_meshr1B.clear_roi_vertices();
    deform_meshr1B.clear_control_vertices();
    PLY(brainmeshr1B, "./Output/BoundingBoxes/Brain1_4.ply");

    set_halfedgeds_items_id(brainmeshr2B);
    Surface_mesh_deformation deform_meshr2B(brainmeshr2B);
    vertex_iterator vbr2B, ver2B;
    boost::tie(vbr2B, ver2B) = vertices(brainmeshr2B);
    deform_meshr2B.insert_control_vertices(vbr2B, ver2B);
    deform_meshr2B.translate(vbr2B, ver2B, -translation2);
    deform_meshr2B.deform();
    deform_meshr2B.overwrite_initial_geometry();
    deform_meshr2B.clear_roi_vertices();
    deform_meshr2B.clear_control_vertices();
    PLY(brainmeshr2B, "./Output/BoundingBoxes/Brain2_4.ply");

    clock_t Rend = clock();
    double Relapsed_time = double(Rend - Rbegin) / CLOCKS_PER_SEC;

    line = line + 2;
    Logger("Orientation of brains aligned!");
    Logger("Registration time: " + std::to_string(Relapsed_time) + " seconds.");

    Polyhedron brainmesh1;
    CGAL::copy_face_graph(brainmeshr1B, brainmesh1);
    set_halfedgeds_items_id(brainmesh1);
    Surface_mesh_deformation deform_mesh(brainmesh1);
    vertex_iterator vb1, ve1;
    boost::tie(vb1, ve1) = vertices(brainmesh1);

    Polyhedron brainmesh2;
    CGAL::copy_face_graph(brainmeshr2B, brainmesh2);

    Polyhedron original;
    CGAL::copy_face_graph(brainmesh1, original);
    PM originalcoord = get(CGAL::vertex_point, original);

    //Compute normals for each vertex
    Tree treebrain2(faces(brainmesh2).first, faces(brainmesh2).second, brainmesh2);
    std::map<face_descriptor, Vector> fnormals;
    std::map<vertex_descriptor, Vector> vnormals;
    CGAL::Polygon_mesh_processing::compute_normals(brainmesh2,
        boost::make_assoc_property_map(vnormals),
        boost::make_assoc_property_map(fnormals));

    std::vector<Surface_mesh_deformation::Point> originalplist;
    std::vector<vertex_descriptor> originalvertexlist;
    for (int i = 0; i < brainmesh1.size_of_vertices(); i++) {
        originalvertexlist.push_back(*std::next(vb1, i));
        Surface_mesh_deformation::Point originalp = get(originalcoord, *std::next(vb1, i));
        originalplist.push_back(originalp);
    }

    std::vector<double> Errorlist;
    std::vector<double> Dicelist;
    std::vector<double> Epochlist;
    std::vector<std::tuple<double, double, double>> TotalTranslist;

    line = line + 5;

    Logger("skip", true);
    Logger("Deformation settings");
    Logger("Epochs: " + std::to_string(endEpoch));
    Logger("Number of rings in controlpoints: " + std::to_string(number_of_rings));
    Logger("Learning rate: " + std::to_string(learningrate));
    double alfa = 1; //Literatuur waardes
    double beta = 0.1; //Literatuur waardes
    double gamma = 0.1; //Literatuur waardes
    double stopcriterion = 0.01; //Literatuur waardes (0) wat is klein genoeg

    int Epoch = 0;
    double total_time = 0;
    line = line + 1;
    Logger("skip", true);
    while(Epoch < endEpoch) {
        std::string pathname = "./Output/" + std::to_string(Epoch);
        _mkdir(pathname.c_str());

        clock_t begin = clock();

        Epochlist.push_back(Epoch);
        line = line + 1;
        Logger("Epoch: " + std::to_string(Epoch));

        double Error = 0;

        //Loop over all vertices in brain1
        double progress = 0;
        double thresh = brainmesh1.size_of_vertices() / 100;
        int pos = 0;

        PM coord1 = get(CGAL::vertex_point, brainmesh1);
        std::vector<Surface_mesh_deformation::Point> plistdef;
        std::vector<vertex_descriptor> verlistdef;
        for (int i = 0; i < brainmesh1.size_of_vertices(); i++) {
            verlistdef.push_back(*std::next(vb1, i));
            Surface_mesh_deformation::Point p = get(coord1, *std::next(vb1, i));
            plistdef.push_back(p);
        }
        for (int i = 0; i < brainmesh1.size_of_vertices(); i++) {

            vertex_descriptor control_1 = *std::next(vb1, i);

            //Create ROI
            int ROI_rings = 0;
            std::vector <vertex_descriptor> ROI;
            ROI.push_back(control_1);
            while (ROI_rings < (number_of_rings+1)) {
                std::vector <vertex_descriptor> ROIappender = ROI;
                for (int ROIindex = 0; ROIindex < ROI.size(); ++ROIindex) {
                    adjacent_vertices(brainmesh1, ROI[ROIindex], std::back_inserter(ROIappender));
                    std::sort(ROIappender.begin(), ROIappender.end());
                    ROIappender.erase(std::unique(ROIappender.begin(), ROIappender.end()), ROIappender.end());
                }
                ROI = ROIappender;
                ++ROI_rings;
            }
            deform_mesh.insert_roi_vertices(ROI.begin(), ROI.end());

            //Select control points
            int control_rings = 0;
            std::vector <vertex_descriptor> controlpointlist;
            controlpointlist.push_back(control_1);
            while (control_rings < number_of_rings) {
                std::vector <vertex_descriptor> controlappender = controlpointlist;
                for (int controlindex = 0; controlindex < controlpointlist.size(); ++controlindex) {
                    adjacent_vertices(brainmesh1, controlpointlist[controlindex], std::back_inserter(controlappender));
                    std::sort(controlappender.begin(), controlappender.end());
                    controlappender.erase(std::unique(controlappender.begin(), controlappender.end()), controlappender.end());
                }
                controlpointlist = controlappender;
                ++control_rings;
            }
            deform_mesh.insert_control_vertices(controlpointlist.begin(), controlpointlist.end());

            bool is_matrix_factorization_OK = deform_mesh.preprocess();
            if (!is_matrix_factorization_OK) {
                line = line + 1;
                Logger("Error in preprocessing, check documentation of preprocess()");
                return EXIT_FAILURE;
            }

            Surface_mesh_deformation::Point p1;
            std::vector<double> mindistlist;
            for (int k = 0; k < controlpointlist.size(); ++k) {
                for (int l = 0; l < verlistdef.size(); ++l) {
                    if (verlistdef[l] == controlpointlist[k]) {
                        p1 = plistdef[l];
                    }
                }
                double mindist = treebrain2.squared_distance(p1);
                mindistlist.push_back(mindist);
            }
            double controlmindist=*min_element(mindistlist.begin(), mindistlist.end());
            double stepsize = controlmindist * learningrate;
            for (int k = 0; k < controlpointlist.size(); ++k) {
                for (int l = 0; l < verlistdef.size(); ++l) {
                    if (verlistdef[l] == controlpointlist[k]) {
                        p1 = plistdef[l];
                    }
                }
                //Algorithm to slowly converge to mesh2 over the normal of each vertex
                vertex_descriptor closestpoint;
                closestpoint = treebrain2.closest_point_and_primitive(p1).second->halfedge()->vertex();

                Vector vertexnormal = vnormals[closestpoint];
                //Is normal pointing outside (orientation) and check closestpoint?

                CGAL::Side_of_triangle_mesh<Polyhedron, Kernel> inside(brainmesh2);
                CGAL::Bounded_side pointinside = inside(p1);
                Surface_mesh_deformation::Point endpoint;
                if (pointinside == CGAL::ON_BOUNDED_SIDE) {
                    endpoint = Surface_mesh_deformation::Point(p1.x() + vertexnormal.x() * stepsize, p1.y() + vertexnormal.y() * stepsize, p1.z() + vertexnormal.z() * stepsize);
                }
                else if (pointinside == CGAL::ON_BOUNDARY) {
                    endpoint = p1;
                }
                else {
                    endpoint = Surface_mesh_deformation::Point(p1.x() - vertexnormal.x() * stepsize, p1.y() - vertexnormal.y() * stepsize, p1.z() - vertexnormal.z() * stepsize);
                }
                deform_mesh.set_target_position(controlpointlist[k], endpoint); // Change position of the vertix
            }

            deform_mesh.deform();

            deform_mesh.overwrite_initial_geometry();
            deform_mesh.clear_roi_vertices();
            deform_mesh.clear_control_vertices();
            if (i % 50 == 0) {
                // Save the deformed mesh into a file
                std::string filename;
                if (i == (brainmesh1.size_of_vertices()-1)) {
                    filename = "./Output/Result/Final_Vertex" + std::to_string(Epoch) + ".ply";
                }
                else {
                    filename = "./Output/" + std::to_string(Epoch) + "/Deform_Vertex" + std::to_string(i) + ".ply";
                }
                PLY(brainmesh1, filename);
            }
            else if (i == (brainmesh1.size_of_vertices()-1)) {
                // Save the deformed mesh into a file
                std::string filename = "./Output/Result/Final_Vertex" + std::to_string(Epoch) + ".ply";
                PLY(brainmesh1, filename);
            }
        }

        //loop over the vertices again to calculate the error for this epoch
        std::map<face_descriptor, Vector> fnormalsnew;
        std::map<vertex_descriptor, Vector> vnormalsnew;
        CGAL::Polygon_mesh_processing::compute_normals(brainmesh1,
            boost::make_assoc_property_map(vnormalsnew),
            boost::make_assoc_property_map(fnormalsnew));

        std::vector<double> Data_Errorlist;
        std::vector<double> Stiffness_Errorlist;
        for (int i=0; i<brainmesh1.size_of_vertices(); ++i){
            vertex_descriptor current_point = *std::next(vb1, i);

            //Data Error for each vertex
            PM newcoord1 = get(CGAL::vertex_point, brainmesh1);
            Surface_mesh_deformation::Point newp1 = get(newcoord1, current_point); 
            FT mindistancesqd = treebrain2.squared_distance(newp1);
            double Data_Error = mindistancesqd;
            Data_Errorlist.push_back(Data_Error);

            //Stiffness Error for the ROI
            Surface_mesh_deformation::Point p1;
            for (int l = 0; l < plistdef.size(); ++l) {
                if (verlistdef[l] == current_point){
                    p1 = plistdef[l];
                }
            }
            std::tuple<double, double, double> Tcenter = std::make_tuple(newp1.x() - p1.x(), newp1.y() - p1.y(), newp1.z() - p1.z());
            std::vector<vertex_descriptor> adjacentv;
            adjacent_vertices(brainmesh1, current_point, std::back_inserter(adjacentv));
            double Stiffness_Error = 0;
            //Compare with previous deformed mesh
            Surface_mesh_deformation::Point ap1;
            for (int s = 0; s != adjacentv.size(); s++) {
                for (int k = 0; k < plistdef.size(); k++) {
                    if (verlistdef[k] == adjacentv[s]) {
                        ap1 = plistdef[k];
                    }
                }
                Surface_mesh_deformation::Point anewp1 = get(newcoord1, adjacentv[s]);
                std::tuple<double, double, double> Tadjacent = std::make_tuple(anewp1.x() - ap1.x(), anewp1.y() - ap1.y(), anewp1.z() - ap1.z());
                Stiffness_Error = Stiffness_Error + pow(get<0>(Tcenter) - get<0>(Tadjacent), 2) + pow(get<1>(Tcenter) - get<1>(Tadjacent), 2) + pow(get<2>(Tcenter) - get<2>(Tadjacent), 2);
            }
            Stiffness_Errorlist.push_back(Stiffness_Error);
        }

        double Data_Errormean = std::accumulate(Data_Errorlist.begin(), Data_Errorlist.end(), 0.0) / Data_Errorlist.size();
        double Stiffness_Errormean = std::accumulate(Stiffness_Errorlist.begin(), Stiffness_Errorlist.end(), 0.0) / Stiffness_Errorlist.size();

        Error = alfa * Data_Errormean + beta * Stiffness_Errormean;
        Errorlist.push_back(Error);
        line = line + 1;
        Logger("Error: " + std::to_string(Errorlist[Epoch]));

        //Increase controlpoints/ (Maybe save brainmesh1)
        /*
        CGAL::Polygon_mesh_processing::experimental::remove_self_intersections(brainmesh1);
        //DICE Score  
        Polyhedron intersection_mesh; // Intersection of the deformed mesh and the target mesh
        Polyhedron def_mesh;
        CGAL::copy_face_graph(brainmesh1, def_mesh);
        Polyhedron comp_mesh;
        CGAL::copy_face_graph(brainmesh2, comp_mesh);       
        bool valid_intersection = CGAL::Polygon_mesh_processing::corefine_and_compute_intersection(def_mesh, comp_mesh, intersection_mesh);
        if (valid_intersection) {
            FT volume_mesh1 = CGAL::Polygon_mesh_processing::volume(def_mesh);
            FT volume_mesh2 = CGAL::Polygon_mesh_processing::volume(comp_mesh);

            FT volume_intersection = CGAL::Polygon_mesh_processing::volume(intersection_mesh); //Calculate volume of intersection

            FT Dice_score = (2 * volume_intersection) / (volume_mesh1 + volume_mesh2);
            Dicelist.push_back(Dice_score);
            line = line + 1;
            Logger("Accuracy is: " + std::to_string(Dicelist[Epoch]));
        }
        else {
            FT Dice_score = 0;
            Dicelist.push_back(Dice_score);
            line = line + 1;
            Logger("Accuracy is: " + std::to_string(Dicelist[Epoch]));
        }
        */
        Dicelist.push_back(Epoch);


        clock_t end = clock();
        double elapsed_time = double(end - begin) / CLOCKS_PER_SEC;
        total_time = total_time + elapsed_time;
        line = line + 1;
        Logger("Epoch time: " + std::to_string(elapsed_time) + " seconds.");

        //Trigger stopcriterion
        if (Epoch > 0) {
            if (std::abs(Errorlist[Epoch] - Errorlist[Epoch - 1]) < stopcriterion) {
                line = line + 3;
                Logger("Stop criterion reached, stop!");
                Logger("skip", true);
                Logger("Total time: " + std::to_string(total_time) + " seconds.");

                std::ofstream output2("./Output/Plot/ErrorAccuracy.csv");
                for (int n = 0; n < Epoch; n++) {
                    output2 << Epochlist[Epoch] << ";" << Errorlist[Epoch] << ";" << Dicelist[Epoch] << std::endl;
                }
                output2.close();

                // Save the deformed mesh into a file
                PLY(brainmesh1, "./Output/Result/Final_Mesh.ply");
                line = line + 1;
                Logger("skip", true);
                break;

                for (int i = 0; i < brainmesh1.size_of_vertices(); i++) {
                    vertex_descriptor control_1 = *std::next(vb1, i);
                    PM finalcoord1 = get(CGAL::vertex_point, brainmesh1);
                    Surface_mesh_deformation::Point finalp1 = get(finalcoord1, control_1);

                    Surface_mesh_deformation::Point beginp1;
                    for (int k = 0; k < originalvertexlist.size(); k++) {
                        if (originalvertexlist[k] == control_1) {
                            beginp1 = originalplist[k];
                        }
                    }

                    std::tuple<double, double, double> TotalTrans = std::make_tuple(finalp1.x() - beginp1.x(), finalp1.y() - beginp1.y(), finalp1.z() - beginp1.z());
                    TotalTranslist.push_back(TotalTrans);
                }
                std::ofstream output1("./Output/Transformation/Transformation.csv");
                for (int n = 0; n < TotalTranslist.size(); n++) {
                    output1 << n << ";" << get<0>(TotalTranslist[n]) << ";" << get<1>(TotalTranslist[n]) << ";" << get<2>(TotalTranslist[n]) << std::endl;
                }
                output1.close();
            }
            if (Errorlist[Epoch] > Errorlist[Epoch - 1]) {
                learningrate = learningrate / 2;
                line = line + 1;
                Logger("Error increase, step size lowered to: " + std::to_string(learningrate));
                Logger("skip", true);
                Epoch++;
                continue;
            }
            else {
                Epoch++;
                line = line + 2;
                Logger("Stop criterion not reached, continue!");
                Logger("skip", true);
                continue;
            }
        }
        
        else {
            Epoch++;
            line = line + 2;
            Logger("Stop criterion not reached, continue!");
            Logger("skip", true);
            continue;
        }
    }

    return EXIT_SUCCESS;

}