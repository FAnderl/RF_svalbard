/*
 *  SdrUsrp.cpp
 *
 *
 *  This file is part of the NoiseMap Svalbard (UNIS) project
 *
 *  Created on: Sep 5, 2018
 *      Author: Florian Anderl (Guest Master Student AGF)
 */


#include "../SdrUsrp.h"
#include "../Constants.h"

/*Declaration of global variables*/
uint64_t XsampleRate;
uint64_t XlowerFrequency;
uint64_t XupperFrequency;

uint32_t XfftBinNumber;

/*Default Constructor*/
SdrUsrp::SdrUsrp(): usrp_address_(kConstUsrpAddress),center_frequency_(kDefaultCenterFrequency), lower_frequency_(kDefaultLowerFrequency),
    upper_frequency_(kDefaultUpperFrequency),sample_rate_desired_(kDefaultSampleRate), gain_(kDefaultGain)
{

  usrp_mode_ = RFmode::standard_band;

  /*For Debugging ONLY*/
  //puts("SdrUsrp Default Constructor");


  XlowerFrequency = lower_frequency_;

  XupperFrequency = upper_frequency_;

}


/*Destructor*/
SdrUsrp::~SdrUsrp()
{
  buffs_.clear();
  rx_stream_->issue_stream_cmd(uhd::stream_cmd_t::STREAM_MODE_STOP_CONTINUOUS);
}


/*Custom Constructor*/
SdrUsrp::SdrUsrp(std::string SdrUsrp_addr, uint64_t l_freq, uint64_t u_freq , int8_t Gain): usrp_address_(SdrUsrp_addr), lower_frequency_(l_freq),
    upper_frequency_(u_freq), gain_(Gain)
{


  usrp_mode_ = RFmode::alternative_band;

  center_frequency_ = 0;

  sample_rate_desired_ = 0;

  XlowerFrequency = lower_frequency_;

  XupperFrequency = upper_frequency_;


}


/*Calculates Important Signal Processing/USRP -related parameters*/
int SdrUsrp::CalculateParameters()
{

  if(usrp_mode_ == RFmode::alternative_band)
    {

      /*Calculate SdrUsrp center frequency from cmd-defined parameters*/
      center_frequency_ = lower_frequency_ + (upper_frequency_ - lower_frequency_)/2;

      /*Calculate SdrUsrp center frequency from cmd-defined parameters
       * -> Fulfills Nyquist + 5% */
      sample_rate_desired_ = (upper_frequency_ - lower_frequency_) +
	  0.05 *(upper_frequency_ - lower_frequency_) ;


      uint32_t temp_freq_res_desired = kDefaultFrequencyResolution;


      /* This routine only permits EVEN DECIMATION FACTORS (ADC-rate/requested sample rate) in order
       * to meet the USRP related performance requirements */
      do
	{
	  if(std::fmod((double(kAdcRate)/double(sample_rate_desired_)),2) == 0){break;}
	  sample_rate_desired_ = sample_rate_desired_ + 1;
	}while(std::fmod((double(kAdcRate)/double(sample_rate_desired_)),2) != 0);


      /* This routine only permits fftBinNumbers for which the following conditions are met:
       *  (1) the sample rate is a integer multiple of fftbinNumber
       *  (2) The FFT contains at least 1024 Bins */
      while((std::fmod((double(sample_rate_desired_)/double(temp_freq_res_desired)),1) != 0) || (
	  double(sample_rate_desired_)/double(temp_freq_res_desired) < 1024))
	{
	  temp_freq_res_desired  = temp_freq_res_desired - 1;
	}


      std::cout << "Frequency Resolution of DFT Output: " << temp_freq_res_desired << std::endl;


      /*SETS FFT BINSIZE -> IMPORTANT*/
      XfftBinNumber = sample_rate_desired_/temp_freq_res_desired;

      std::cout << "XfftBinNumber: " << XfftBinNumber << std::endl;

    }


  else if(usrp_mode_ == RFmode::standard_band)
    {
      sample_rate_desired_ = kDefaultSampleRate;

      XfftBinNumber = kDefaultFFTBinNumber;



    }


  if(XdebugMode)
    std::cout << "FFT Binsize set to: " << XfftBinNumber << std::endl;


  return 0;
}





/*Allocates Memory according to given parameters*/
int SdrUsrp::PrepareSampleBuffer()
{

  buffs_.resize(XfftBinNumber);

  return 0;
}



/*Configures the SdrUsrp for Receving*/
int SdrUsrp::InitializeUSRP()
{

  std::cout << "Configuring SdrUsrp Device...\n" << std::endl;

  usrp_address_ = "addr=" + usrp_address_;


  usrp_intern_ =  uhd::usrp::multi_usrp::make(usrp_address_); /* Creates internal SdrUsrp object for which generic SdrUsrp class is wrapper*/


  usrp_intern_->set_clock_source(kConstClockSource); /* Sets SdrUsrp clock source */


  usrp_intern_->set_rx_subdev_spec(kConstSubDevice); /* Sets subdevice specification (daughterboard) -
  > see http://files.ettus.com/manual/page_configuration.html#config_subdev */


  std::cout << "Chosen Subdevice (Daughterboard): \n" << usrp_intern_->get_pp_string() << std::endl;


  usrp_intern_->set_rx_rate(sample_rate_desired_);  /* Sets Rx sample rate */


  XsampleRate = usrp_intern_->get_rx_rate();  /* Initializes external sample rate variable */


  std::cout << "I/Q Sampling Rate set to: " << XsampleRate << std::endl;



  /*Center Frequency*/
  uhd::tune_request_t tune_request(center_frequency_); /* Makes tune request to lo for mixing down to baseband in analog frontend*/

  usrp_intern_->set_rx_freq(tune_request);


  std::cout << "Rx Center Frequency set to:  " << usrp_intern_->get_rx_freq() << std::endl;



  usrp_intern_->set_rx_gain(gain_);   /*Sets USRP gain (if possible, depending on USRP model)*/

  std::cout << "Rx Gain set to: " << usrp_intern_->get_rx_gain() << std::endl;


  /*Specify Antenna*/
  /*TODO: Necessary for some USRP Models */


  return 0;
}


/* Starts USRP & sets up streaming receiver object
 * -> stream command is issued here but actual data aquisition is not done here*/
int SdrUsrp::StartUpUSRP()
{

  std::cout << "Begin Streaming" << std::endl;


  uhd::stream_args_t stream_args("fc64");  /*stream in complex double 64bit format*/


  rx_stream_ = usrp_intern_->get_rx_stream(stream_args); /*make receiver stream object*/


  uhd::stream_cmd_t sc(uhd::stream_cmd_t::STREAM_MODE_START_CONTINUOUS);  /* Creates Stream Command Object */


  sc.stream_now = true;


  rx_stream_->issue_stream_cmd(sc);  /* Stream Command issued by the STREAM object */


  return 0;
}






std::complex<double>* SdrUsrp::RFDataAcquisitionUSRP()
{


  std::vector<std::complex<double>> dummy;


  size_t num_rx_samples = 0;

  /* ACTUAL STREAMING is done here ->
   * loop is only left if required number of samples is received from USRP*/
  do{


      num_rx_samples = rx_stream_->recv(&buffs_.front(), buffs_.size(), md_, 1);  /* DATA Acquisition happens here*/


      if(num_rx_samples != buffs_.size()){continue;}


  }while(num_rx_samples != XfftBinNumber);



  if (num_rx_samples != XfftBinNumber)
    {
      std::cout << "CRITICAL ERROR: FALSE NUMBER OF SAMPLES... ABORT" <<std::endl;
      exit(-1);
    }



  XnumRecvSamplesFileTag = XnumRecvSamplesFileTag + num_rx_samples; /* Incrementing extern control variable -> main.cpp*/


  XnumRecvSamplesIntegrationTag = XnumRecvSamplesIntegrationTag + num_rx_samples;    /* Incrementing extern control variable */



  return &buffs_[0];  /* Returns pointer to adress of FIRST ELEMENT of buffer containing the last received samples*/
}

