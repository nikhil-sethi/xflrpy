#ifndef ENGINE_PARAMS_H
#define ENGINE_PARAMS_H


#define PI             3.14159265358979
#define PRECISION  0.00000001  /**< Values are assumed 0 if less than this value. This is to avoid comparing the equality of two floating point numbers */


//3D analysis parameters
#define MAXWINGS            4     /**< Wing, wing2, elevator, fin, in that order.*/
#define MAXSPANSTATIONS   250     /**< The max number of stations for LLT. For a VLM analysis, this is the max number of panels in the spanwise direction. */

#endif // ENGINE_PARAMS_H
