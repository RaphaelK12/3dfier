
#include "Polygon3d.h"
#include "input.h"

Polygon3d::Polygon3d(Polygon2d* p, std::string id) {
  _id = id;
  _p2 = p;
}

Polygon3d::~Polygon3d() {
  // TODO: clear memory properly
  std::cout << "I am dead" << std::endl;
}

Box Polygon3d::get_bbox2d() {
  return bg::return_envelope<Box>(*_p2);
}

std::string Polygon3d::get_id() {
  return _id;
}

Polygon2d* Polygon3d::get_polygon2d() {
    return _p2;
}



//-------------------------------
//-------------------------------

Polygon3dBlock::Polygon3dBlock(Polygon2d* p, std::string id, std::string lifttype) : Polygon3d(p, id) 
{
  _lifttype = lifttype;
}

std::string Polygon3dBlock::get_lift_type() {
  return _lifttype;
}

bool Polygon3dBlock::threeDfy() {
  return true;
}

std::string Polygon3dBlock::get_3d_citygml() {
  std::stringstream ss;
  ss << "<cityObjectMember>";
  ss << "<bldg:Building>";
  ss << "<bldg:measuredHeight uom=\"#m\">";
  ss << this->get_height();
  ss << "</bldg:measuredHeight>";
  ss << "<bldg:lod1Solid>";
  ss << "<gml:Solid>";
  ss << "<gml:exterior>";
  ss << "<gml:CompositeSurface>";
  //-- get floor
  ss << get_polygon_lifted_gml(this->_p2, 0, false);
  //-- get roof
  ss << get_polygon_lifted_gml(this->_p2, this->get_height(), true);
  //-- get the walls
  auto r = bg::exterior_ring(*(this->_p2));
  for (int i = 0; i < (r.size() - 1); i++) 
    ss << get_extruded_line_gml(&r[i], &r[i + 1], this->get_height(), 0, false);
  ss << "</gml:CompositeSurface>";
  ss << "</gml:exterior>";
  ss << "</gml:Solid>";
  ss << "</bldg:lod1Solid>";
  ss << "</bldg:Building>";
  ss << "</cityObjectMember>";
  return ss.str(); 
}

std::string Polygon3dBlock::get_3d_csv() {
  std::stringstream ss;
  ss << this->get_id() << ";" << this->get_height() << std::endl;
  return ss.str(); 
}

std::string Polygon3dBlock::get_obj_v() {
  return "EMPTY";
}

std::string Polygon3dBlock::get_obj_f() {
  return "EMPTY";
}

double Polygon3dBlock::get_height() {
  // TODO : return an error if no points
  if (_zvalues.size() == 0)
    return -999;
  std::string t = _lifttype.substr(_lifttype.find_first_of("-") + 1);
  if (t == "MAX") {
    double v = -99999;
    for (auto z : _zvalues) {
      if (z > v)
        v = z;
    }
    return v;
  }
  else if (t == "MIN") {
    double v = 99999;
    for (auto z : _zvalues) {
      if (z < v)
        v = z;
    }
    return v;
  }
  else if (t == "AVG") {
    double sum = 0.0;
    for (auto z : _zvalues) 
      sum += z;
    return (sum / _zvalues.size());
  }
  else if (t == "MEDIAN") {
    std::nth_element(_zvalues.begin(), _zvalues.begin() + (_zvalues.size() / 2), _zvalues.end());
    return _zvalues[_zvalues.size() / 2];
  }
  else {
    std::cout << "UNKNOWN HEIGHT" << std::endl;
  }
  return -9999;
}


bool Polygon3dBlock::add_elevation_point(double x, double y, double z) {
  _zvalues.push_back(z);
  return true;
}


//-------------------------------
//-------------------------------

Polygon3dBoundary::Polygon3dBoundary(Polygon2d* p, std::string id) : Polygon3d(p, id) 
{
}

std::string Polygon3dBoundary::get_lift_type() {
  return "BOUNDARY3D";
}

bool Polygon3dBoundary::threeDfy() {
  build_CDT();
  return true;
}

std::string Polygon3dBoundary::get_3d_citygml() {
  build_CDT();
  std::stringstream ss;
  ss << "# vertices: " << _vertices.size() << std::endl;
  ss << "# triangles: " << _triangles.size() << std::endl;
  return ss.str(); 
}

std::string Polygon3dBoundary::get_3d_csv() {
  return "EMPTY"; 
}

std::string Polygon3dBoundary::get_obj_v() {
  std::stringstream ss;
  for (auto& v : _vertices)
    ss << "v\t" << bg::get<0>(v) << "\t" << bg::get<1>(v) << "\t" << bg::get<2>(v) << std::endl;
  return ss.str();
}

std::string Polygon3dBoundary::get_obj_f() {
  std::stringstream ss;
 for (auto& t : _triangles)
    ss << "f\t" << (t.v0 + 1) << "\t" << (t.v1 + 1) << "\t" << (t.v2 + 1) << std::endl;
  return ss.str();
}


bool Polygon3dBoundary::add_elevation_point(double x, double y, double z) {
  _lidarpts.push_back(Point3d(x, y, z));
  return true;
}

bool Polygon3dBoundary::build_CDT() {
  std::cout << "is valid? " << (bg::is_valid(*_p2) ? "yes" : "no") << std::endl;
  getCDT(_p2, _vertices, _triangles);
  return true;
}

//-------------------------------
//-------------------------------

Polygon3dTin::Polygon3dTin(Polygon2d* p, std::string id, std::string lifttype) : Polygon3d(p, id) 
{
  _lifttype = lifttype;
}

std::string Polygon3dTin::get_lift_type() {
  return _lifttype;
}

bool Polygon3dTin::threeDfy() {
  build_CDT();
  return true;
}

std::string Polygon3dTin::get_3d_citygml() {
  
  // std::stringstream ss;
  // std::cout << "-----LIDAR POINTS-----" << std::endl;
  // for (auto& pt : _lidarpts)
  //   std::cout << std::setprecision(12) << bg::get<0>(pt) << ", " << bg::get<1>(pt) << ", " << bg::get<2>(pt) << std::endl;
  // // ss << "# vertices: " << _vertices.size() << std::endl;
  // // ss << "# triangles: " << _triangles.size() << std::endl;
  // // return ss.str();
  build_CDT();
  return "done.";
}

std::string Polygon3dTin::get_3d_csv() {
  return "EMPTY"; 
}

std::string Polygon3dTin::get_obj_v() {
  std::stringstream ss;
  for (auto& v : _vertices)
    ss << "v\t" << bg::get<0>(v) << "\t" << bg::get<1>(v) << "\t" << bg::get<2>(v) << std::endl;
  return ss.str();
}

std::string Polygon3dTin::get_obj_f() {
  std::stringstream ss;
 for (auto& t : _triangles)
    ss << "f\t" << (t.v0 + 1) << "\t" << (t.v1 + 1) << "\t" << (t.v2 + 1) << std::endl;
  return ss.str();
}


bool Polygon3dTin::add_elevation_point(double x, double y, double z) {
  _lidarpts.push_back(Point3d(x, y, z));
  return true;
}

bool Polygon3dTin::build_CDT() {
  std::cout << "is valid? " << (bg::is_valid(*_p2) ? "yes" : "no") << std::endl;
  getCDT(_p2, _vertices, _triangles, _lidarpts);
  return true;
}

