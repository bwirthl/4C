// This file is part of 4C multiphysics licensed under the
// GNU Lesser General Public License v3.0 or later.
//
// See the LICENSE.md file in the top-level for license information.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "4C_comm_mpi_utils.hpp"
#include "4C_comm_utils.hpp"
#include "4C_fem_discretization.hpp"
#include "4C_global_data.hpp"
#include "4C_mat_micromaterial.hpp"
#include "4C_so3_hex27.hpp"

FOUR_C_NAMESPACE_OPEN



/*----------------------------------------------------------------------*
 |  homogenize material density (public)                                |
 *----------------------------------------------------------------------*/
// this routine is intended to determine a homogenized material
// density for multi-scale analyses by averaging over the initial volume

void Discret::Elements::SoHex27::soh27_homog(Teuchos::ParameterList& params)
{
  if (Core::Communication::my_mpi_rank(
          Global::Problem::instance(0)->get_communicators()->sub_comm()) == owner())
  {
    double homogdens = 0.;
    const static std::vector<double> weights = soh27_weights();

    for (int gp = 0; gp < NUMGPT_SOH27; ++gp)
    {
      const double density = material()->density(gp);
      homogdens += detJ_[gp] * weights[gp] * density;
    }

    double homogdensity = params.get<double>("homogdens", 0.0);
    params.set("homogdens", homogdensity + homogdens);
  }

  return;
}


/*----------------------------------------------------------------------*
 |  Read restart on the microscale                                      |
 *----------------------------------------------------------------------*/
void Discret::Elements::SoHex27::soh27_read_restart_multi()
{
  std::shared_ptr<Core::Mat::Material> mat = material();

  if (mat->material_type() == Core::Materials::m_struct_multiscale)
  {
    auto* micro = dynamic_cast<Mat::MicroMaterial*>(mat.get());
    int eleID = id();
    bool eleowner = false;
    if (Core::Communication::my_mpi_rank(
            Global::Problem::instance()->get_dis("structure")->get_comm()) == owner())
      eleowner = true;

    for (int gp = 0; gp < NUMGPT_SOH27; ++gp) micro->read_restart(gp, eleID, eleowner);
  }

  return;
}

FOUR_C_NAMESPACE_CLOSE
