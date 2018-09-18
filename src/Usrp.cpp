/*
 *  Usrp.cpp
 *
 *
 *  This file belongs to the NoiseMap Svalbard (UNIS) project
 *
 *  Created on: Sep 5, 2018
 *      Author: Florian Anderl (Guest Master Student AGF)
 */


#include "../Usrp.h"
#include "../defines.h"

/*Declaration of global variables*/
uint64_t ext_sample_rate;
uint64_t ext_lower_frequency;
uint64_t ext_upper_frequency;

uint32_t ext_fft_resolution; /*strongly coupled with other parameters*/

Usrp::Usrp(): usrp_address(const_usrp_addr),center_frequency(DEF_CENT_FREQ), lower_frequency(DEF_L_FREQ),
    upper_frequency(DEF_U_FREQ),sample_rate_desired(DEF_SAMP_RATE), gain(DEF_GAIN)
{

  /*For Debugging ONLY*/
  puts("USRP Default Constructor");


  ext_lower_frequency = lower_frequency;

  ext_upper_frequency = upper_frequency;

}


/*Stops Streaming from USRP Device*/
Usrp::~Usrp()
{
  buffs.clear();
  rx_stream->issue_stream_cmd(uhd::stream_cmd_t::STREAM_MODE_STOP_CONTINUOUS);
}


Usrp::Usrp(std::string usrp_addr, uint64_t l_freq, uint64_t u_freq , int8_t Gain): usrp_address(usrp_addr), lower_frequency(l_freq),
    upper_frequency(u_freq), gain(Gain)
{

  center_frequency = 0;

  sample_rate_desired = 0;

  ext_lower_frequency = lower_frequency;

  ext_upper_frequency = upper_frequency;


}


/*Calculates Valid Signal Processing/device parameters*/
int Usrp::UsrpCalculateParameters()
{


  /*Calculate usrp center frequency from cmd-defined parameters*/
  center_frequency = lower_frequency + (upper_frequency - lower_frequency)/2;

  /*Calculate usrp center frequency from cmd-defined parameters
   * TODO: rework -> fixed set of possible sample rates*/
  sample_rate_desired = (upper_frequency - lower_frequency) + 0.1 *(upper_frequency - lower_frequency) ; /*+ 10% of spectral distance -> (sufficient?) */

  /*Initializes local assist variable -> TODO: maybe replace with class member variable*/
  uint32_t temp_freq_res_desired = DEF_FREQ_RES;

  /*TODO: Test!*/
  /* Increments samples rate until even decimation rate requirement is satisfied
   * ONLY BETTER SAMPLES RATES ALLOWED*/
  do
    {
      if(std::fmod((double(DEF_ADC_RATE)/double(sample_rate_desired)),2) == 0){break;}
      sample_rate_desired = sample_rate_desired + 1;
    }while(std::fmod((double(DEF_ADC_RATE)/double(sample_rate_desired)),2) != 0);



  while((sample_rate_desired/temp_freq_res_desired)%1 != 0)
    {
      temp_freq_res_desired  = temp_freq_res_desired - 1;
    }


  /*SETS FFT BINSIZE -> EXTREMELY IMPORTANT*/
  ext_fft_resolution = sample_rate_desired/temp_freq_res_desired;

  std::cout << "FFT BINSIZE: " << ext_fft_resolution << std::endl;


  return 0;
}





/*Allocates Memory according to given parameters
 * REASONING: */
int Usrp::UsrpPrepareSampleBuffer()
{



  /*Allocates memory for received data samples*/
  buffs.resize(ext_fft_resolution);



  return 0;
}



/*Configures the USRP for Receving*/
int Usrp::UsrpConfig()
{

  std::cout << "Configuring USRP Device...\n" << std::endl;

  /*Creating USRP Object*/
  usrp_address = "addr=" + usrp_address; /*completes address line for making usrp object*/

  usrp_intern =  uhd::usrp::multi_usrp::make(usrp_address); /*creates internal usrp object for which generic Usrp class is wrapper*/


  /*Clock Source*/
  usrp_intern->set_clock_source(const_clock_src); /*Sets usrp clock source*/


  /*Daughterboard Specification*/
  usrp_intern->set_rx_subdev_spec(const_subdev); /*sets subdevice specification (daughterboard) -> see http://files.ettus.com/manual/page_configuration.html#config_subdev */

  std::cout << "Chosen Subdevice (Daughterboard): \n" << usrp_intern->get_pp_string() << std::endl;


  /*Sample Rate*/
  usrp_intern->set_rx_rate(sample_rate_desired);  /*sets Rx sample rate*/


  /*Should be same as sample_rate_desired, TODO: TEST*/
  ext_sample_rate = usrp_intern->get_rx_rate();

  std::cout << "I/Q Sampling Rate set to: " << ext_sample_rate << std::endl;



  /*Center Frequency*/
  uhd::tune_request_t tune_request(center_frequency); /*makes tune request to lo for mixing down to baseband in analog frontend*/

  usrp_intern->set_rx_freq(tune_request);


  std::cout << "Rx Center Frequency set to:  " << usrp_intern->get_rx_freq() << std::endl;


  /*Gain TODO: Fix && Uncomment*/
  //  usrp_intern->set_rx_gain(gain);
  //
  //  std::cout << "Rx Gain set to: " << usrp_intern->get_rx_gain() << std::endl;


  /*Antenna*/
  /*TODO: If necessary*/


  return 0;
}


int Usrp::UsrpStartUp()
{

  std::cout << "Begin Streaming" << std::endl;


  /*stream in complex double 64bit format*/
  uhd::stream_args_t stream_args("fc64");

  /*make receiver stream object*/
  rx_stream = usrp_intern->get_rx_stream(stream_args);


  /*TODO: try different stream commands*/
  uhd::stream_cmd_t sc(uhd::stream_cmd_t::STREAM_MODE_START_CONTINUOUS);
  sc.stream_now = true;

  /*Stream Command issued by the STREAM object*/
  rx_stream->issue_stream_cmd(sc);


  return 0;
}






std::complex<double>* Usrp::UsrpRFDataAcquisition()
{

  /*For Debugging Use Only*/
  std::vector<std::complex<double>> dummy;


  size_t num_rx_samples = 0;

  /* ACTUAL STREAMING*/
  do{


      /*TODO: Check Data types & validity of pointers/adresses & method arguments*/

      /*recv timeout currently set to 1 second*/
      num_rx_samples = rx_stream->recv(&buffs.front(), buffs.size(), md, 1);

      /*For DEBUGGING ONLY*/
//            for(int i = 0; i< buffs.size(); i++)
//      	{
//      	  std::cout << buffs[i] << ",";
//      	}

      if(num_rx_samples != buffs.size()){continue;}


  }while(num_rx_samples != ext_fft_resolution);



  if (num_rx_samples != ext_fft_resolution)
    {
      std::cout << "CRITICAL ERROR: FALSE NUMBER OF SAMPLES... ABORT" <<std::endl;
      exit(-1);
    }

  /*NOTE: num_rx_samps has to have specified & expected size at this point*/

  /*incrementing extern control variable*/
  ext_num_FILE_recv_RF_samps = ext_num_FILE_recv_RF_samps + num_rx_samples;

  /*incrementing extern control variable*/
  ext_num_INT_recv_RF_samps = ext_num_INT_recv_RF_samps + num_rx_samples;



  return &buffs[0]; /*returns pointer to adress of FIRST ELEMENT of
  buffer containing the last received samples*/
}

