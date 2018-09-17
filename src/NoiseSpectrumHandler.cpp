/*
 * NoiseSpectrumHandler.cpp
 *  
 *
 *  This file is part of the NoiseMap Svalbard (UNIS) project
 * 
 *  Created on: Sep 6, 2018
 *      Author: Florian Anderl (Guest Master Student AGF)
 */

#include "../NoiseSpectrumHandler.h"

NoiseSpectrumHandler::NoiseSpectrumHandler () : nsh_DFT_samples(NULL), nsh_pwr_DFT_samples(NULL)
{
  /*DEPRECATED: Do never use the default constructor*/


  active_rf_mode = RFmode::standard_band;

  integration_PWR_buffer = new double[DEF_FFT_BINSIZE];

  for(int i = 0; i< DEF_FFT_BINSIZE; i++)
    {
      integration_PWR_buffer[i] = 0;
    }

}


NoiseSpectrumHandler::~NoiseSpectrumHandler ()
{

  f_noise_spectrum.close();

}



NoiseSpectrumHandler::NoiseSpectrumHandler(RFmode rf_m) : nsh_DFT_samples(NULL), nsh_pwr_DFT_samples(NULL)
{

  active_rf_mode = rf_m;

  nsh_pwr_DFT_samples = new double[DEF_FFT_BINSIZE];

  integration_PWR_buffer = new double[DEF_FFT_BINSIZE];

  for(int i = 0; i< DEF_FFT_BINSIZE; i++)
    {
      integration_PWR_buffer[i] = 0;
    }

  frequency_band_str = "";

}



int NoiseSpectrumHandler::FileConfig(time_t start_time)
{

  /*Variables needed for processing time information*/
  char buffer[20];
  struct tm *timeinfo = localtime(&start_time);

  /*Formats time string*/
  strftime(buffer, 20, "%Y%m%d_T%H%M%S", timeinfo);

  date_time_str = buffer;



  switch(active_rf_mode)
  {
    case RFmode::standard_band : frequency_band_str = "8to20Mhz(Standardband)"; break;
    case RFmode::alternative_band: frequency_band_str = std::to_string(int(ext_lower_frequency)/1e3)+"_"
	+std::to_string(int(ext_upper_frequency)/1e3)+"kHz";

  }

  f_noise_spectrum.open(frequency_band_str+"@"+date_time_str+".csv");



  return 0;
}




/*Implements the Integration functionality*/
int NoiseSpectrumHandler::IntegratePWR()
{


  /*Copy samples to integration buffer*/
  for(int i = 0; i < DEF_FFT_BINSIZE; i++)
    {
      integration_PWR_buffer[i] = integration_PWR_buffer[i]/ (DEF_INTEGRATION_CONST*ext_sample_rate);
    }

  return 0;
}





/* Computes Magnitude of complex dft data for POWER SPECTRAL DENSITY */
int NoiseSpectrumHandler::ConvertDFTData()
{

  for(int i = 0; i < DEF_FFT_BINSIZE; i++)
    {

      integration_PWR_buffer[i] =  integration_PWR_buffer[i] +
	  (double(1.0/DEF_FFT_BINSIZE) *
	      (sqrt((nsh_DFT_samples[i][0]*nsh_DFT_samples[i][0])+
		    (nsh_DFT_samples[i][1]*nsh_DFT_samples[i][1]))*sqrt((nsh_DFT_samples[i][0]*nsh_DFT_samples[i][0])+
									(nsh_DFT_samples[i][1]*nsh_DFT_samples[i][1]))));
      /*TODO: Verify equation for DFT power spectrum:  1/n * F²*/

    }



  return 0;
}




int NoiseSpectrumHandler::GetDFTData(fftw_complex *DFTsamples)
{

  nsh_DFT_samples = DFTsamples;

  return 0;
}



int NoiseSpectrumHandler::ExportRawDataToFile()
{

  for(int i = 0; i< DEF_FFT_BINSIZE; i++)
    {
      if(i == DEF_FFT_BINSIZE-1)
	{
	  f_noise_spectrum <<  integration_PWR_buffer[i];
	}
      else
	{
	  f_noise_spectrum <<  integration_PWR_buffer[i] << ",";
	}
    }




  /*new line for new DEF_FFT_SIZE samples*/
  f_noise_spectrum << "\n";



  /*Deletion & Re-Initialization of integration buffer*/
  delete integration_PWR_buffer;

  integration_PWR_buffer = new double[DEF_FFT_BINSIZE];

  for(int i = 0; i< DEF_FFT_BINSIZE; i++)
    {
      integration_PWR_buffer[i] = 0;
    }
  return 0;
}
