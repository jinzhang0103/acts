// This file is part of the Acts project.
//
// Copyright (C) 2018 Acts project team
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

///  Boost include(s)
#define BOOST_TEST_MODULE SurfaceProvider Tests

#include <boost/test/included/unit_test.hpp>
// leave blank line

#include <boost/test/data/test_case.hpp>
// leave blank line

#include <boost/test/output_test_stream.hpp>
// leave blank line

#include <memory>
#include "Acts/Extrapolator/Navigator.hpp"
#include "Acts/Extrapolator/SurfaceProvider.hpp"
#include "Acts/Surfaces/PlaneSurface.hpp"

namespace bdata = boost::unit_test::data;
namespace tt    = boost::test_tools;

namespace Acts {

namespace Test {

  struct PropagatorState
  {
    Navigator::State navigation;
  };

  BOOST_AUTO_TEST_CASE(SurfaceProviderTest)
  {
    PlaneSurface psf6(Vector3D(6., 0., 0.), Vector3D(1., 0., 0.));
    PlaneSurface psf7(Vector3D(7., 0., 0.), Vector3D(1., 0., 0.));
    PlaneSurface psf8(Vector3D(8., 0., 0.), Vector3D(1., 0., 0.));
    PlaneSurface psf9(Vector3D(9., 0., 0.), Vector3D(1., 0., 0.));
    PlaneSurface psf10(Vector3D(10., 0., 0.), Vector3D(1., 0., 0.));

    // The surface Provider and its result
    SurfaceProvider              sProvider;
    SurfaceProvider::result_type sProviderResult;

    // The Propagator state
    PropagatorState pState;
  }

}  // namespace Acts
}  // namespace Test