/*
 * DiscreteFourierTransformator.h
 *  
 *
 *  This file is part of the NoiseMap Svalbard (UNIS) project
 * 
 *  Created on: Sep 5, 2018
 *      Author: Florian Anderl (Guest Master Student AGF)
 */

#ifndef DISCRETEFOURIERTRANSFORMATOR_H_
#define DISCRETEFOURIERTRANSFORMATOR_H_

#include <fftw3.h> /*Fourier Transform Library -> DFT*/

#include "Constants.h"
#include "SdrUsrp.h"


class DiscreteFourierTransformator
{

public:
  DiscreteFourierTransformator ();
  virtual
  ~DiscreteFourierTransformator ();


  /*reads RF samples from stream*/
  int GetRFSamples(std::complex<double>* addr_rf_samples);

  /*Applies a Blackmann Window to time-domain samples*/
  int Windowing();


  /*computes DFT of input samples*/
  int ComputeDFT();



  /*TODO: Move functionality to dedicated class*/
  fftw_complex* ExportDFTResults();





private:

  fftw_complex * input_samples_; /*contains input samples sent by USRP*/

  fftw_complex * DFT_samples_; /*contains the COMPLEX DFT samples */

  fftw_plan DFT_plan_; /*configuration container for DFT*/



};

#endif /* DISCRETEFOURIERTRANSFORMATOR_H_ */
