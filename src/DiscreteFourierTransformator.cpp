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

/* Default Constructor */
DiscreteFourierTransformator::DiscreteFourierTransformator ()
{

  /* Initializatin of buffers for DFT */

  input_samples_ = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * XfftBinNumber);


  DFT_samples_ = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * XfftBinNumber);


  DFT_plan_ = fftw_plan_dft_1d(XfftBinNumber, input_samples_, DFT_samples_, FFTW_FORWARD, FFTW_ESTIMATE); /* Creates DFT plan -> see FFTW documentation: http://www.fftw.org/fftw3.pdf*/


}

DiscreteFourierTransformator::~DiscreteFourierTransformator ()
{
  fftw_destroy_plan(DFT_plan_);
  fftw_free(input_samples_);
  fftw_free(DFT_samples_);
}



int DiscreteFourierTransformator::GetRFSamples(std::complex<double> *addr_rf_samples)
{

  for(uint i = 0; i < XfftBinNumber; i++)
    {
      input_samples_[i][0] = addr_rf_samples[i].real() ;   /*Copies Real Part of Sample*/
      input_samples_[i][1] = addr_rf_samples[i].imag() ;   /*Copies Imaginary Part of Sample*/
    }

  return 0;
}


/* Applies windowing to RF samples BEFORE applying DFT
 * -> the window is implemented through gnuradio:fft:window library */
int DiscreteFourierTransformator::Windowing()
{


  std::vector<float> blackmann_window = gr::fft::window::blackman(XfftBinNumber);


  /*Apply blackmann window to TIME DOMAIN input samples*/
  for(uint i = 0; i < XfftBinNumber; i++)
    {
      input_samples_[i][0] = input_samples_[i][0] * blackmann_window[i];
      input_samples_[i][1] = input_samples_[i][1] * blackmann_window[i];

    }




  return 0;

}


int DiscreteFourierTransformator::ComputeDFT()
{

  fftw_execute(DFT_plan_);   /* Computation of DFT implemented as FFT */

  return 0;
}



fftw_complex* DiscreteFourierTransformator::ExportDFTResults()
{
  return DFT_samples_;
}


