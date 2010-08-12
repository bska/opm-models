// $Id: transportproperties.hh 3732 2010-06-11 13:27:20Z bernd $
/*****************************************************************************
 *   Copyright (C) 2009 by Markus Wolff                                      *
 *   Institute of Hydraulic Engineering                                      *
 *   University of Stuttgart, Germany                                        *
 *   email: <givenname>.<name>@iws.uni-stuttgart.de                          *
 *                                                                           *
 *   This program is free software; you can redistribute it and/or modify    *
 *   it under the terms of the GNU General Public License as published by    *
 *   the Free Software Foundation; either version 2 of the License, or       *
 *   (at your option) any later version, as long as this copyright notice    *
 *   is included in its original form.                                       *
 *                                                                           *
 *   This program is distributed WITHOUT ANY WARRANTY.                       *
 *****************************************************************************/
#ifndef DUMUX_TRANSPORT_PROPERTIES_HH
#define DUMUX_TRANSPORT_PROPERTIES_HH


/*!
 * \file
 * \brief Specify the shape functions, operator assemblers, etc
 *        used for the BoxScheme.
 */
namespace Dumux
{

template<class TypeTag>
class DiffusivePart;

template<class TypeTag>
class ConvectivePart;

template<class TypeTag>
class TwoPCommonIndices;

template<class TypeTag>
class FluidSystem2P;

template<class TypeTag>
class TwoPFluidState;

template<class TypeTag>
class VariableClass2P;

namespace Properties
{
/*!
 * \addtogroup diffusion
 */
// \{

//////////////////////////////////////////////////////////////////
// Type tags tags
//////////////////////////////////////////////////////////////////

//! The type tag for models based on the diffusion-scheme
NEW_TYPE_TAG(Transport);

//////////////////////////////////////////////////////////////////
// Property tags
//////////////////////////////////////////////////////////////////

NEW_PROP_TAG( DiffusivePart );         //!< The type of the diffusive part in a transport equation
NEW_PROP_TAG( ConvectivePart );        //!< The type of a convective part in a transport equation
NEW_PROP_TAG( Variables );
NEW_PROP_TAG( NumPhases );
NEW_PROP_TAG( NumComponents );
NEW_PROP_TAG( TwoPIndices );
NEW_PROP_TAG( FluidSystem );
NEW_PROP_TAG( FluidState );
NEW_PROP_TAG( EnableCompressibility );
NEW_PROP_TAG( PressureFormulation );
NEW_PROP_TAG( SaturationFormulation );
NEW_PROP_TAG( VelocityFormulation );
NEW_PROP_TAG( CFLFactor );

SET_TYPE_PROP(Transport, DiffusivePart, DiffusivePart<TypeTag>);
SET_TYPE_PROP(Transport, ConvectivePart, ConvectivePart<TypeTag>);
SET_TYPE_PROP(Transport, Variables, VariableClass2P<TypeTag>);
SET_INT_PROP(Transport, NumPhases, 2);
SET_INT_PROP(Transport, NumComponents, 1);
SET_TYPE_PROP(Transport, TwoPIndices, TwoPCommonIndices<TypeTag>);
SET_TYPE_PROP(Transport, FluidSystem, FluidSystem2P<TypeTag>);
SET_TYPE_PROP(Transport, FluidState, TwoPFluidState<TypeTag>);
SET_BOOL_PROP(Transport, EnableCompressibility, false);
SET_INT_PROP(Transport, PressureFormulation, TwoPCommonIndices<TypeTag>::pressureW);
SET_INT_PROP(Transport, SaturationFormulation, TwoPCommonIndices<TypeTag>::saturationW);
SET_INT_PROP(Transport, VelocityFormulation, TwoPCommonIndices<TypeTag>::velocityTotal);
SET_SCALAR_PROP(Transport, CFLFactor, 1.0);
}
}

#endif
