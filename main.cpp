#pragma region Include Libaries
#include <CGAL/Simple_cartesian.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/draw_polyhedron.h>
#include <CGAL/Surface_mesh_default_triangulation_3.h>
#include <CGAL/Complex_2_in_triangulation_3.h>
#include <CGAL/Surface_mesh/IO/PLY.h>
#include <CGAL/IO/PLY_reader.h>
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
#include <CGAL/Polygon_mesh_processing/compute_normal.h>
#include <CGAL/mesh_segmentation.h>
#include <fstream>
#include <string>
#include <iostream>
#include <omp.h>
//#include <matplotlibcpp.h>
#pragma endregion

#pragma region Typedefs
typedef CGAL::Simple_cartesian<double>                                       Kernel;
typedef CGAL::Surface_mesh_default_triangulation_3                           Tr;
typedef Tr::Geom_traits                                                      GT;
typedef GT::Point_3                                                          Point;
typedef Kernel::Vector_3                                                     Vector;

typedef GT::FT                                                               FT;
typedef FT(*Function)(Point);
typedef CGAL::Surface_mesh<Point>                                            Mesh;
typedef CGAL::Polyhedron_3<Kernel, CGAL::Polyhedron_items_with_id_3>         Polyhedron;
typedef boost::graph_traits<Polyhedron>::vertex_descriptor                   vertex_descriptor;
typedef boost::graph_traits<Polyhedron>::vertex_iterator                     vertex_iterator;
typedef boost::graph_traits<Polyhedron>::halfedge_descriptor                 halfedge_descriptor;
typedef boost::graph_traits<Polyhedron>::edge_descriptor                     edge_descriptor;
typedef boost::graph_traits<Polyhedron>::face_descriptor                     face_descriptor;
typedef CGAL::Halfedge_around_target_iterator<Polyhedron>                    halfedge_around_target_iterator;
typedef CGAL::Surface_mesh_deformation<Polyhedron>                           Surface_mesh_deformation;
typedef boost::property_map<Polyhedron, CGAL::vertex_point_t>::type          PM;
namespace PMP = CGAL::Polygon_mesh_processing;
typedef PMP::Face_location<Polyhedron, FT>                                   Face_location;
typedef typename Polyhedron::Vertex_const_iterator                           VCI;
typedef typename Polyhedron::Facet_const_iterator                            FCI;
typedef typename Polyhedron::Halfedge_around_facet_const_circulator          HFCC;
typedef CGAL::AABB_face_graph_triangle_primitive<Polyhedron>                 Primitive;
typedef CGAL::AABB_traits<Kernel, Primitive>                                 Traits;
typedef CGAL::AABB_tree<Traits>                                              Tree;
//namespace plt = matplotlibcpp;
#pragma endregion

#pragma region Find adjacent vertices
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
#pragma endregion

int main(int argc, char* argv[])
{
    #pragma region Load Brains
    //Brain 1
    Mesh sm1;
    std::ifstream in1((argc > 1) ? argv[1] : "C:/Users/20181933/Documents/Jaar 3/Kwartiel 4/BEP Medical Imaging/Data/Littlesegment.ply");
    CGAL::read_ply(in1, sm1);
    Polyhedron original;
    CGAL::copy_face_graph(sm1, original);
    PM originalcoord = get(CGAL::vertex_point, original);

    Polyhedron brainmesh1;
    CGAL::copy_face_graph(sm1, brainmesh1);
    CGAL::draw(brainmesh1);

    //Brain 2
    Mesh sm2;
    std::ifstream in2((argc > 2) ? argv[2] : "C:/Users/20181933/Documents/Jaar 3/Kwartiel 4/BEP Medical Imaging/Data/LittlesegmentBig.ply");
    CGAL::read_ply(in2, sm2);
    Polyhedron brainmesh2;
    CGAL::copy_face_graph(sm2, brainmesh2);
    CGAL::draw(brainmesh2);
    #pragma endregion

    #pragma region Rigid Registration of Brains
        //Registration van center en rotation.
        //punt-Centerofmass zodat alles op 0,0,0
    #pragma endregion

    #pragma region Init the indices of the halfedges and the vertices
    set_halfedgeds_items_id(brainmesh1);
    set_halfedgeds_items_id(brainmesh2);

    // Create a deformation object
    Surface_mesh_deformation deform_mesh(brainmesh1);

    vertex_iterator vb1, ve1;
    boost::tie(vb1, ve1) = vertices(brainmesh1);
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

    #pragma region Parameters
    int endEpoch;
    std::cout << "Epochs: ";
    std::cin >> endEpoch;
    double learningrate; //Literatuur
    std::cout << "Learning rate: ";
    std::cin >> learningrate;
    int number_of_rings;
    std::cout << "Number of rings in ROI: ";
    std::cin >> number_of_rings;
    double alfa = 0.1; //Literatuur waardes
    double beta = 0.1; //Literatuur waardes
    double stopcriterion = 0.0001; //Literatuur waardes (0) wat is klein genoeg
    #pragma endregion

    int Epoch = 0;
    #pragma endregion
   
    #pragma region Algorithm for deformation
    while(Epoch < endEpoch) {
        Epochlist.push_back(Epoch);
        double Error = 0;
        std::cout << std::endl;
        std::cout << "Epoch: " << Epoch << std::endl;

        //Compute normals for each vertex
        std::map<face_descriptor, Vector> fnormals;
        std::map<vertex_descriptor, Vector> vnormals;
        CGAL::Polygon_mesh_processing::compute_normals(brainmesh1,
            boost::make_assoc_property_map(vnormals),
            boost::make_assoc_property_map(fnormals));

        //Loop over all vertices in brain1
        double progress = 0;
        double thresh = brainmesh1.size_of_vertices() / 100;
        int pos = 0;

        std::vector<double> Data_Errorlist;
        std::vector<double> Stiffness_Errorlist;
        std::vector<double> Landmark_Errorlist;

        for (int i = 0; i < 1; i++) {

            #pragma region Loadbar per Epoch
            int barWidth = 100;
            if (progress > thresh) {
                pos++;
                thresh = thresh + brainmesh1.size_of_vertices() / 100;
            }
            std::cout << "[";
            for (int k = 0; k < barWidth; ++k) {
                if (k < pos) std::cout << "=";
                else if (k == pos) std::cout << ">";
                else std::cout << " ";
            }
            std::cout << "] " << int(progress / brainmesh1.size_of_vertices() * 100.0) << " %\r";
            std::cout.flush();
            progress++;
            #pragma endregion

            vertex_descriptor control_1 = *std::next(vb1, i);

            int ROI_rings = 0;
            std::vector <vertex_descriptor> ROI;
            ROI.push_back(control_1);
            while (ROI_rings < number_of_rings) {
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

            PM coord1 = get(CGAL::vertex_point, brainmesh1);
            Surface_mesh_deformation::Point p1 = get(coord1, control_1);

            deform_mesh.insert_control_vertex(control_1);

            //brainmesh2 is target
            bool is_matrix_factorization_OK = deform_mesh.preprocess();
            if (!is_matrix_factorization_OK) {
                std::cerr << "Error in preprocessing, check documentation of preprocess()" << std::endl;
                return 1;
            }
            //Algorithm to slowly converge to mesh2 over the normal of each vertex
            Vector vertexnormal = vnormals[control_1];

            CGAL::Side_of_triangle_mesh<Polyhedron, Kernel> inside(brainmesh2);
            CGAL::Bounded_side pointinside = inside(p1);

            Surface_mesh_deformation::Point endpoint;

            if (pointinside == CGAL::ON_BOUNDED_SIDE) {
                endpoint = Surface_mesh_deformation::Point(p1.x() + vertexnormal.x() * learningrate, p1.y() + vertexnormal.y() * learningrate, p1.z() + vertexnormal.z() * learningrate);
            }
            else if (pointinside == CGAL::ON_BOUNDARY) {
                endpoint = p1;
            }
            else {
                endpoint = Surface_mesh_deformation::Point(p1.x() - vertexnormal.x() * learningrate, p1.y() - vertexnormal.y() * learningrate, p1.z() - vertexnormal.z() * learningrate);
            }

            deform_mesh.set_target_position(control_1, endpoint); // Change position of the vertix
            deform_mesh.deform();
        //loop over the vertices again to calculate the error for this epoch
            //Data Error for the ROI for each vertex
            PM newcoord1 = get(CGAL::vertex_point, brainmesh1);
            Surface_mesh_deformation::Point newp1 = get(newcoord1, control_1);
            Tree treebrain2(faces(brainmesh2).first, faces(brainmesh2).second, brainmesh2);
            Surface_mesh_deformation::Point closestpoint = treebrain2.closest_point(newp1);
            FT mindistancesqd = treebrain2.squared_distance(newp1);
            double Data_Error = mindistancesqd;
            Data_Errorlist.push_back(Data_Error);

            //Stiffness Error for the ROI
            std::tuple<double, double, double> Tcenter = std::make_tuple(newp1.x() - p1.x(), newp1.y() - p1.y(), newp1.z() - p1.z());
            std::vector<vertex_descriptor> adjacentv;
            adjacent_vertices(brainmesh1, control_1, std::back_inserter(adjacentv));
            double Stiffness_Error = 0;
            //Compare with previous deformed mesh
            Surface_mesh_deformation::Point ap1;
            for (int s = 0; s != adjacentv.size(); s++) {
                for (int k = 0; k < originalplist.size(); k++) {
                    if (originalvertexlist[k] == adjacentv[s]) {
                        ap1 = originalplist[k];
                    }
                }
                Surface_mesh_deformation::Point anewp1 = get(newcoord1, adjacentv[s]);
                std::tuple<double, double, double> Tadjacent = std::make_tuple(anewp1.x() - ap1.x(), anewp1.y() - ap1.y(), anewp1.z() - ap1.z());
                Stiffness_Error = Stiffness_Error + pow(get<0>(Tcenter) - get<0>(Tadjacent), 2) + pow(get<1>(Tcenter) - get<1>(Tadjacent), 2) + pow(get<2>(Tcenter) - get<2>(Tadjacent), 2);
            }
            Stiffness_Errorlist.push_back(Stiffness_Error);

            //Landmark Error
            double Landmark_Error = 0;
            Landmark_Errorlist.push_back(Landmark_Error);

            deform_mesh.overwrite_initial_geometry();
            deform_mesh.clear_roi_vertices();
            deform_mesh.clear_control_vertices();
        }
        double Data_Errormean = std::accumulate(Data_Errorlist.begin(), Data_Errorlist.end(), 0.0) / Data_Errorlist.size();
        double Stiffness_Errormean = std::accumulate(Stiffness_Errorlist.begin(), Stiffness_Errorlist.end(), 0.0) / Stiffness_Errorlist.size();
        double Landmark_Errormean = std::accumulate(Landmark_Errorlist.begin(), Landmark_Errorlist.end(), 0.0) / Landmark_Errorlist.size();

        Error = Data_Errormean + alfa * Stiffness_Errormean + beta * Landmark_Errormean;
        Errorlist.push_back(Error);
        std::cout << "Error: " << Errorlist[Epoch] << std::endl;

        //DICE Score  
        Polyhedron intersection_mesh; // Intersection of the deformed mesh and the target mesh
        Polyhedron def_mesh;
        CGAL::copy_face_graph(brainmesh1, def_mesh);
        Polyhedron comp_mesh;
        CGAL::copy_face_graph(brainmesh2, comp_mesh);       
        bool valid_intersection = CGAL::Polygon_mesh_processing::corefine_and_compute_intersection(def_mesh, comp_mesh, intersection_mesh);
        std::cout << "No error" << std::endl;
        if (valid_intersection) {
            FT volume_mesh1 = CGAL::Polygon_mesh_processing::volume(def_mesh);
            FT volume_mesh2 = CGAL::Polygon_mesh_processing::volume(comp_mesh);

            FT volume_intersection = CGAL::Polygon_mesh_processing::volume(intersection_mesh); //Calculate volume of intersection

            FT Dice_score = (2 * volume_intersection) / (volume_mesh1 + volume_mesh2);
            Dicelist.push_back(Dice_score);
            std::cout << "The Dice score is: " << Dicelist[Epoch] << std::endl;
        }
        else {
            FT Dice_score = 0;
            Dicelist.push_back(Dice_score);
            std::cout << "No Intersection, Dice score is: " << Dicelist[Epoch] << std::endl;
        }
                
        //Trigger stopcriterion
        if (Epoch > 0) {
            if (Errorlist[Epoch] > Errorlist[Epoch - 1]) {
                learningrate = learningrate / 2;
                std::cout << "Error increase, step size lowered to: " << learningrate << std::endl;
                Epoch++;
                continue;
            }
            else if (std::abs(Errorlist[Epoch]-Errorlist[Epoch-1])<stopcriterion) {
                std::cout << "Stop criterion reached, stop!" << std::endl;

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
                    std::cout << "Total Transformation for Vertex " << i << " is " << get<0>(TotalTranslist[i]) << " " << get<1>(TotalTranslist[i]) << " " << get<2>(TotalTranslist[i]) << "." << std::endl;
                }

                /* Python
                //Plot Error and Accuracy
                plt::figure_size(1280, 720);
                plt::named_plot("Error", Epochlist, Errorlist);
                plt::xlim(0.0, Epochlist[Epoch]);
                plt::title("Error of deformation for each Epoch.");
                plt::legend();
                plt::save("./ErrorFigure.pdf");
                
                plt::figure_size(1280, 720);
                plt::named_plot("Accuracy (Dice Score)", Epochlist, Dicelist);
                plt::xlim(0.0, Epochlist[Epoch]);
                plt::title("Accuracy (Dice Score) of deformation for each Epoch.");
                plt::legend();
                plt::save("./AccuracyFigure.pdf");
                */

                //Draw deformed surface mesh
                CGAL::draw(brainmesh1);

                #pragma region Export as PLY file
                // Save the deformed mesh into a file
                std::filebuf fb;
                fb.open("Brain_1_deform.ply", std::ios::out);
                std::ostream os(&fb);
                os << "ply\n"
                    << "format ascii 1.0\n"
                    << "element vertex " << brainmesh1.size_of_vertices() << "\n"
                    << "property float x\n"
                    << "property float y\n"
                    << "property float z\n"
                    << "element face " << brainmesh1.size_of_facets() << "\n"
                    << "property list uchar int vertex_index\n"
                    << "end_header\n";
                for (auto it = brainmesh1.vertices_begin(); it != brainmesh1.vertices_end(); it++) {
                    os << it->point().x() << " " << it->point().y() << " " << it->point().z() << std::endl;
                }
                typedef CGAL::Inverse_index<VCI> Index;
                Index index(brainmesh1.vertices_begin(), brainmesh1.vertices_end());
                for (FCI fi = brainmesh1.facets_begin(); fi != brainmesh1.facets_end(); ++fi) {
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
                #pragma endregion

                break;
            }
            else {
                Epoch++;
                std::cout << "Stop criterion not reached, continue!" << std::endl;
                std::cout << std::endl;
                continue;
            }
        }
        
        else {
            Epoch++;
            std::cout << "Stop criterion not reached, continue!" << std::endl;
            std::cout << std::endl;
            continue;
        }
    }
    #pragma endregion

    return EXIT_SUCCESS;

}