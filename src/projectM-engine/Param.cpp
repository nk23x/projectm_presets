/**
 * projectM -- Milkdrop-esque visualisation SDK
 * Copyright (C)2003-2004 projectM Team
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * See 'LICENSE.txt' included within this release
 *
 */

/* Basic Parameter Functions */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "projectM.hpp"

#include "fatal.h"
#include "Common.hpp"

#include "CustomShape.hpp"
#include "Eval.hpp"
#include "Expr.hpp"
#include "InitCond.hpp"
#include "Param.hpp"
#include "Preset.hpp"
#include <map>

/** Constructor */
Param::Param( std::string _name, short int _type, short int _flags, void * _engine_val, void * _matrix,
              CValue _default_init_val, CValue _upper_bound, CValue _lower_bound):
	name(_name),
        type(_type),
        flags (_flags),
        matrix_flag (0),
        engine_val(_engine_val),
        matrix (_matrix),
        default_init_val (_default_init_val),
        upper_bound (_upper_bound),
        lower_bound (_lower_bound)
    {

    /** @@FIX THIS */
    /// @bug listen to above's advice! 
    /// @NOTE this is a hack, but will probably work fine
    projectM::currentEngine->getCurrentMeshSize(this->gx, this->gy);
    
}


/* Creates a user defined parameter */
Param::Param(std::string _name) :
	name(_name),
        type(P_TYPE_DOUBLE),
	flags(P_FLAG_USERDEF),
        matrix_flag(0),
        matrix(0)
        {

	engine_val = new float();

	default_init_val.float_val = DEFAULT_DOUBLE_IV;
        upper_bound.float_val = DEFAULT_DOUBLE_UB;
        lower_bound.float_val = DEFAULT_DOUBLE_LB;
   
    /** @@FIX THIS */
    /// @bug listen to above's advice! 
    /// @NOTE this is a hack, but will probably work fine

    projectM::currentEngine->getCurrentMeshSize(this->gx, this->gy);
    assert(this->gx > 0);
    assert(this->gy > 0); 
   
}

/* Free's a parameter type */
Param::~Param() {

    int x;

    // I hate this, but will let it be for now
    if (flags & P_FLAG_USERDEF) {
        delete((double*)engine_val);
    }

    //if (!(param->flags & P_FLAG_DONT_FREE_MATRIX)) {
    if (matrix && !(flags & P_FLAG_DONT_FREE_MATRIX)) {

        if (flags & P_FLAG_PER_POINT) {
            free(matrix);
	    matrix = 0;
        } //FIX THIS NOW XMAS05
        else if (flags & P_FLAG_PER_PIXEL) {
            for (x = 0; x < gx; x++)
                free(((float**)matrix)[x]);
            free(matrix);
            matrix = 0;
        }
    }

    if (PARAM_DEBUG) printf("~Param: freeing \"%s\".\n", name.c_str());
}


/* Returns nonzero if the string is valid parameter name */
bool Param::is_valid_param_string( const char * string ) {

    if (string == NULL)
        return false;

    /* This ensures the first character is non numeric */
    if ( ((*string) >= 48) && ((*string) <= 57))
        return false;

    /* These probably should never happen */
    if (*string == '.')
        return false;

    if (*string == '+')
        return false;

    if (*string == '-')
        return false;

    /* Could also add checks for other symbols. May do later */

    return true;

}



/* Loads a float parameter into the builtin database */
Param * Param::new_param_float(char * name, short int flags, void * engine_val, void * matrix,
                               float upper_bound, float lower_bound, float init_val) {

    Param * param;
    CValue iv, ub, lb;
    assert(engine_val);

    iv.float_val = init_val;
    ub.float_val = upper_bound;
    lb.float_val = lower_bound;

    if ((param = new Param(name, P_TYPE_DOUBLE, flags, engine_val, matrix,iv, ub, lb)) == NULL)
        return NULL;


    /* Finished, return success */
    return param;
}

/* Creates a new parameter of type int */
Param * Param::new_param_int(char * name, short int flags, void * engine_val,	
                             int upper_bound, int lower_bound, int init_val) {

    Param * param;
    CValue iv, ub, lb;
    assert(engine_val);

    iv.int_val = init_val;
    ub.int_val = upper_bound;
    lb.int_val = lower_bound;

    if ((param = new Param(name, P_TYPE_INT, flags, engine_val, NULL, iv, ub, lb)) == NULL)
        return NULL;


    /* Finished, return success */
    return param;
}

/* Creates a new parameter of type bool */
Param * Param::new_param_bool(char * name, short int flags, void * engine_val,
                              int upper_bound, int lower_bound, int init_val) {

    Param * param;
    CValue iv, ub, lb;
    assert(engine_val);

    iv.bool_val = init_val;
    ub.bool_val = upper_bound;
    lb.bool_val = lower_bound;

    if ((param = new Param(name, P_TYPE_BOOL, flags, engine_val, NULL, iv, ub, lb)) == NULL)
        return NULL;


    /* Finished, return success */
    return param;
}

/* Creates a new parameter of type bool */
Param * Param::new_param_string(char * name, short int flags, void * engine_val) {

    Param * param;
    CValue iv, ub, lb;
    assert(engine_val);

//    iv.bool_val = init_val;
  //  ub.bool_val = upper_bound;
  //  lb.bool_val = lower_bound;

    if ((param = new Param(name, P_TYPE_STRING, flags, engine_val, NULL, iv, ub, lb)) == NULL)
        return NULL;


    /* Finished, return success */
    return param;
}

