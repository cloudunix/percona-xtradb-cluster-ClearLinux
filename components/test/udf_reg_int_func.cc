/* Copyright (c) 2017, Oracle and/or its affiliates. All rights reserved.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; version 2 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02111-1307  USA */

/* This test component register the myfunc_int method in init (install) and
   unregister it in deinit (uninstall). */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <mysql/components/component_implementation.h>
#include <mysql/components/service_implementation.h>
#include <mysql/components/services/udf_registration.h>
#include <string>

REQUIRES_SERVICE_PLACEHOLDER(udf_registration);
REQUIRES_SERVICE_PLACEHOLDER(udf_registration_aggregate);

/***************************************************************************
** UDF long long function.
** Arguments:
** initid       Return value from xxxx_init
** args         The same structure as to xxx_init. This structure
**              contains values for all parameters.
**              Note that the functions MUST check and convert all
**              to the type it wants!  Null values are represented by
**              a NULL pointer
** is_null      If the result is null, one should store 1 here.
** error        If something goes fatally wrong one should store 1 here.
**
** This function should return the result as a long long
***************************************************************************/

/* This function returns the sum of all arguments */

long long myfunc_int(UDF_INIT *, UDF_ARGS *args,
                    char *,
                    char *)
{ 
  long long val = 0;
  unsigned i;
  
  for (i = 0; i < args->arg_count; i++)
  { 
    if (args->args[i] == NULL)
      continue;
    switch (args->arg_type[i]) {        
    case STRING_RESULT:                 /* Add string lengths */
      val += args->lengths[i];
      break;
    case INT_RESULT:                    /* Add numbers */
      val += *((long long*) args->args[i]);
      break;
    case REAL_RESULT:                   /* Add numers as long long */
      val += (long long) *((double*) args->args[i]);
      break;
    default:
      break;
    }
  }
  return val;
}


bool myfunc_int_init(UDF_INIT *, UDF_ARGS *, char *)
{ 
  return 0;
}

/**************************************************************************************/

static mysql_service_status_t init()
{
  bool ret_int= false;
  ret_int= mysql_service_udf_registration->udf_register("myfunc_int",
                                                    INT_RESULT,
                                                    (Udf_func_any)myfunc_int,
                                                    (Udf_func_init)myfunc_int_init,
                                                    NULL);
  return ret_int;
}

static mysql_service_status_t deinit()
{
  int was_present= 0;
  for (int i=0; i<10; i++)
  {  
    mysql_service_udf_registration->udf_unregister("myfunc_int",
                                                 &was_present);
    if (was_present != 0) break;
  }
  return false;
}

BEGIN_COMPONENT_PROVIDES(test_udf_registration)
END_COMPONENT_PROVIDES()


BEGIN_COMPONENT_REQUIRES(test_udf_registration)
  REQUIRES_SERVICE(udf_registration)
  REQUIRES_SERVICE(udf_registration_aggregate)
END_COMPONENT_REQUIRES()

BEGIN_COMPONENT_METADATA(test_udf_registration)
  METADATA("mysql.author", "Oracle Corporation")
  METADATA("mysql.license", "GPL")
  METADATA("test_property", "1")
END_COMPONENT_METADATA()

DECLARE_COMPONENT(test_udf_registration, "mysql:test_udf_registration")
  init,
  deinit
END_DECLARE_COMPONENT()

DECLARE_LIBRARY_COMPONENTS
  &COMPONENT_REF(test_udf_registration)
END_DECLARE_LIBRARY_COMPONENTS