// -*- mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
// vi: set et ts=4 sw=4 sts=4:
/*
  This file is part of the Open Porous Media project (OPM).

  OPM is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 2 of the License, or
  (at your option) any later version.

  OPM is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with OPM.  If not, see <http://www.gnu.org/licenses/>.

  Consult the COPYING file in the top-level source directory of this
  module for the precise wording of the license and the list of
  copyright holders.
*/
/*!
 * \file
 * \brief Provides convenience routines to bring up the simulation at runtime.
 */
#ifndef EWOMS_START_HH
#define EWOMS_START_HH

#include <opm/models/utils/propertysystem.hh>
// the following header is not required here, but it must be included before
// dune/common/densematrix.hh because of some c++ ideosyncrasies
#include <opm/material/densead/Evaluation.hpp>

#include "parametersystem.hh"

#include <opm/models/utils/simulator.hh>
#include <opm/models/utils/timer.hh>

#include <opm/material/common/Valgrind.hpp>

#include <opm/material/common/ResetLocale.hpp>

#include <dune/grid/io/file/dgfparser/dgfparser.hh>
#include <dune/common/version.hh>
#include <dune/common/parametertreeparser.hh>
#include <dune/common/parallel/mpihelper.hh>

#if HAVE_DUNE_FEM
#include <dune/fem/misc/mpimanager.hh>
#endif

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <locale>

#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <string.h>

#if HAVE_MPI
#include <mpi.h>
#endif

//! \cond SKIP_THIS

namespace Opm {
/*!
 * \brief Announce all runtime parameters to the registry but do not specify them yet.
 */
template <class TypeTag>
static inline void registerAllParameters_(bool finalizeRegistration = true)
{
    using Simulator = GetPropType<TypeTag, Properties::Simulator>;
    using ThreadManager = GetPropType<TypeTag, Properties::ThreadManager>;

    Parameters::Register<Parameters::ParameterFile>
        ("An .ini file which contains a set of run-time parameters");
    Parameters::Register<Parameters::PrintParameters>
        ("Print the values of the run-time parameters at the "
         "start of the simulation");

    ThreadManager::registerParameters();
    Simulator::registerParameters();

    if (finalizeRegistration) {
        Parameters::endRegistration();
    }
}

/*!
 * \brief Register all runtime parameters, parse the command line
 *        arguments and the parameter file.
 *
 * \param argc The number of command line arguments
 * \param argv Array with the command line argument strings
 * \return A negative value if --help or --print-properties was provided,
 *         a positive value for errors or 0 for success.
 */
template <class TypeTag>
static inline int setupParameters_(int argc,
                                   const char **argv,
                                   bool registerParams=true,
                                   bool allowUnused=false,
                                   bool handleHelp = true)
{
    using Problem = GetPropType<TypeTag, Properties::Problem>;

    // first, get the MPI rank of the current process
    int myRank = 0;

    ////////////////////////////////////////////////////////////
    // Register all parameters
    ////////////////////////////////////////////////////////////
    if (registerParams)
        registerAllParameters_<TypeTag>();

    ////////////////////////////////////////////////////////////
    // set the parameter values
    ////////////////////////////////////////////////////////////

    // fill the parameter tree with the options from the command line
    const auto& positionalParamCallback = Problem::handlePositionalParameter;
    std::string helpPreamble = ""; // print help if non-empty!
    if (myRank == 0 && handleHelp)
        helpPreamble = Problem::helpPreamble(argc, argv);
    std::string s =
        Parameters::parseCommandLineOptions(argc,
                                            argv,
                                            helpPreamble,
                                            positionalParamCallback);
    if (!s.empty())
    {
        int status = 1;
        if (s == "Help called") // only on master process
            status = -1; // Use negative values to indicate --help argument
#if HAVE_MPI
        // Force -1 if the master process has that.
        int globalStatus;
        MPI_Allreduce(&status, &globalStatus, 1, MPI_INT, MPI_MIN, MPI_COMM_WORLD);
        return globalStatus;
#endif
        return status;
    }

    const std::string paramFileName = Parameters::Get<Parameters::ParameterFile>(false);
    if (!paramFileName.empty()) {
        ////////////////////////////////////////////////////////////
        // add the parameters specified using an .ini file
        ////////////////////////////////////////////////////////////

        // check whether the parameter file is readable.
        std::ifstream tmp;
        tmp.open(paramFileName.c_str());
        if (!tmp.is_open()) {
            std::ostringstream oss;
            if (myRank == 0) {
                oss << "Parameter file \"" << paramFileName
                    << "\" does not exist or is not readable.";
                Parameters::printUsage(argv[0], oss.str());
            }
            return /*status=*/1;
        }

        // read the parameter file.
        Parameters::parseParameterFile(paramFileName, /*overwrite=*/false);
    }

    // make sure that no unknown parameters are encountered
    using KeyValuePair = std::pair<std::string, std::string>;
    using ParamList = std::list<KeyValuePair>;

    ParamList usedParams;
    ParamList unusedParams;

    Parameters::getLists(usedParams, unusedParams);
    if (!allowUnused && !unusedParams.empty()) {
        if (myRank == 0) {
            if (unusedParams.size() == 1)
                std::cerr << "The following explicitly specified parameter is unknown:\n";
            else
                std::cerr << "The following " << unusedParams.size()
                          << " explicitly specified parameters are unknown:\n";

            std::cerr << "\n";
            for (const auto& keyValue : unusedParams)
                std::cerr << "   " << keyValue.first << "=\"" << keyValue.second << "\"\n";
            std::cerr << "\n";

            std::cerr << "Use\n"
                      << "\n"
                      << "  " << argv[0] << " --help\n"
                      << "\n"
                      <<"to obtain the list of recognized command line parameters.\n\n";
        }
        return /*status=*/1;
    }

    return /*status=*/0;
}

/*!
 * \brief Resets the current TTY to a usable state if the program was aborted.
 *
 * This is intended to be called as part of a generic exception handler
 */
static inline void resetTerminal_()
{
    // make sure stderr and stderr do not contain any unwritten data and make sure that
    // the TTY does not see any unfinished ANSI escape sequence.
    std::cerr << "    \r\n";
    std::cerr.flush();
    std::cout << "    \r\n";
    std::cout.flush();

    // it seems like some terminals sometimes takes their time to react, so let's
    // accommodate them.
    usleep(/*usec=*/500*1000);

    // this requires the 'stty' command to be available in the command search path. on
    // most linux systems, is the case. (but even if the system() function fails, the
    // worst thing which can happen is that the TTY stays potentially choked up...)
    if (system("stty sane") != 0)
        std::cout << "Executing the 'stty' command failed."
                  << " Terminal might be left in an undefined state!\n";
}

/*!
 * \brief Resets the current TTY to a usable state if the program was interrupted by
 *        SIGABRT or SIGINT.
 */
static inline void resetTerminal_(int signum)
{
    // first thing to do when a nuke hits: restore the default signal handler
    signal(signum, SIG_DFL);

#if HAVE_MPI
    int rank = 0;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    if (rank != 0) {
        // re-raise the signal
        raise(signum);

        return;
    }
#endif

    if (isatty(fileno(stdout)) && isatty(fileno(stdin))) {
        std::cout << "\n\nReceived signal " << signum
                  << " (\"" << strsignal(signum) << "\")."
                  << " Trying to reset the terminal.\n";

        resetTerminal_();
    }

    // after we did our best to clean the pedestrian way, re-raise the signal
    raise(signum);
}
//! \endcond

/*!
 * \ingroup Common
 *
 * \brief Provides a main function which reads in parameters from the
 *        command line and a parameter file and runs the simulation
 *
 * \tparam TypeTag  The type tag of the problem which needs to be solved
 *
 * \param argc The number of command line arguments
 * \param argv The array of the command line arguments
 */
template <class TypeTag>
static inline int start(int argc, char **argv,  bool registerParams=true)
{
    using Scalar = GetPropType<TypeTag, Properties::Scalar>;
    using Simulator = GetPropType<TypeTag, Properties::Simulator>;
    using Problem = GetPropType<TypeTag, Properties::Problem>;
    using ThreadManager = GetPropType<TypeTag, Properties::ThreadManager>;

    // set the signal handlers to reset the TTY to a well defined state on unexpected
    // program aborts
    if (isatty(STDIN_FILENO)) {
        signal(SIGINT, resetTerminal_);
        signal(SIGHUP, resetTerminal_);
        signal(SIGABRT, resetTerminal_);
        signal(SIGFPE, resetTerminal_);
        signal(SIGSEGV, resetTerminal_);
        signal(SIGPIPE, resetTerminal_);
        signal(SIGTERM, resetTerminal_);
    }

    resetLocale();

    int myRank = 0;
    try
    {
        int paramStatus = setupParameters_<TypeTag>(argc, const_cast<const char**>(argv), registerParams);
        if (paramStatus == 1)
            return 1;
        if (paramStatus == 2)
            return 0;

        ThreadManager::init();

        // initialize MPI, finalize is done automatically on exit
#if HAVE_DUNE_FEM
        Dune::Fem::MPIManager::initialize(argc, argv);
        myRank = Dune::Fem::MPIManager::rank();
#else
        myRank = Dune::MPIHelper::instance(argc, argv).rank();
#endif

        // read the initial time step and the end time
        Scalar endTime = Parameters::Get<Parameters::EndTime<Scalar>>();
        if (endTime < -1e50) {
            if (myRank == 0)
                Parameters::printUsage(argv[0],
                                       "Mandatory parameter '--end-time' not specified!");
            return 1;
        }

        Scalar initialTimeStepSize = Parameters::Get<Parameters::InitialTimeStepSize<Scalar>>();
        if (initialTimeStepSize < -1e50) {
            if (myRank == 0)
                Parameters::printUsage(argv[0],
                                       "Mandatory parameter '--initial-time-step-size' "
                                       "not specified!");
            return 1;
        }

        if (myRank == 0) {
#ifdef EWOMS_VERSION
            std::string versionString = EWOMS_VERSION;
#else
            std::string versionString = "";
#endif
            const std::string briefDescription = Problem::briefDescription();
            if (!briefDescription.empty()) {
                std::string tmp = Parameters::breakLines_(briefDescription,
                                                          /*indentWidth=*/0,
                                                          Parameters::getTtyWidth_());
                std::cout << tmp << std::endl << std::endl;
            }
            else
                std::cout << "opm models " << versionString
                          << " will now start the simulation. " << std::endl;
        }

        // print the parameters if requested
        int printParams = Parameters::Get<Parameters::PrintParameters>();
        if (myRank == 0) {
            std::string endParametersSeparator("# [end of parameters]\n");
            if (printParams) {
                bool printSeparator = false;
                if (printParams == 1 || !isatty(fileno(stdout))) {
                    Parameters::printValues();
                    printSeparator = true;
                }
                else
                    // always print the list of specified but unused parameters
                    printSeparator =
                        printSeparator ||
                        Parameters::printUnused();
                if (printSeparator)
                    std::cout << endParametersSeparator;
            }
            else
                // always print the list of specified but unused parameters
                if (Parameters::printUnused())
                    std::cout << endParametersSeparator;
        }

        // instantiate and run the concrete problem. make sure to
        // deallocate the problem and before the time manager and the
        // grid
        Simulator simulator;
        simulator.run();

        if (myRank == 0) {
            std::cout << "Simulation completed" << std::endl;                                 
        }
        return 0;
    }
    catch (std::exception& e)
    {
        if (myRank == 0) {
            std::cout << e.what() << ". Abort!\n" << std::flush;

            std::cout << "Trying to reset TTY.\n";
            resetTerminal_();
        }

        return 1;
    }
    catch (...)
    {
        if (myRank == 0) {
            std::cout << "Unknown exception thrown!\n" << std::flush;

            std::cout << "Trying to reset TTY.\n";
            resetTerminal_();
        }

        return 3;
    }
}

} // namespace Opm

#endif
