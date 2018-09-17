/*
 * NoiseSpectrumHandler.h
 *  
 *
 *  This file is part of the NoiseMap Svalbard (UNIS) project
 * 
 *  Created on: Sep 6, 2018
 *      Author: Florian Anderl (Guest Master Student AGF)
 */

#ifndef NOISESPECTRUMHANDLER_H_
#define NOISESPECTRUMHANDLER_H_


#include <stdint.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <cmath>
#include <ctgmath>
#include "defines.h"


/*class Noise Spectrum Handler takes care of interpreting, exporting and perhaps
 * converting the DFT results*/

class NoiseSpectrumHandler
{
public:
  NoiseSpectrumHandler ();
  virtual
  ~NoiseSpectrumHandler ();

  /*Constructor*/
  NoiseSpectrumHandler(RFmode rf_m);

  int FileConfig(time_t start_time);

  int ConvertDFTData();

  int IntegratePWR();

  int GetDFTData(fftw_complex* DFTsamples);

  int ExportRawDataToFile();

private:

  /*RF Recording mode in which main() was called*/
  RFmode active_rf_mode;

  std::string date_time_str;

  std::string frequency_band_str;

  std::ofstream f_noise_spectrum;

  /*Complex samples*/
  fftw_complex* nsh_DFT_samples;


  /*Integration buffer*/
  double* integration_PWR_buffer;


  /*Magnitues of samples from nsh_DFT_samples*/
  double* nsh_pwr_DFT_samples;

};

#endif /* NOISESPECTRUMHANDLER_H_ */
