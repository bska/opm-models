/*****************************************************************************
 *   See the file COPYING for full copying permissions.                      *
 *                                                                           *
 *   This program is free software: you can redistribute it and/or modify    *
 *   it under the terms of the GNU General Public License as published by    *
 *   the Free Software Foundation, either version 2 of the License, or       *
 *   (at your option) any later version.                                     *
 *                                                                           *
 *   This program is distributed in the hope that it will be useful,         *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the            *
 *   GNU General Public License for more details.                            *
 *                                                                           *
 *   You should have received a copy of the GNU General Public License       *
 *   along with this program. If not, see <http://www.gnu.org/licenses/>.    *
 *****************************************************************************/
/*!
 * \file
 *
 * \brief Defines default values for the properties required by the
 *        2pDFM box model.
 *
 * \ingroup Properties
 * \ingroup TwoPDFMBoxModel
 * \ingroup BoxProperties
 */
#ifndef EWOMS_BOXMODELS_2PDFM_PROPERTY_DEFAULTS_HH
#define EWOMS_BOXMODELS_2PDFM_PROPERTY_DEFAULTS_HH

#include <dumux/boxmodels/common/boxdarcyfluxvariables.hh>
#include <dumux/material/components/nullcomponent.hh>
#include <dumux/material/fluidstates/immisciblefluidstate.hh>
#include <dumux/material/fluidsystems/2pimmisciblefluidsystem.hh>
#include <dumux/material/fluidsystems/gasphase.hh>
#include <dumux/material/fluidsystems/liquidphase.hh>
#include <dumux/material/spatialparams/boxspatialparams.hh>

#include "2pdfmmodel.hh"
#include "2pdfmproblem.hh"
#include "2pdfmindices.hh"
#include "2pdfmfluxvariables.hh"
#include "2pdfmvolumevariables.hh"
#include "2pdfmproperties.hh"

namespace Ewoms
{
namespace Properties
{
//////////////////////////////////////////////////////////////////
// Property defaults
//////////////////////////////////////////////////////////////////
SET_INT_PROP(BoxTwoPDFM, NumEq, 2); //!< set the number of equations to 2
SET_INT_PROP(BoxTwoPDFM, NumPhases, 2); //!< The number of phases in the 2p model is 2

//! Set the default formulation to pWsN
SET_INT_PROP(BoxTwoPDFM,
             Formulation,
             TwoPFormulation::pwSn);

//! Use the 2p local jacobian operator for the 2p model
SET_TYPE_PROP(BoxTwoPDFM,
              LocalResidual,
              TwoPDFMLocalResidual<TypeTag>);

//! the Model property
SET_TYPE_PROP(BoxTwoPDFM, Model, TwoPDFMModel<TypeTag>);

//! the VolumeVariables property
SET_TYPE_PROP(BoxTwoPDFM, VolumeVariables, TwoPDFMVolumeVariables<TypeTag>);

//! the FluxVariables property
SET_TYPE_PROP(BoxTwoPDFM, FluxVariables, TwoPDFMFluxVariables<TypeTag>);

//! the upwind weight for the mass conservation equations.
SET_SCALAR_PROP(BoxTwoPDFM, ImplicitMassUpwindWeight, 1.0);

//! weight for the upwind mobility in the velocity calculation
SET_SCALAR_PROP(BoxTwoPDFM, ImplicitMobilityUpwindWeight, 1.0);

//! The indices required by the isothermal 2pDFM model
SET_PROP(BoxTwoPDFM, Indices)
{ private:
    enum { Formulation = GET_PROP_VALUE(TypeTag, Formulation) };
 public:
    typedef TwoPDFMIndices<TypeTag, Formulation, 0> type;
};


//! The spatial parameters to be employed.
//! Use BoxSpatialParams by default.
SET_TYPE_PROP(BoxTwoPDFM, SpatialParams, BoxSpatialParams<TypeTag>);

/*!
 * \brief Set the property for the material parameters by extracting
 *        it from the material law.
 */
SET_TYPE_PROP(BoxTwoPDFM,
              MaterialLawParams,
              typename GET_PROP_TYPE(TypeTag, MaterialLaw)::Params);

SET_PROP(BoxTwoPDFM, WettingPhase)
{ private:
    typedef typename GET_PROP_TYPE(TypeTag, Scalar) Scalar;
public:
    typedef Ewoms::LiquidPhase<Scalar, Ewoms::NullComponent<Scalar> > type;
};

SET_PROP(BoxTwoPDFM, NonwettingPhase)
{ private:
    typedef typename GET_PROP_TYPE(TypeTag, Scalar) Scalar;
public:
    typedef Ewoms::LiquidPhase<Scalar, Ewoms::NullComponent<Scalar> > type;
};

SET_PROP(BoxTwoPDFM, FluidSystem)
{ private:
    typedef typename GET_PROP_TYPE(TypeTag, Scalar) Scalar;
    typedef typename GET_PROP_TYPE(TypeTag, WettingPhase) WettingPhase;
    typedef typename GET_PROP_TYPE(TypeTag, NonwettingPhase) NonwettingPhase;

public:
    typedef Ewoms::FluidSystems::TwoPImmiscible<Scalar,
                                                WettingPhase,
                                                NonwettingPhase> type;
};

SET_PROP(BoxTwoPDFM, FluidState)
{
private:
    typedef typename GET_PROP_TYPE(TypeTag, Scalar) Scalar;
    typedef typename GET_PROP_TYPE(TypeTag, FluidSystem) FluidSystem;
public:
    typedef ImmiscibleFluidState<Scalar, FluidSystem> type;
};

// disable velocity output by default
SET_BOOL_PROP(BoxTwoPDFM, VtkAddVelocity, false);

// enable gravity by default
SET_BOOL_PROP(BoxTwoPDFM, ProblemEnableGravity, true);
} // end namespace Properties
} // end namespace Ewoms

#endif // EWOMS_BOXMODELS_2PDFM_PROPERTY_DEFAULTS_HH
