// *****************************************************************************
/*!
  \file      src/NoWarning/cg.def.h
  \copyright 2012-2015, J. Bakosi, 2016-2017, Los Alamos National Security, LLC.
  \brief     Include cg.def.h with turning off specific compiler warnings
*/
// *****************************************************************************
#ifndef nowarning_cg_def_h
#define nowarning_cg_def_h

#include "Macro.h"

#if defined(__clang__)
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wextra-semi"
  #pragma clang diagnostic ignored "-Wold-style-cast"
  #pragma clang diagnostic ignored "-Wsign-conversion"
  #pragma clang diagnostic ignored "-Wshorten-64-to-32"
  #pragma clang diagnostic ignored "-Wunused-variable"
  #pragma clang diagnostic ignored "-Wunused-parameter"
  #pragma clang diagnostic ignored "-Wundef"
#elif defined(STRICT_GNUC)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wcast-qual"
  #pragma GCC diagnostic ignored "-Wunused-variable"
  #pragma GCC diagnostic ignored "-Wunused-parameter"
  #pragma GCC diagnostic ignored "-Wswitch-default"
#endif

#include "../Inciter/cg.def.h"

#if defined(__clang__)
  #pragma clang diagnostic pop
#elif defined(STRICT_GNUC)
  #pragma GCC diagnostic pop
#endif

#endif // nowarning_cg_def_h