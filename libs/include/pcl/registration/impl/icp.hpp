/*
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2010, Willow Garage, Inc.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of Willow Garage, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 * $Id: icp.hpp 1370 2011-06-19 01:06:01Z jspricke $
 *
 */

#include <boost/unordered_map.hpp>

template <typename PointSource, typename PointTarget> void
pcl::IterativeClosestPoint<PointSource, PointTarget>::computeTransformation (PointCloudSource &output)
{
  pcl::IterativeClosestPoint<PointSource, PointTarget>::computeTransformation (output, Eigen::Matrix4f::Identity());
}

template <typename PointSource, typename PointTarget> void
pcl::IterativeClosestPoint<PointSource, PointTarget>::computeTransformation (PointCloudSource &output, const Eigen::Matrix4f &guess)
{
  // Allocate enough space to hold the results
  std::vector<int> nn_indices (1);
  std::vector<float> nn_dists (1);

  // Point cloud containing the correspondences of each point in <input, indices>
  PointCloudTarget input_corresp;
  input_corresp.points.resize (indices_->size ());

  nr_iterations_ = 0;
  converged_ = false;
  double dist_threshold = corr_dist_threshold_ * corr_dist_threshold_;

  // If the guessed transformation is non identity
  if(guess != Eigen::Matrix4f::Identity())
  {
    // Initialise final transformation to the guessed one
    final_transformation_ = guess;
    // Apply guessed transformation prior to search for neighbours
    transformPointCloud (output, output, guess);
  }

  while (!converged_)           // repeat until convergence
  {
    // Save the previously estimated transformation
    previous_transformation_ = transformation_;

    int cnt = 0;
    std::vector<int> source_indices (indices_->size ());
    std::vector<int> target_indices (indices_->size ());

    // Iterating over the entire index vector and  find all correspondences
    for (size_t idx = 0; idx < indices_->size (); ++idx)
    {
      if (!searchForNeighbors (output, idx, nn_indices, nn_dists))
      {
        PCL_ERROR ("[pcl::%s::computeTransformation] Unable to find a nearest neighbor in the target dataset for point %d in the source!\n", getClassName ().c_str (), (*indices_)[idx]);
        return;
      }

      // Check if the distance to the nearest neighbor is smaller than the user imposed threshold
      if (nn_dists[0] < dist_threshold)
      {
        source_indices[cnt] = idx;
        target_indices[cnt] = nn_indices[0];
        cnt++;
      }
    }
    // Resize to the actual number of valid correspondences
    source_indices.resize (cnt); target_indices.resize (cnt);

    std::vector<int> source_indices_good;
    std::vector<int> target_indices_good;
    {
      // From the set of correspondences found, attempt to remove outliers
      // Create the registration model
      typedef typename SampleConsensusModelRegistration<PointSource>::Ptr SampleConsensusModelRegistrationPtr;
      SampleConsensusModelRegistrationPtr model;
      model.reset (new SampleConsensusModelRegistration<PointSource> (output.makeShared (), source_indices));
      // Pass the target_indices
      model->setInputTarget (target_, target_indices);
      // Create a RANSAC model
      RandomSampleConsensus<PointSource> sac (model, inlier_threshold_);
      sac.setMaxIterations (1000);

      // Compute the set of inliers
      if (!sac.computeModel ())
      {
        source_indices_good = source_indices;
        target_indices_good = target_indices;
      }
      else
      {
        std::vector<int> inliers;
        // Get the inliers
        sac.getInliers (inliers);
        source_indices_good.resize (inliers.size ());
        target_indices_good.resize (inliers.size ());

        boost::unordered_map<int, int> source_to_target;
        for (unsigned int i = 0; i < source_indices.size(); ++i)
          source_to_target[source_indices[i]] = target_indices[i];

        // Copy just the inliers
        std::copy(inliers.begin(), inliers.end(), source_indices_good.begin());
        for (size_t i = 0; i < inliers.size (); ++i)
          target_indices_good[i] = source_to_target[inliers[i]];
      }
    }

    // Check whether we have enough correspondences
    cnt = (int)source_indices_good.size ();
    if (cnt < min_number_correspondences_)
    {
      PCL_ERROR ("[pcl::%s::computeTransformation] Not enough correspondences found. Relax your threshold parameters.\n", getClassName ().c_str ());
      converged_ = false;
      return;
    }

    PCL_DEBUG ("[pcl::%s::computeTransformation] Number of correspondences %d [%f%%] out of %lu points [100.0%%], RANSAC rejected: %lu [%f%%].\n", getClassName ().c_str (), cnt, (cnt * 100.0) / indices_->size (), (unsigned long)indices_->size (), (unsigned long)source_indices.size () - cnt, (source_indices.size () - cnt) * 100.0 / source_indices.size ());
  
    // Estimate the transform
    rigid_transformation_estimation_(output, source_indices_good, *target_, target_indices_good, transformation_);

    // Tranform the data
    transformPointCloud (output, output, transformation_);

    // Obtain the final transformation    
    final_transformation_ = transformation_ * final_transformation_;

    nr_iterations_++;
    // Check for convergence
    if (nr_iterations_ >= max_iterations_ ||
        fabs ((transformation_ - previous_transformation_).sum ()) < transformation_epsilon_)
    {
      converged_ = true;
      PCL_DEBUG ("[pcl::%s::computeTransformation] Convergence reached. Number of iterations: %d out of %d. Transformation difference: %f\n",
                 getClassName ().c_str (), nr_iterations_, max_iterations_, fabs ((transformation_ - previous_transformation_).sum ()));
    }
  }
}

