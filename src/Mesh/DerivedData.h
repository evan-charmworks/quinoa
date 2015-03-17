//******************************************************************************
/*!
  \file      src/Mesh/DerivedData.h
  \author    J. Bakosi
  \date      Tue 17 Mar 2015 07:44:42 AM MDT
  \copyright 2012-2015, Jozsef Bakosi.
  \brief     Generate data structures derived from unstructured mesh
  \details   Generate data structures derived from the connectivity information
     of an unstructured mesh.
*/
//******************************************************************************
#ifndef DerivedData_h
#define DerivedData_h

#include <UnsMesh.h>

namespace tk {

//! Shift node IDs to start with zero in element connectivity
void shiftToZero( std::vector< int >& inpoel );

//! Generate derived data structure, elements surrounding points
std::pair< std::vector< std::size_t >, std::vector< std::size_t > >
genEsup( const std::vector< int >& inpoel, std::size_t npoin );

} // tk::

#endif // DerivedData_h