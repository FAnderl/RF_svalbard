/*
 * defines.h
 *
 *  Created on: Sep 4, 2018
 *      Author: Florian Anderl
 */

#ifndef DEFINES_H_
#define DEFINES_H_

#include <string>
#include <time.h>
#include <fftw3.h>


/*---------------VERSION INFORMATION -----------------------*/
const std::string version_number = "0.1";



/*--------------------------GLOBAL CONSTANTS-------------------------------*/

const uint64_t DEF_CENT_FREQ = 14000000;
const uint64_t DEF_L_FREQ = 8000000;
const uint64_t DEF_U_FREQ = 20000000;
const uint64_t DEF_SAMP_RATE = 12500000;
const int8_t DEF_GAIN = 0;
const double DEF_INTEGRATION_CONST  = 3;

const uint32_t DEF_ADC_RATE = 100000000;
const uint32_t DEF_FREQ_RES = 10000; /* minimum 10 kHz ferquency resolution */



/*-----------------------SIGNAL PROCESSING CONSTANTS----------------------------------------*/


/*Defines NUMBER of acquired samples & FFT size -> of course very, very, very important */
const int32_t DEF_FFT_BINSIZE = 1250 ;

/*File Constant defines when a new file for data storage will be create
DEFINITION: time of file coverage in seconds*/
const int32_t DEF_FILE_CONSTANT =  60;


/*Default values for default Usrp constructor*/
const std::string const_usrp_addr = "192.168.10.2";
const std::string const_clock_src = "internal";
const std::string const_subdev = "A:A"; /*For LFRX daughterboard using RXA -> TODO: Verify*/


typedef enum RFmode {
  standard_band = 0u,
  alternative_band = 1u,
}RFmode;


/*-------------------------GLOBAL VARIABLES--------------------------------------------*/
extern uint64_t ext_num_FILE_recv_RF_samps;
extern uint64_t ext_num_INT_recv_RF_samps;

extern uint64_t ext_sample_rate;
extern uint64_t ext_lower_frequency;
extern uint64_t ext_upper_frequency;
extern int8_t ext_rx_gain;

extern uint32_t ext_fft_resolution; /*strongly coupled with other parameters*/
extern std::string ext_dev_addr;


#endif /* DEFINES_H_ */
