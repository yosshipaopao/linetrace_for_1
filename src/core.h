#ifndef _core_h
#define _core_h

#include <Arduino.h>
void cal_motor_power(int *l_power, int *r_power, int pow_l, int pow_m, int pow_s, int state, int *pre_dir)
{
    switch (state)
    {
    case 0b0000:
        switch (*pre_dir)
        {
        case -1:
            *l_power = pow_s;
            *r_power = pow_l;
            break;
        case 0:
            *l_power = pow_m;
            *r_power = pow_m;
            break;
        case 1:
            *l_power = pow_l;
            *r_power = pow_s;
            break;
        }
        break;
    case 0b0110:
        *l_power = pow_l;
        *r_power = pow_l;
        break;
    case 0b0010:
        *l_power = pow_l;
        *r_power = pow_m;
        break;
    case 0b0100:
        *l_power = pow_m;
        *r_power = pow_l;
        break;
    case 0b0011:
    case 0b0001:
        *l_power = pow_l;
        *r_power = pow_s;
        *pre_dir = 1;
        break;
    case 0b1100:
    case 0b1000:
        *l_power = pow_s;
        *r_power = pow_l;
        *pre_dir = -1;
        break;
    case 0b0111:
        *l_power = pow_l;
        *r_power = pow_m;
        *pre_dir = 1;
        break;
    case 0b1110:
        *l_power = pow_m;
        *r_power = pow_l;
        *pre_dir = -1;
        break;
    case 0b1111:
        *l_power = pow_l;
        *r_power = pow_l;
        break;
    default:
        *l_power = 0;
        *r_power = 0;
        break;
    }
}

#endif