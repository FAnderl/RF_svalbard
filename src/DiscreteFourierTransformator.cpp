/*
 * DiscreteFourierTransformator.cpp
 *  
 *
 *  This file is part of the NoiseMap Svalbard (UNIS) project
 * 
 *  Created on: Sep 5, 2018
 *      Author: Florian Anderl (Guest Master Student AGF)
 */

#include "../DiscreteFourierTransformator.h"

#include <gnuradio/fft/window.h>

DiscreteFourierTransformator::DiscreteFourierTransformator ()
{

  input_samples_ = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * XfftBinNumber);

  /*Output memory which  will contain DTF samples in COMPLEX Format*/
  DFT_samples_ = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * XfftBinNumber);


  DFT_plan_ = fftw_plan_dft_1d(XfftBinNumber, input_samples_, DFT_samples_, FFTW_FORWARD, FFTW_ESTIMATE);


}

DiscreteFourierTransformator::~DiscreteFourierTransformator ()
{
  fftw_destroy_plan(DFT_plan_);
  fftw_free(input_samples_);
  fftw_free(DFT_samples_);
}


/*Gets RF samples & copies them into allocated DFT input memory*/
int DiscreteFourierTransformator::GetRFSamples(std::complex<double> *addr_rf_samples)
{

  for(int i = 0; i < XfftBinNumber; i++)
    {
      input_samples_[i][0] = addr_rf_samples[i].real() ;   /*Copies Real Part of Sample*/
      input_samples_[i][1] = addr_rf_samples[i].imag() ;   /*Copies Imaginary Part of Sample*/
    }

  return 0;
}


/*Applies windowing to RF samples BEFORE applying DFT */
int DiscreteFourierTransformator::Windowing()
{


  std::vector<float> blackmann_window = gr::fft::window::blackman(XfftBinNumber);


  /*Apply blackmann window to TIME DOMAIN input samples*/
  for(int i = 0; i < XfftBinNumber; i++)
    {
      input_samples_[i][0] = input_samples_[i][0] * blackmann_window[i];
      input_samples_[i][1] = input_samples_[i][1] * blackmann_window[i];

    }




  return 0;

}


int DiscreteFourierTransformator::ComputeDFT()
{

  /*Computation of DFT implemented as FFT*/
  fftw_execute(DFT_plan_);

  return 0;
}



fftw_complex* DiscreteFourierTransformator::ExportDFTResults()
{
  return DFT_samples_;
}


