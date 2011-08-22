/*
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2009, Willow Garage, Inc.
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
 * $Id: io.h 1524 2011-07-01 01:01:43Z rusu $
 *
 */

#ifndef PCL_IO_IO_H_
#define PCL_IO_IO_H_

#include <string>
#include <boost/fusion/sequence/intrinsic/at_key.hpp>
#include "pcl/pcl_base.h"
#include "pcl/PointIndices.h"
#include "pcl/ros/conversions.h"
#include <locale>

namespace pcl
{
  /** \brief Get the index of a specified field (i.e., dimension/channel)
    * \param cloud the the point cloud message
    * \param field_name the string defining the field name
    * \ingroup io
    */
  inline int
  getFieldIndex (const sensor_msgs::PointCloud2 &cloud, const std::string &field_name)
  {
    // Get the index we need
    for (size_t d = 0; d < cloud.fields.size (); ++d)
      if (cloud.fields[d].name == field_name)
        return (d);
    return (-1);
  }

  /** \brief Get the index of a specified field (i.e., dimension/channel)
    * \param cloud the the point cloud message
    * \param field_name the string defining the field name
    * \param fields a vector to the original \a PointField vector that the raw PointCloud message contains
    * \ingroup io
    */
  template <typename PointT> inline int 
  getFieldIndex (const pcl::PointCloud<PointT> &cloud, const std::string &field_name, 
                 std::vector<sensor_msgs::PointField> &fields);

  /** \brief Get the list of available fields (i.e., dimension/channel)
    * \param cloud the the point cloud message
    * \param fields a vector to the original \a PointField vector that the raw PointCloud message contains
    * \ingroup io
    */
  template <typename PointT> inline void 
  getFields (const pcl::PointCloud<PointT> &cloud, std::vector<sensor_msgs::PointField> &fields);

  /** \brief Get the list of all fields available in a given cloud
    * \param cloud the the point cloud message
    * \ingroup io
    */
  template <typename PointT> inline std::string 
  getFieldsList (const pcl::PointCloud<PointT> &cloud);

  /** \brief Get the available point cloud fields as a space separated string
    * \param cloud a pointer to the PointCloud message
    * \ingroup io
    */
  inline std::string
  getFieldsList (const sensor_msgs::PointCloud2 &cloud)
  {
    std::string result;
    for (size_t i = 0; i < cloud.fields.size () - 1; ++i)
      result += cloud.fields[i].name + " ";
    result += cloud.fields[cloud.fields.size () - 1].name;
    return (result);
  }

  /** \brief Obtains the size of a specific field data type in bytes
    * \param datatype the field data type (see PointField.h)
    * \ingroup io
    */
  inline int
  getFieldSize (int datatype)
  {
    switch (datatype)
    {
      case sensor_msgs::PointField::INT8:
      case sensor_msgs::PointField::UINT8:
        return (1);

      case sensor_msgs::PointField::INT16:
      case sensor_msgs::PointField::UINT16:
        return (2);

      case sensor_msgs::PointField::INT32:
      case sensor_msgs::PointField::UINT32:
      case sensor_msgs::PointField::FLOAT32:
        return (4);

      case sensor_msgs::PointField::FLOAT64:
        return (8);

      default:
        return (0);
    }
  }

  /** \brief Obtains the type of the PointField from a specific size and type
    * \param size the size in bytes of the data field
    * \param type a char describing the type of the field  ('F' = float, 'I' = signed, 'U' = unsigned)
    * \ingroup io
    */
  inline int
  getFieldType (int size, char type)
  {
    type = std::toupper (type, std::locale::classic ());
    switch (size)
    {
      case 1:
        if (type == 'I')
          return (sensor_msgs::PointField::INT8);
        if (type == 'U')
          return (sensor_msgs::PointField::UINT8);

      case 2:
        if (type == 'I')
          return (sensor_msgs::PointField::INT16);
        if (type == 'U')
          return (sensor_msgs::PointField::UINT16);

      case 4:
        if (type == 'I')
          return (sensor_msgs::PointField::INT32);
        if (type == 'U')
          return (sensor_msgs::PointField::UINT32);
        if (type == 'F')
          return (sensor_msgs::PointField::FLOAT32);

      case 8:
        return (sensor_msgs::PointField::FLOAT64);

      default:
        return (-1);
    }
  }

  /** \brief Obtains the type of the PointField from a specific PointField as a char
    * \param type the PointField field type
    * \ingroup io
    */
  inline char
  getFieldType (int type)
  {
    switch (type)
    {
      case sensor_msgs::PointField::INT8:
      case sensor_msgs::PointField::INT16:
      case sensor_msgs::PointField::INT32:
        return ('I');

      case sensor_msgs::PointField::UINT8:
      case sensor_msgs::PointField::UINT16:
      case sensor_msgs::PointField::UINT32:
        return ('U');

      case sensor_msgs::PointField::FLOAT32:
      case sensor_msgs::PointField::FLOAT64:
        return ('F');
      default:
        return ('?');
    }
  }

  /** \brief Copy all the fields from a given point cloud into a new point cloud
    * \param cloud_in the input point cloud dataset
    * \param cloud_out the resultant output point cloud dataset
    * \ingroup io
    */
  template <typename PointInT, typename PointOutT> void 
  copyPointCloud (const pcl::PointCloud<PointInT> &cloud_in, 
                  pcl::PointCloud<PointOutT> &cloud_out);

  /** \brief Concatenate two sensor_msgs::PointCloud2. Returns true if successful, false if failed (e.g., name/number 
    * of fields differs)
    * \param cloud1 the first input point cloud dataset
    * \param cloud2 the second input point cloud dataset
    * \param cloud_out the resultant output point cloud dataset
    * \ingroup io
    */
  PCL_EXPORTS bool 
  concatenatePointCloud (const sensor_msgs::PointCloud2 &cloud1, 
                         const sensor_msgs::PointCloud2 &cloud2, 
                         sensor_msgs::PointCloud2 &cloud_out);

  /** \brief Extract the indices of a given point cloud as a new point cloud
    * \param cloud_in the input point cloud dataset
    * \param indices the vector of indices representing the points to be copied from \a cloud_in
    * \param cloud_out the resultant output point cloud dataset
    * \ingroup io
    */
  PCL_EXPORTS void 
  copyPointCloud (const sensor_msgs::PointCloud2 &cloud_in, 
                  const std::vector<int> &indices, 
                  sensor_msgs::PointCloud2 &cloud_out);

  /** \brief Extract the indices of a given point cloud as a new point cloud
    * \param cloud_in the input point cloud dataset
    * \param indices the vector of indices representing the points to be copied from \a cloud_in
    * \param cloud_out the resultant output point cloud dataset
    * \ingroup io
    */
  template <typename PointT> void 
  copyPointCloud (const pcl::PointCloud<PointT> &cloud_in, 
                  const std::vector<int> &indices, 
                  pcl::PointCloud<PointT> &cloud_out);

  /** \brief Extract the indices of a given point cloud as a new point cloud
    * \param cloud_in the input point cloud dataset
    * \param indices the vector of indices representing the points to be copied from \a cloud_in
    * \param cloud_out the resultant output point cloud dataset
    * \ingroup io
    */
  template <typename PointInT, typename PointOutT> void 
  copyPointCloud (const pcl::PointCloud<PointInT> &cloud_in, 
                  const std::vector<int> &indices, 
                  pcl::PointCloud<PointOutT> &cloud_out);

  /** \brief Extract the indices of a given point cloud as a new point cloud
    * \param cloud_in the input point cloud dataset
    * \param indices the PointIndices structure representing the points to be copied from \a cloud_in
    * \param cloud_out the resultant output point cloud dataset
    * \ingroup io
    */
  template <typename PointT> void 
  copyPointCloud (const pcl::PointCloud<PointT> &cloud_in, 
                  const PointIndices &indices, 
                  pcl::PointCloud<PointT> &cloud_out);

  /** \brief Extract the indices of a given point cloud as a new point cloud
    * \param cloud_in the input point cloud dataset
    * \param indices the PointIndices structure representing the points to be copied from \a cloud_in
    * \param cloud_out the resultant output point cloud dataset
    * \ingroup io
    */
  template <typename PointInT, typename PointOutT> void 
  copyPointCloud (const pcl::PointCloud<PointInT> &cloud_in, 
                  const PointIndices &indices, 
                  pcl::PointCloud<PointOutT> &cloud_out);

  /** \brief Extract the indices of a given point cloud as a new point cloud
    * \param cloud_in the input point cloud dataset
    * \param indices the vector of indices representing the points to be copied from \a cloud_in
    * \param cloud_out the resultant output point cloud dataset
    * \ingroup io
    */
  template <typename PointT> void 
  copyPointCloud (const pcl::PointCloud<PointT> &cloud_in, 
                  const std::vector<pcl::PointIndices> &indices, 
                  pcl::PointCloud<PointT> &cloud_out);

  /** \brief Extract the indices of a given point cloud as a new point cloud
    * \param cloud_in the input point cloud dataset
    * \param indices the vector of indices representing the points to be copied from \a cloud_in
    * \param cloud_out the resultant output point cloud dataset
    * \ingroup io
    */
  template <typename PointInT, typename PointOutT> void 
  copyPointCloud (const pcl::PointCloud<PointInT> &cloud_in, 
                  const std::vector<pcl::PointIndices> &indices, 
                  pcl::PointCloud<PointOutT> &cloud_out);

  /** \brief Concatenate two datasets representing different fields.
    * \param cloud1_in the first input dataset
    * \param cloud2_in the second input dataset
    * \param cloud_out the resultant output dataset created by the concatenation of all the fields in the input datasets
    * \ingroup io
    */
  template <typename PointIn1T, typename PointIn2T, typename PointOutT> void 
  concatenateFields (const pcl::PointCloud<PointIn1T> &cloud1_in, 
                     const pcl::PointCloud<PointIn2T> &cloud2_in, 
                     pcl::PointCloud<PointOutT> &cloud_out);

  /** \brief Copy the XYZ dimensions of a sensor_msgs::PointCloud2 into Eigen format
    * \param in the point cloud message
    * \param out the resultant Eigen MatrixXf format containing XYZ0 / point
    * \ingroup io
    */
  PCL_EXPORTS bool 
  getPointCloudAsEigen (const sensor_msgs::PointCloud2 &in, Eigen::MatrixXf &out);

  /** \brief Copy the XYZ dimensions from an Eigen MatrixXf into a sensor_msgs::PointCloud2 message
    * \param in the Eigen MatrixXf format containing XYZ0 / point
    * \param out the resultant point cloud message
    * \note the method assumes that the PointCloud2 message already has the fields set up properly !
    * \ingroup io
    */
  PCL_EXPORTS bool 
  getEigenAsPointCloud (Eigen::MatrixXf &in, sensor_msgs::PointCloud2 &out);
  
  namespace io 
  {
    /** \brief swap bytes order of a char array of length N
      * \param bytes char array to swap
      * \ingroup io
      */
    template <std::size_t N>
    void swapByte(char* bytes);

   /** \brief specialization of swapByte for dimension 1
     * \param bytes char array to swap
     **/
    template <>
    inline void swapByte<1>(char* bytes) {}

  
   /** \brief specialization of swapByte for dimension 2
     * \param bytes char array to swap
     **/
    template <>
    inline void swapByte<2>(char* bytes) { std::swap(bytes[0], bytes[1]); }
  
   /** \brief specialization of swapByte for dimension 4
     * \param bytes char array to swap
     **/
    template <> 
    inline void swapByte<4>(char* bytes)
    {
      std::swap(bytes[0], bytes[3]);
      std::swap(bytes[1], bytes[2]);
    }
  
   /** \brief specialization of swapByte for dimension 8
     * \param bytes char array to swap
     **/
    template <>
    inline void swapByte<8>(char* bytes)
    {
      std::swap(bytes[0], bytes[7]);
      std::swap(bytes[1], bytes[6]);
      std::swap(bytes[2], bytes[5]);
      std::swap(bytes[3], bytes[4]);
    }
  
    /** \brief swaps byte of an arbitrary type T casting it to char*
      * \param value the data you want its bytes swapped
      */
    template <typename T>
    void swapByte(T& value)
    {
      pcl::io::swapByte<sizeof(T)>(reinterpret_cast<char*>(&value));
    }
  }
}

#include "pcl/io/impl/io.hpp"

#endif  //#ifndef PCL_IO_IO_H_
