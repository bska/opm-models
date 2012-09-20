// -*- mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
// vi: set et ts=4 sw=4 sts=4:
/*****************************************************************************
 *   Copyright (C) 2008-2012 by Andreas Lauser                               *
 *                                                                           *
 *   This program is free software: you can redistribute it and/or modify    *
 *   it under the terms of the GNU General Public License as published by    *
 *   the Free Software Foundation, either version 2 of the License, or       *
 *   (at your option) any later version.                                     *
 *                                                                           *
 *   This program is distributed in the hope that it will be useful,         *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 *   GNU General Public License for more details.                            *
 *                                                                           *
 *   You should have received a copy of the GNU General Public License       *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.   *
 *****************************************************************************/
/*!
 * \file
 *
 * \brief Test for the isothermal compositional model based on flash
 *        calculations.
 */
#include "config.h"

#include "problems/injectionproblem.hh"

#include <dumux/boxmodels/flash/flashmodel.hh>
#include <dumux/common/start.hh>

namespace Dumux {
namespace Properties {
NEW_TYPE_TAG(InjectionFlashProblem, INHERITS_FROM(BoxFlash, InjectionBaseProblem));

// for the flash model we want to use thermodynamic hints or it will
// get _very_ slow.
SET_BOOL_PROP(InjectionFlashProblem, EnableHints, true);

// the flash model has serious problems with the numerical
// precision. if quadruple precision math is available, we use it,
// else we increase the precision of the Newton solver
#if HAVE_QUAD
SET_TYPE_PROP(InjectionFlashProblem, Scalar, quad);
#else
SET_SCALAR_PROP(InjectionFlashProblem, NewtonRelTolerance, 1e-5);
#endif

}
}

int main(int argc, char** argv)
{
    typedef TTAG(InjectionFlashProblem) ProblemTypeTag;
    return Dumux::start<ProblemTypeTag>(argc, argv);
}
