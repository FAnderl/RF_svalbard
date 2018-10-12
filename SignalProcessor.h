/*
 * SignalProcessor.h
 *  
 *
 *  This file is part of the NoiseMap Svalbard (UNIS) project
 * 
 *  Created on: Sep 6, 2018
 *      Author: Florian Anderl (Guest Master Student AGF)
 */

#ifndef SIGNALPROCESSOR_H_
#define SIGNALPROCESSOR_H_


#include <stdint.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <cmath>
#include <ctgmath>

#include "Constants.h"


/*class Signal Processor takes care of interpreting, exporting and perhaps
 * converting the DFT results*/

class SignalProcessor
{
public:
  SignalProcessor ();
  virtual
  ~SignalProcessor ();

  /*Constructor*/
  SignalProcessor(RFmode rf_m);

int InitializeOutFile(time_t start_time);

  int RearrangeDFT();

  int ComputePowerPeriodogram();

  int IntegratePWR();

  int GetDFTData(fftw_complex* DFTsamples);

  int ResetIntegrationBuffer();

  int ExportRawDataToFile();

private:

  /*RF Recording mode in which main() was called*/
  RFmode active_rf_mode_;

  std::string date_time_str_;

  std::string frequency_band_str_;

  std::ofstream f_noise_spectrum_;

  /*Complex samples*/
  fftw_complex* nsh_DFT_samples_;


  /*Integration buffer*/
  double* integration_PWR_buffer_;


};

#endif /* SIGNALPROCESSOR_H_ */
