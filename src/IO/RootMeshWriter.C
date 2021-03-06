// *****************************************************************************
/*!
  \file      src/IO/RootMeshWriter.C
  \copyright 2016-2017, Los Alamos National Security, LLC.
  \brief     Root mesh-based data writer
  \details   Root mesh-based data writer class definition.
*/
// *****************************************************************************

#include <algorithm>
#include <functional>
#include <iterator>
#include <string>
#include <utility>

#include "RootMeshWriter.h"
#include "Exception.h"
#include "UnsMesh.h"

using tk::RootMeshWriter;

RootMeshWriter::RootMeshWriter( const std::string filename, int option ) :
  m_filename( filename )
// *****************************************************************************
//  Constructor: create/open Root file
//! \param[in] "filename" File to open as Root file
//! \param[in] mode Root writer constructor mode: ExoWriter::CREATE for
//!   creating a new file, ExoWriter::OPEN for opening an existing file for
//!   appending
// *****************************************************************************
{
  if (option == 0 ) {

    m_rfile = new TFile( filename.c_str(), "RECREATE" );
    m_tree_connect = new TTree ( "ctree", "store the connectivity" );

  } else if (option == 1) {

      m_rfile = TFile::Open( filename.c_str(), "UPDATE" );
      m_tree_connect = static_cast< TTree* >( m_rfile->Get( "ctree" ) );

  } else Throw( "Root Mesh modes not supported" );
}

RootMeshWriter::~RootMeshWriter() noexcept
// *****************************************************************************
//  Destructor
// *****************************************************************************
{
  if (m_rfile) {
    m_rfile->Write();
    m_rfile->Close();
  }
}

void
RootMeshWriter::writeMesh( const UnsMesh& mesh )
// *****************************************************************************
//  Write Root mesh file
//! \param[in] mesh Unstructured mesh object
// *****************************************************************************
{
  writeHeader( mesh );
  writeNodes( mesh );
  writeElements( mesh );
}

void
RootMeshWriter::writeHeader( const UnsMesh& mesh )
// *****************************************************************************
//  Write Root header
//! \param[in] mesh Unstructured mesh object
// *****************************************************************************
{

  m_csobject = new mesh_data( mesh.size(),
                            mesh.tetinpoel().size() + mesh.triinpoel().size() );

}

void
RootMeshWriter::writeNodes( const UnsMesh& mesh )
// *****************************************************************************
//  Write node coordinates to Root file
//! \param[in] mesh Unstructured mesh object
// *****************************************************************************
{
  // the file requires the vertices and the number of triangles
  // 4 triangles per tetrahedron and mesh.tetinpoel() stores 4 
  // vertices per tet in the vector (# vertices = # of triangles)

  m_tree_connect->Branch( "coord", &m_csobject->coordinates, "coordinates/I" );
  m_tree_connect->Branch( "trian", &m_csobject->triangles, "triangles/I" );
  
  m_tree_connect->Branch( "x_coord", &m_csobject->mx_root );
  m_tree_connect->Branch( "y_coord", &m_csobject->my_root );
  m_tree_connect->Branch( "z_coord", &m_csobject->mz_root );

  auto c = static_cast< std::size_t >( m_csobject->coordinates );
  for (std::size_t i=0 ; i<c; ++i) {
    m_csobject->mx_root.push_back( mesh.x()[i] );
    m_csobject->my_root.push_back( mesh.y()[i] );
    m_csobject->mz_root.push_back( mesh.z()[i] );
  }
}

void
RootMeshWriter::writeElements( const UnsMesh& mesh ) const
// *****************************************************************************
//  Write element connectivity to Root file
//! \param[in] mesh Unstructured mesh object
// *****************************************************************************
{
  int elclass = 0;

  writeElemBlock( elclass, mesh.triinpoel() );
  writeElemBlock( elclass, mesh.tetinpoel() );
}

void
RootMeshWriter::writeElemBlock( int& elclass,
                                const std::vector< std::size_t >& inpoel )
const
// *****************************************************************************
//  Write element block to ROOT file
//! \param[inout] elclass Count element class ids in file
//! \param[in] inpoel Element connectivity.
// *****************************************************************************
{
  if (inpoel.empty()) return;

  // increase number of element classes in file
  ++elclass;

  // Make sure element connectivity starts with zero
  Assert( *std::minmax_element( begin(inpoel), end(inpoel) ).first == 0,
          "node ids should start from zero" );
  
  // create a branch for storing the tetrahedrons  
  m_tree_connect->Branch( "tetconnect", &m_csobject->tet_connect );
  for ( auto itr = inpoel.begin(); itr != inpoel.end(); itr++ )
    m_csobject->tet_connect.push_back( *itr );

  m_tree_connect->Fill();
  m_tree_connect->Write(); 

}

void
RootMeshWriter::writeNodeVarNames( const std::vector< std::string >& nv )
const
// *****************************************************************************
//  Write the names of nodal output variables to ROOT file
//! \param[in] nv Nodal variable names
// *****************************************************************************
{
  std::vector < std::string > nv_copy;
  TBranch *branch = m_tree_connect->Branch( "variables", &nv_copy );

  for ( auto itr = nv.begin(); itr != nv.end(); itr++)
    nv_copy.push_back( *itr );

  branch->Fill();
  m_tree_connect->Write();

}

void
RootMeshWriter::writeTimeStamp( uint64_t it, tk::real time )
// *****************************************************************************
//  Write time stamp to ROOT file
//! \param[in] it Iteration number
//! \param[in] time Time
// *****************************************************************************
{
  
  // create a friend tree to the main tree
  std::string tree = "tf_var_" + std::to_string(it);
  m_friendTree = new TTree ( tree.c_str(), "friend trees" );

  // create a branch for the time step
  tk::real elapsed_time = time;
  std::string branch_time = "time_branch_" + std::to_string(it);
  m_friendTree->Branch( branch_time.c_str(), &elapsed_time, "elapsed_time/D" );

  m_friendTree->Fill();
  m_tree_connect->AddFriend( tree.c_str() );

}

void
RootMeshWriter::writeNodeScalar( uint64_t it,
                                     int varid,
                                     const std::vector< tk::real >& var ) const
// *****************************************************************************
//  Write node scalar field to ROOT file
//! \param[in] it Iteration number
//! \param[in] varid Variable id
//! \param[in] var Vector of variable to output
// *****************************************************************************
{

  std::vector< tk::real > vec_copy;

  std::string branch_field = "branch_" + std::to_string(it) + "_field_" + 
			      std::to_string(varid);
  //create a Branch
  m_friendTree->Branch( branch_field.c_str(), &vec_copy );
  
  //copy the values
  for( auto itr = var.begin(); itr != var.end(); itr++ )
    vec_copy.push_back( *itr );

  m_friendTree->Fill();
  m_friendTree->Write();

}

