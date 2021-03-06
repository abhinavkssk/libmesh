// The libMesh Finite Element Library.
// Copyright (C) 2002-2014 Benjamin S. Kirk, John W. Peterson, Roy H. Stogner

// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.

// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

// C++ includes
#include <algorithm> // for std::sort

// Local includes
#include "libmesh/centroid_partitioner.h"
#include "libmesh/mesh_base.h"
#include "libmesh/elem.h"

namespace libMesh
{


//---------------------------------------------------------
// CentroidPartitioner methods
void CentroidPartitioner::_do_partition (MeshBase& mesh,
                                         const unsigned int n)
{
  // Check for an easy return
  if (n == 1)
    {
      this->single_partition (mesh);
      return;
    }


  // Possibly reconstruct centroids
  if (mesh.n_elem() != _elem_centroids.size())
    this->compute_centroids (mesh);



  switch (this->sort_method())
    {
    case X:
      {
        std::sort(_elem_centroids.begin(),
                  _elem_centroids.end(),
                  CentroidPartitioner::sort_x);

        break;
      }


    case Y:
      {
        std::sort(_elem_centroids.begin(),
                  _elem_centroids.end(),
                  CentroidPartitioner::sort_y);

        break;

      }


    case Z:
      {
        std::sort(_elem_centroids.begin(),
                  _elem_centroids.end(),
                  CentroidPartitioner::sort_z);

        break;
      }


    case RADIAL:
      {
        std::sort(_elem_centroids.begin(),
                  _elem_centroids.end(),
                  CentroidPartitioner::sort_radial);

        break;
      }
    default:
      libmesh_error();
    }


  // Make sure the user has not handed us an
  // invalid number of partitions.
  libmesh_assert_greater (n, 0);

  // the number of elements, e.g. 1000
  const dof_id_type n_elem      = mesh.n_elem();
  // the number of elements per processor, e.g 400
  const dof_id_type target_size = n_elem / n;

  // Make sure the mesh hasn't changed since the
  // last time we computed the centroids.
  libmesh_assert_equal_to (mesh.n_elem(), _elem_centroids.size());

  for (dof_id_type i=0; i<n_elem; i++)
    {
      Elem* elem = _elem_centroids[i].second;

      elem->processor_id() =
        std::min (libmesh_cast_int<processor_id_type>(i / target_size),
                  libmesh_cast_int<processor_id_type>(n-1));
    }
}








void CentroidPartitioner::compute_centroids (MeshBase& mesh)
{
  _elem_centroids.clear();
  _elem_centroids.reserve(mesh.n_elem());

  //   elem_iterator it(mesh.elements_begin());
  //   const elem_iterator it_end(mesh.elements_end());

  MeshBase::element_iterator       it     = mesh.elements_begin();
  const MeshBase::element_iterator it_end = mesh.elements_end();

  for (; it != it_end; ++it)
    {
      Elem* elem = *it;

      _elem_centroids.push_back(std::make_pair(elem->centroid(), elem));
    }
}




bool CentroidPartitioner::sort_x (const std::pair<Point, Elem*>& lhs,
                                  const std::pair<Point, Elem*>& rhs)
{
  return (lhs.first(0) < rhs.first(0));
}




bool CentroidPartitioner::sort_y (const std::pair<Point, Elem*>& lhs,
                                  const std::pair<Point, Elem*>& rhs)
{
  return (lhs.first(1) < rhs.first(1));
}





bool CentroidPartitioner::sort_z (const std::pair<Point, Elem*>& lhs,
                                  const std::pair<Point, Elem*>& rhs)
{
  return (lhs.first(2) < rhs.first(2));
}



bool CentroidPartitioner::sort_radial (const std::pair<Point, Elem*>& lhs,
                                       const std::pair<Point, Elem*>& rhs)
{
  return (lhs.first.size() < rhs.first.size());
}

} // namespace libMesh
