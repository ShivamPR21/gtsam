/* ----------------------------------------------------------------------------

 * GTSAM Copyright 2010, Georgia Tech Research Corporation,
 * Atlanta, Georgia 30332-0415
 * All Rights Reserved
 * Authors: Frank Dellaert, et al. (see THANKS for the full author list)

 * See LICENSE for the license information

 * -------------------------------------------------------------------------- */

/**
 * @file    testMagFactor.cpp
 * @brief   Unit test for MagFactor
 * @author  Frank Dellaert
 * @date   January 29, 2014
 */

#include <gtsam/navigation/MagFactor.h>
#include <gtsam/base/Testable.h>
#include <gtsam/base/numericalDerivative.h>

#include <CppUnitLite/TestHarness.h>

#include <GeographicLib/LocalCartesian.hpp>

using namespace std;
using namespace gtsam;
using namespace GeographicLib;

// *************************************************************************
// Convert from Mag to ENU
// ENU Origin is where the plane was in hold next to runway
// const double lat0 = 33.86998, lon0 = -84.30626, h0 = 274;

// Get field from http://www.ngdc.noaa.gov/geomag-web/#igrfwmm
// Declination = -4.94 degrees (West), Inclination = 62.78 degrees Down
// As NED vector, in nT:
Vector3 nM(22653.29982, -1956.83010, 44202.47862);
// Let's assume scale factor,
double scale = 255.0 / 50000.0;
// ...ground truth orientation,
Rot3 nRb = Rot3::yaw(-0.1);
Rot2 theta = -nRb.yaw();
// ...and bias
Vector3 bias(10, -10, 50);
// ... then we measure
Vector3 scaled = scale * nM;
Vector3 measured = scale * nRb.transpose() * nM + bias;

LieScalar s(scale * nM.norm());
Sphere2 dir(nM[0], nM[1], nM[2]);

SharedNoiseModel model = noiseModel::Isotropic::Sigma(3, 0.25);

using boost::none;

// *************************************************************************
TEST( MagFactor, unrotate ) {
  Matrix H;
  Sphere2 expected(0.457383, 0.00632703, 0.889247);
  EXPECT( assert_equal(expected, MagFactor::unrotate(theta,dir,H),1e-5));
  EXPECT( assert_equal(numericalDerivative11<Sphere2,Rot2> //
      (boost::bind(&MagFactor::unrotate, _1, dir, none), theta), H, 1e-7));
}

// *************************************************************************
TEST( MagFactor, Factors ) {

  Matrix H1, H2, H3;

  // MagFactor
  MagFactor f(1, measured, s, dir, bias, model);
  EXPECT( assert_equal(zero(3),f.evaluateError(theta,H1),1e-5));
  EXPECT( assert_equal(numericalDerivative11<Rot2> //
      (boost::bind(&MagFactor::evaluateError, &f, _1, none), theta), H1, 1e-7));

// MagFactor1
  MagFactor1 f1(1, measured, s, dir, bias, model);
  EXPECT( assert_equal(zero(3),f1.evaluateError(nRb,H1),1e-5));
  EXPECT( assert_equal(numericalDerivative11<Rot3> //
      (boost::bind(&MagFactor1::evaluateError, &f1, _1, none), nRb), H1, 1e-7));

// MagFactor2
  MagFactor2 f2(1, 2, measured, nRb, model);
  EXPECT( assert_equal(zero(3),f2.evaluateError(scaled,bias,H1,H2),1e-5));
  EXPECT( assert_equal(numericalDerivative11<LieVector> //
      (boost::bind(&MagFactor2::evaluateError, &f2, _1, bias, none, none), scaled),//
      H1, 1e-7));
  EXPECT( assert_equal(numericalDerivative11<LieVector> //
      (boost::bind(&MagFactor2::evaluateError, &f2, scaled, _1, none, none), bias),//
      H2, 1e-7));

// MagFactor2
  MagFactor3 f3(1, 2, 3, measured, nRb, model);
  EXPECT(assert_equal(zero(3),f3.evaluateError(s,dir,bias,H1,H2,H3),1e-5));
  EXPECT(assert_equal(numericalDerivative11<LieScalar> //
      (boost::bind(&MagFactor3::evaluateError, &f3, _1, dir, bias, none, none, none), s),//
      H1, 1e-7));
  EXPECT(assert_equal(numericalDerivative11<Sphere2> //
      (boost::bind(&MagFactor3::evaluateError, &f3, s, _1, bias, none, none, none), dir),//
      H2, 1e-7));
  EXPECT(assert_equal(numericalDerivative11<LieVector> //
      (boost::bind(&MagFactor3::evaluateError, &f3, s, dir, _1, none, none, none), bias),//
      H3, 1e-7));
}

// *************************************************************************
int main() {
  TestResult tr;
  return TestRegistry::runAllTests(tr);
}
// *************************************************************************
