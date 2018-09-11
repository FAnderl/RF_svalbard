/*
 * DFTNoise.h
 *  
 *
 *  This file is part of the NoiseMap Svalbard (UNIS) project
 * 
 *  Created on: Sep 5, 2018
 *      Author: Florian Anderl (Guest Master Student AGF)
 */

#ifndef DFTNOISE_H_
#define DFTNOISE_H_

#include <fftw3.h> /*Fourier Transform Library -> DFT*/
#include "defines.h"
#include "Usrp.h"


class DFTNoise
{
public:
  DFTNoise ();
  virtual
  ~DFTNoise ();


  int GetRFSamples(std::complex<double>* addr_rf_samples); /*reads RF samples from stream*/


  int ComputeNoiseDFT(); /*computes DFT of input samples*/


  /*TODO: Move functionality to dedicated class*/
  fftw_complex* ExportDFTResults();





private:

  fftw_complex * input_samples; /*contains input samples sent by USRP*/

  fftw_complex * DFT_samples; /*contains the COMPLEX DFT samples */

  fftw_plan DFT_plan; /*configuration container for DFT*/



};

#endif /* DFTNOISE_H_ */
