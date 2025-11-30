#include "../include/project.h"

/*=========
* METHODS: *
==========*/

bool params_validate(parameters_table *params, int params_count) {
    //Validation foreach parameters which takes values:
    for (int i=0; i<params_count; i++) {
        //Current parameter:
        parameters_table *param = &params[i];

        //Rules for each type of parameters:
        switch(param->parameter_type) {
            case PARAM_REMOTE_CONFIG:
                //The parameters isn't correct if the value is null:
                if (strlen(param->parameter_value.str_param) == 0) {
                    return false;
                }
                break;

                case PARAM_CONNEXION_TYPE:
                    //The parameters isn't correct if the value is null:
                    if (strlen(param->parameter_value.str_param) == 0) {
                        return false;
                    }
                    break;

                case PARAM_PORT:
                    //The parameters isn't correct if the value is null:
                    if (param->parameter_value.int_param == 0) {
                        return false;
                    }
                    break;

                case PARAM_LOGIN:
                    //The parameters isn't correct if the value is null:
                    if (strlen(param->parameter_value.str_param) == 0) {
                        return false;
                    }
                    break;
                        
                case PARAM_REMOTE_SERVER:
                    //The parameters isn't correct if the value is null:
                    if (strlen(param->parameter_value.str_param) == 0) {
                        return false;
                    }
                    break;

                case PARAM_USERNAME:
                    //The parameters isn't correct if the value is null:
                    if (strlen(param->parameter_value.str_param) == 0) {
                        return false;
                    }
                    break;
                
                case PARAM_PASSWORD:
                    //The parameters isn't correct if the value is null:
                    if (strlen(param->parameter_value.str_param) == 0) {
                        return false;
                    }
                    break;
            }
        }

    //If nothing goes wrong we return true:
    return true;
}