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

/* Default Constructor*/
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


/* Destructor */
SignalProcessor::~SignalProcessor ()
{

  delete []integration_PWR_buffer_; /* Free memory -> note delete []! */
  f_noise_spectrum_.close();	   /* Closes current data output file orderly wihout loosing data */

}


/* Custom Constructor */
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


  char buffer[20]; /* Variable needed for processing time information */


  struct tm *timeinfo = localtime(&start_time);  /* Variable needed for processing time information */


  strftime(buffer, 20, "%Y%m%d_T%H%M%S", timeinfo);  /* Formats time string */


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


  delete []temp_DFT; /* Free memory */

  return 0;
}



/* Implements the Integration functionality */
int SignalProcessor::IntegratePWR()
{


  /* Every Sample is averaged/integrated by divison by number of
   * number of times a fft power density frame was added up in the buffer */
  for(uint i = 0; i < XfftBinNumber; i++)
    {
      integration_PWR_buffer_[i] = integration_PWR_buffer_[i]/ (kDefaultIntegrationConstant * XsampleRate);
    }

  return 0;
}





/* Computes the Power Periodogram by applying formula:
 * PWR = 1/N ⋅ |F|² */
int SignalProcessor::ComputePowerPeriodogram()
{

  for(uint i = 0; i < XfftBinNumber; i++)
    {

      integration_PWR_buffer_[i] =  integration_PWR_buffer_[i] +
	  (double(1.0/XfftBinNumber) *
	      (sqrt((nsh_DFT_samples_[i][0]*nsh_DFT_samples_[i][0])+
		    (nsh_DFT_samples_[i][1]*nsh_DFT_samples_[i][1]))*sqrt((nsh_DFT_samples_[i][0]*nsh_DFT_samples_[i][0])+
									(nsh_DFT_samples_[i][1]*nsh_DFT_samples_[i][1]))));


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


  f_noise_spectrum_ << "\n";

return 0;

}
