/*
 *  Constants.h
 *
 *
 *  This file is part of the NoiseMap Svalbard (UNIS) project
 *
 *  Created on: Sep 5, 2018
 *      Author: Florian Anderl (Guest Master Student AGF)
 */

#ifndef CONSTANTS_H_
#define CONSTANTS_H_

#include <string>
#include <time.h>
#include <fftw3.h>


/*---------------VERSION INFORMATION -----------------------*/
const std::string version_number = "0.4";



/*--------------------------GLOBAL CONSTANTS-------------------------------*/

const uint64_t kDefaultCenterFrequency = 14000000;
const uint64_t kDefaultLowerFrequency = 8000000;
const uint64_t kDefaultUpperFrequency = 20000000;
const uint64_t kDefaultSampleRate = 12500000;
const int8_t kDefaultGain = 0;
const double kDefaultIntegrationConstant  = 3;

const uint32_t kAdcRate = 100000000;
const uint32_t kDefaultFrequencyResolution = 10000; /* minimum 10 kHz ferquency resolution */



/*-----------------------SIGNAL PROCESSING CONSTANTS----------------------------------------*/


/*Defines NUMBER of acquired samples & FFT size -> of course very, very, very important */
const int32_t kDefaultFFTBinNumber = 1250 ;

/*File Constant defines when a new file for data storage will be create
DEFINITION: time of file coverage in seconds*/
const int32_t kFileConstant =  7200;


/*Default values for default Usrp constructor*/
const std::string kConstUsrpAddress = "192.168.10.2";
const std::string kConstClockSource = "internal";
const std::string kConstSubDevice = "A:A"; /*For LFRX daughterboard using RXA -> TODO: Verify*/


typedef enum RFmode {
  standard_band = 0u,
  alternative_band = 1u,
}RFmode;


/*-------------------------GLOBAL VARIABLES--------------------------------------------*/
extern uint64_t XnumRecvSamplesFileTag;
extern uint64_t XnumRecvSamplesIntegrationTag;

extern uint64_t XsampleRate;
extern uint64_t XlowerFrequency;
extern uint64_t XupperFrequency;
extern int8_t XrxGain;

extern uint32_t XfftBinNumber;  /* Do not confuse with *kDefaultFFTBinNumber */
extern std::string XdeviceAddress;



extern bool XdebugMode;

#endif /* CONSTANTS_H_ */
