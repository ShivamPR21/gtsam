/* ----------------------------------------------------------------------------

 * GTSAM Copyright 2010, Georgia Tech Research Corporation, 
 * Atlanta, Georgia 30332-0415
 * All Rights Reserved
 * Authors: Frank Dellaert, et al. (see THANKS for the full author list)

 * See LICENSE for the license information

 * -------------------------------------------------------------------------- */

/**
 *  @file  pose3SLAM.cpp
 *  @brief: bearing/range measurements in 2D plane
 *  @author Frank Dellaert
 **/

#include <gtsam/slam/pose3SLAM.h>
#include <gtsam/nonlinear/NonlinearFactorGraph.h>

// Use pose3SLAM namespace for specific SLAM instance
namespace pose3SLAM {

  /* ************************************************************************* */
  Values Values::Circle(size_t n, double radius) {
    Values x;
    double theta = 0, dtheta = 2 * M_PI / n;
    // We use aerospace/navlab convention, X forward, Y right, Z down
    // First pose will be at (R,0,0)
    // ^y   ^ X
    // |    |
    // z-->xZ--> Y  (z pointing towards viewer, Z pointing away from viewer)
    // Vehicle at p0 is looking towards y axis (X-axis points towards world y)
    Rot3 gR0(Point3(0, 1, 0), Point3(1, 0, 0), Point3(0, 0, -1));
    for (size_t i = 0; i < n; i++, theta += dtheta) {
      Point3 gti(radius*cos(theta), radius*sin(theta), 0);
      Rot3 _0Ri = Rot3::yaw(-theta); // negative yaw goes counterclockwise, with Z down !
      Pose3 gTi(gR0 * _0Ri, gti);
      x.insert(i, gTi);
    }
    return x;
  }

  /* ************************************************************************* */
  Matrix Values::translations() const {
    size_t j=0;
    ConstFiltered<Pose3> poses = filter<Pose3>();
    Matrix result(poses.size(),3);
    BOOST_FOREACH(const ConstFiltered<Pose3>::KeyValuePair& keyValue, poses)
      result.row(j++) = keyValue.value.translation().vector();
    return result;
  }

  /* ************************************************************************* */
  void Graph::addPoseConstraint(Key i, const Pose3& p) {
    sharedFactor factor(new NonlinearEquality<Pose3>(i, p));
    push_back(factor);
  }

  /* ************************************************************************* */
  void Graph::addPosePrior(Key i, const Pose3& p, const SharedNoiseModel& model) {
    sharedFactor factor(new PriorFactor<Pose3>(i, p, model));
    push_back(factor);
  }

  /* ************************************************************************* */
  void Graph::addRelativePose(Key i1, Key i2, const Pose3& z, const SharedNoiseModel& model) {
    push_back(boost::make_shared<BetweenFactor<Pose3> >(i1, i2, z, model));
  }

  /* ************************************************************************* */

} // pose3SLAM
