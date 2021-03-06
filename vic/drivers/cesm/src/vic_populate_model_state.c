/******************************************************************************
 * @section DESCRIPTION
 *
 * Populate model state.
 *
 * @section LICENSE
 *
 * The Variable Infiltration Capacity (VIC) macroscale hydrological model
 * Copyright (C) 2016 The Computational Hydrology Group, Department of Civil
 * and Environmental Engineering, University of Washington.
 *
 * The VIC model is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *****************************************************************************/

#include <vic_driver_cesm.h>

/******************************************************************************
 * @brief    This function handles tasks related to populating model state.
 *****************************************************************************/
void
vic_populate_model_state(char *runtype_str)
{
    extern all_vars_struct *all_vars;
    extern lake_con_struct *lake_con;
    extern domain_struct    local_domain;
    extern option_struct    options;
    extern soil_con_struct *soil_con;
    extern veg_con_struct **veg_con;
    extern filenames_struct filenames;


    size_t                  i;
    unsigned short int      runtype;

    debug("In vic_populate_model_state");

    runtype = start_type_from_char(runtype_str);

    // read the model state from the netcdf file if there is one
    if (runtype == CESM_RUNTYPE_RESTART || runtype == CESM_RUNTYPE_BRANCH) {
        // Get restart file from rpointer file
        read_rpointer_file(filenames.init_state);

        // read initial state file
        vic_restore();
    }
    else if (runtype == CESM_RUNTYPE_CLEANSTART) {
        // run type is clean start
        for (i = 0; i < local_domain.ncells_active; i++) {
            generate_default_state(&(all_vars[i]), &(soil_con[i]), veg_con[i]);
            if (options.LAKES) {
                generate_default_lake_state(&(all_vars[i]), &(soil_con[i]),
                                            lake_con[i]);
            }
        }
    }

    // compute those state variables that are derived from the others
    for (i = 0; i < local_domain.ncells_active; i++) {
        compute_derived_state_vars(&(all_vars[i]), &(soil_con[i]), veg_con[i]);
        if (options.LAKES) {
            compute_derived_lake_dimensions(&(all_vars[i].lake_var),
                                            lake_con[i]);
        }
    }
}

/******************************************************************************
 * @brief Convert runtype string to enum integer
 * @note  See CESM's seq_infodata_mod.F90 for more information
 *****************************************************************************/
unsigned short int
start_type_from_char(char *start_str)
{
    if (strcasecmp("startup", start_str) == 0) {
        return CESM_RUNTYPE_CLEANSTART;
    }
    else if (strcasecmp("continue", start_str) == 0) {
        return CESM_RUNTYPE_RESTART;
    }
    else if (strcasecmp("branch", start_str) == 0) {
        return CESM_RUNTYPE_BRANCH;
    }
    else {
        log_err("Unknown calendar specified: %s", start_str);
    }
}

/******************************************************************************
 * @brief Read rpointer file
 *****************************************************************************/
void
read_rpointer_file(char *fname)
{
    FILE *fp = NULL;

    fname = NULL;

    fp = fopen(RPOINTER, "r");

    if (fp == NULL) {
        log_err("Error reading rpointer file %s", RPOINTER);
    }

    fgets(fname, MAXSTRING, fp);

    fclose(fp);

    if (fname == NULL) {
        log_err("Error reading rpointer file %s", RPOINTER);
    }
}
