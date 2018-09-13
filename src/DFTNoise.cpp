/*
 * DFTNoise.cpp
 *  
 *
 *  This file is part of the NoiseMap Svalbard (UNIS) project
 * 
 *  Created on: Sep 5, 2018
 *      Author: Florian Anderl (Guest Master Student AGF)
 */

#include "../DFTNoise.h"


/*WINDOW Function Library*/
#include <gnuradio/fft/window.h>

DFTNoise::DFTNoise ()
{

  input_samples = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * DEF_FFT_BINSIZE);

  /*Output memory which  will contain DTF samples in COMPLEX Format*/
  DFT_samples = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * DEF_FFT_BINSIZE);

  /*Preliminary DFT Plan TODO: Decide on parameters (FFTW_FORWARD means 'normal' FT)*/
  DFT_plan = fftw_plan_dft_1d(DEF_FFT_BINSIZE, input_samples, DFT_samples, FFTW_FORWARD, FFTW_ESTIMATE);


}

DFTNoise::~DFTNoise ()
{
  fftw_destroy_plan(DFT_plan);
  fftw_free(input_samples), fftw_free(DFT_samples);
}


/*Gets RF samples & copies them into allocated DFT input memory*/
int DFTNoise::GetRFSamples(std::complex<double> *addr_rf_samples)
{

  for(int i = 0; i < DEF_FFT_BINSIZE; i++)
    {
      input_samples[i][0] = addr_rf_samples[i].real() ;   /*Copies Real Part of Sample*/
      input_samples[i][1] = addr_rf_samples[i].imag() ;   /*Copies Imaginary Part of Sample*/
    }

  return 0;
}


/*Applies windowing to RF samples BEFORE applying DFT */
int DFTNoise::Windowing()
{

  /*TODO: Verify that the window being of type FLOAT is not a problem !
   *
   * Sets up blackmann window of size FFT_SIZE*/
  gr::fft::window blackmann_window = gr::fft::window::blackman(DEF_FFT_BINSIZE);


  /*Apply blackmann window to TIME DOMAIN input samples*/
  for(int i = 0; i < DEF_FFT_BINSIZE; i++)
    {
      input_samples[i][0] = input_samples[i][0] * blackmann_window[i];
      input_samples[i][1] = input_samples[i][1] * blackmann_window[i];

    }




  return 0;

}


int DFTNoise::ComputeNoiseDFT()
{

  /*Computation of DFT implemented as FFT*/
  fftw_execute(DFT_plan);

  return 0;
}


/*TODO: Check return_type: pointer of type fftw_complex*/
fftw_complex* DFTNoise::ExportDFTResults()
{
  return DFT_samples;
}


