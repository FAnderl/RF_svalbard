/*
 *  SdrUsrp.cpp
 *
 *
 *  This file belongs to the NoiseMap Svalbard (UNIS) project
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

uint32_t XfftBinNumber; /*strongly coupled with other parameters*/

SdrUsrp::SdrUsrp(): usrp_address_(kConstUsrpAddress),center_frequency_(kDefaultCenterFrequency), lower_frequency_(kDefaultLowerFrequency),
    upper_frequency_(kDefaultUpperFrequency),sample_rate_desired_(kDefaultSampleRate), gain_(kDefaultGain)
{

  usrp_mode_ = RFmode::standard_band;

  /*For Debugging ONLY*/
  //puts("SdrUsrp Default Constructor");


  XlowerFrequency = lower_frequency_;

  XupperFrequency = upper_frequency_;

}


/*Stops Streaming from SdrUsrp Device*/
SdrUsrp::~SdrUsrp()
{
  buffs_.clear();
  rx_stream_->issue_stream_cmd(uhd::stream_cmd_t::STREAM_MODE_STOP_CONTINUOUS);
}


SdrUsrp::SdrUsrp(std::string SdrUsrp_addr, uint64_t l_freq, uint64_t u_freq , int8_t Gain): usrp_address_(SdrUsrp_addr), lower_frequency_(l_freq),
    upper_frequency_(u_freq), gain_(Gain)
{


  usrp_mode_ = RFmode::alternative_band;

  center_frequency_ = 0;

  sample_rate_desired_ = 0;

  XlowerFrequency = lower_frequency_;

  XupperFrequency = upper_frequency_;


}


/*Calculates Valid Signal Processing/device parameters*/
int SdrUsrp::CalculateParameters()
{

  if(usrp_mode_ == RFmode::alternative_band)
    {

      /*Calculate SdrUsrp center frequency from cmd-defined parameters*/
      center_frequency_ = lower_frequency_ + (upper_frequency_ - lower_frequency_)/2;

      /*Calculate SdrUsrp center frequency from cmd-defined parameters
       * TODO: rework -> fixed set of possible sample rates*/
      sample_rate_desired_ = (upper_frequency_ - lower_frequency_) +
	  0.05 *(upper_frequency_ - lower_frequency_) ; /*+ 5% of spectral distance -> (sufficient?) */

      /*Initializes local assist variable -> TODO: maybe replace with class member variable*/
      uint32_t temp_freq_res_desired = kDefaultFrequencyResolution;

      /*TODO: Test!*/
      /* Increments samples rate until even decimation rate requirement is satisfied
       * ONLY BETTER SAMPLES RATES ALLOWED
       * -> TODO: Rework architecture of this do.. while.. loop*/
      do
	{
	  if(std::fmod((double(kAdcRate)/double(sample_rate_desired_)),2) == 0){break;}
	  sample_rate_desired_ = sample_rate_desired_ + 1;
	}while(std::fmod((double(kAdcRate)/double(sample_rate_desired_)),2) != 0);


      /*ALLOWS ONLY INTEGER FFT BINSIZES (number of fft bins)
       * TODO: TEST! */
      while(std::fmod((double(sample_rate_desired_)/double(temp_freq_res_desired)),1) != 0)
	{
	  temp_freq_res_desired  = temp_freq_res_desired - 1;
	}


      std::cout << "Frequency Resolution of DFT Output: " << temp_freq_res_desired << std::endl;


      /*SETS FFT BINSIZE -> EXTREMELY IMPORTANT*/
      XfftBinNumber = sample_rate_desired_/temp_freq_res_desired;



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





/*Allocates Memory according to given parameters
 * REASONING: */
int SdrUsrp::PrepareSampleBuffer()
{
  /*Allocates memory for received data samples*/
  buffs_.resize(XfftBinNumber);

  return 0;
}



/*Configures the SdrUsrp for Receving*/
int SdrUsrp::InitializeUSRP()
{

  std::cout << "Configuring SdrUsrp Device...\n" << std::endl;

  /*Creating SdrUsrp Object*/
  usrp_address_ = "addr=" + usrp_address_; /*completes address line for making SdrUsrp object*/

  usrp_intern_ =  uhd::usrp::multi_usrp::make(usrp_address_); /*creates internal SdrUsrp object for which generic SdrUsrp class is wrapper*/


  /*Clock Source*/
  usrp_intern_->set_clock_source(kConstClockSource); /*Sets SdrUsrp clock source*/


  /*Daughterboard Specification*/
  usrp_intern_->set_rx_subdev_spec(kConstSubDevice); /*sets subdevice specification (daughterboard) -> see http://files.ettus.com/manual/page_configuration.html#config_subdev */

  std::cout << "Chosen Subdevice (Daughterboard): \n" << usrp_intern_->get_pp_string() << std::endl;


  /*Sample Rate*/
  usrp_intern_->set_rx_rate(sample_rate_desired_);  /*sets Rx sample rate*/


  /*Should be same as sample_rate_desired, TODO: TEST*/
  XsampleRate = usrp_intern_->get_rx_rate();

  std::cout << "I/Q Sampling Rate set to: " << XsampleRate << std::endl;



  /*Center Frequency*/
  uhd::tune_request_t tune_request(center_frequency_); /*makes tune request to lo for mixing down to baseband in analog frontend*/

  usrp_intern_->set_rx_freq(tune_request);


  std::cout << "Rx Center Frequency set to:  " << usrp_intern_->get_rx_freq() << std::endl;


  /*Gain TODO: Fix && Uncomment*/
  //  usrp_intern_->set_rx_gain(gain);
  //
  //  std::cout << "Rx Gain set to: " << usrp_intern_->get_rx_gain() << std::endl;


  /*Antenna*/
  /*TODO: If necessary*/


  return 0;
}


int SdrUsrp::StartUpUSRP()
{

  std::cout << "Begin Streaming" << std::endl;


  /*stream in complex double 64bit format*/
  uhd::stream_args_t stream_args("fc64");

  /*make receiver stream object*/
  rx_stream_ = usrp_intern_->get_rx_stream(stream_args);


  /*TODO: try different stream commands*/
  uhd::stream_cmd_t sc(uhd::stream_cmd_t::STREAM_MODE_START_CONTINUOUS);
  sc.stream_now = true;

  /*Stream Command issued by the STREAM object*/
  rx_stream_->issue_stream_cmd(sc);


  return 0;
}






std::complex<double>* SdrUsrp::RFDataAcquisitionUSRP()
{

  /*For Debugging Use Only*/
  std::vector<std::complex<double>> dummy;


  size_t num_rx_samples = 0;

  /* ACTUAL STREAMING*/
  do{


      /*TODO: Check Data types & validity of pointers/adresses & method arguments*/

      /*recv timeout currently set to 1 second*/
      num_rx_samples = rx_stream_->recv(&buffs_.front(), buffs_.size(), md_, 1);


      if(num_rx_samples != buffs_.size()){continue;}


  }while(num_rx_samples != XfftBinNumber);



  if (num_rx_samples != XfftBinNumber)
    {
      std::cout << "CRITICAL ERROR: FALSE NUMBER OF SAMPLES... ABORT" <<std::endl;
      exit(-1);
    }

  /*NOTE: num_rx_samps has to have specified & expected size at this point*/

  /*incrementing extern control variable*/
  XnumRecvSamplesFileTag = XnumRecvSamplesFileTag + num_rx_samples;

  /*incrementing extern control variable*/
  XnumRecvSamplesIntegrationTag = XnumRecvSamplesIntegrationTag + num_rx_samples;



  return &buffs_[0]; /*returns pointer to adress of FIRST ELEMENT of
  buffer containing the last received samples*/
}

