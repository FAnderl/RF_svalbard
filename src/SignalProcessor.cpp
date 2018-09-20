/*
 * SignalProcessor.cpp
 *  
 *
 *  This file is part of the NoiseMap Svalbard (UNIS) project
 * 
 *  Created on: Sep 6, 2018
 *      Author: Florian Anderl (Guest Master Student AGF)
 */

#include "../SignalProcessor.h"

SignalProcessor::SignalProcessor () : nsh_DFT_samples_(nullptr)
{
  /*DEPRECATED: Do never use the default constructor*/

  active_rf_mode_ = RFmode::standard_band;

  integration_PWR_buffer_ = new double[XfftBinNumber];

  for(uint i = 0; i < XfftBinNumber; i++)
    {
      integration_PWR_buffer_[i] = 0;
    }

}


SignalProcessor::~SignalProcessor ()
{

  delete []integration_PWR_buffer_; /*free memory -> note delete []!*/
  f_noise_spectrum_.close();	   /*closes current data output file orderly*/

}



SignalProcessor::SignalProcessor(RFmode rf_m) : nsh_DFT_samples_(nullptr)
{

  active_rf_mode_ = rf_m;


  integration_PWR_buffer_ = new double[XfftBinNumber];

  for(uint i = 0; i < XfftBinNumber; i++)
    {
      integration_PWR_buffer_[i] = 0;
    }

  frequency_band_str_ = "";

}



int SignalProcessor::InitializeOutFile(time_t start_time)
{

  /*Variables needed for processing time information*/
  char buffer[20];
  struct tm *timeinfo = localtime(&start_time);

  /*Formats time string*/
  strftime(buffer, 20, "%Y%m%d_T%H%M%S", timeinfo);

  date_time_str_ = buffer;



  switch(active_rf_mode_)
  {
    case RFmode::standard_band : frequency_band_str_ = "8to20Mhz(Standardband)"; break;
    case RFmode::alternative_band: frequency_band_str_ = std::to_string(int(XlowerFrequency)/1e3)+"_"
	+std::to_string(int(XupperFrequency)/1e3)+"kHz";

  }

  f_noise_spectrum_.open(frequency_band_str_+"@"+date_time_str_+".csv");



  return 0;
}



/*Rearranges the DFT samples from "in-order" to
 * negative/positive frequencies */

int SignalProcessor::RearrangeDFT()
{

  fftw_complex * temp_DFT = new fftw_complex[XfftBinNumber/2];

  /*Initialize array*/

  for(size_t i_t = 0; i_t < sizeof(temp_DFT); i_t++)
    {
      temp_DFT[i_t][0] = 0;
      temp_DFT[i_t][1] = 0;
    }


  /*Temporarily stores second half of DFT samples in buffer*/
  for(uint i = 0; i < XfftBinNumber/2; i++)
    {
      temp_DFT[i][0] = nsh_DFT_samples_[XfftBinNumber/2 + i][0];
      temp_DFT[i][1] = nsh_DFT_samples_[XfftBinNumber/2 + i][1];
    }



  /*Realigns DFT samples*/
  for(uint i = 0 ; i < XfftBinNumber/2; i++)
    {

      nsh_DFT_samples_[i+XfftBinNumber/2][0] = nsh_DFT_samples_[i][0];
      nsh_DFT_samples_[i+XfftBinNumber/2][1] = nsh_DFT_samples_[i][1];


      nsh_DFT_samples_[i][0] = temp_DFT[i][0];
      nsh_DFT_samples_[i][1] = temp_DFT[i][1];

    }


  delete []temp_DFT;

  return 0;
}



/*Implements the Integration functionality*/
int SignalProcessor::IntegratePWR()
{


  /*Copy samples to integration buffer*/
  for(uint i = 0; i < XfftBinNumber; i++)
    {
      integration_PWR_buffer_[i] = integration_PWR_buffer_[i]/ (kDefaultIntegrationConstant * XsampleRate);
    }

  return 0;
}





/* Computes Magnitude of complex dft data for POWER SPECTRAL DENSITY */
int SignalProcessor::ConvertDFTData()
{

  for(uint i = 0; i < XfftBinNumber; i++)
    {

      integration_PWR_buffer_[i] =  integration_PWR_buffer_[i] +
	  (double(1.0/XfftBinNumber) *
	      (sqrt((nsh_DFT_samples_[i][0]*nsh_DFT_samples_[i][0])+
		    (nsh_DFT_samples_[i][1]*nsh_DFT_samples_[i][1]))*sqrt((nsh_DFT_samples_[i][0]*nsh_DFT_samples_[i][0])+
									(nsh_DFT_samples_[i][1]*nsh_DFT_samples_[i][1]))));
      /*TODO: Verify equation for DFT power spectrum:  1/n * F²*/

    }



  return 0;
}




int SignalProcessor::GetDFTData(fftw_complex *DFTsamples)
{

  nsh_DFT_samples_ = DFTsamples;

  return 0;
}


/*Resets POWER Integration Buffer after values were written to file
 * -> Note that this function should be called after ExportDataToFile() */

int SignalProcessor::ResetIntegrationBuffer()
{

  /*Re-0 initilization of integration_PWR_buffer_*/
  for(uint i = 0; i< XfftBinNumber; i++)
    {
      integration_PWR_buffer_[i] = 0;
    }


  return 0;
}


int SignalProcessor::ExportRawDataToFile()
{

  for(uint i = 0; i< XfftBinNumber; i++)
    {
      if(i == XfftBinNumber-1)
	{
	  f_noise_spectrum_ <<  integration_PWR_buffer_[i];
	}
      else
	{
	  f_noise_spectrum_ <<  integration_PWR_buffer_[i] << ",";
	}
    }



  /*NEW LINE FOR NEW #{DEF_FFT_SIZE SAMPLES}*/
  f_noise_spectrum_ << "\n";

return 0;

}
