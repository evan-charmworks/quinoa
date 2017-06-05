#ifndef AMR_types_h
#define AMR_types_h

#include <array>
#include <map>
#include "Refinement_State.h"

const int DIMENSION = 3;
const size_t NUM_TET_NODES = 4;
const size_t NUM_FACE_NODES = 3;
const size_t NUM_TET_EDGES = 6;
const size_t NUM_TET_FACES = 4;
const size_t ID_SHIFT = 3;
const size_t MAX_CHILDREN = 8;
const char KEY_DELIM = '-';

using real_t = tk::real;
using coordinate_t = std::array<real_t, DIMENSION>;
using tet_t = std::array<size_t, NUM_TET_NODES>;
using edge_list_ids_t  = std::array<std::size_t, NUM_TET_EDGES>;
using edge_list_t  = std::array<std::string, NUM_TET_EDGES>;
using node_pair_t  = std::array<std::size_t, 2>; // TODO: should this be a std::pair

using face_ids_t = std::array<std::size_t, NUM_FACE_NODES>;
using face_list_t  = std::array< face_ids_t, NUM_TET_FACES>;

using child_id_list_t = std::array<size_t, MAX_CHILDREN>;

using tet_list_t = std::map<size_t, tet_t>;

using inpoel_t = std::vector< std::size_t >;     //!< Tetrahedron connectivity
using node_list_t = std::vector<real_t>;

using coord_type = std::vector< tk::real >;

// Complex types
using edges_t = std::map<std::string, AMR::Edge_Refinement>;
#endif
