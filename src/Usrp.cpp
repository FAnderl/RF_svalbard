/*
 *  Usrp.cpp
 *
 *
 *  This file belongs to the NoiseMap Svalbard (UNIS) project
 *
 *  Created on: Sep 5, 2018
 *      Author: Florian Anderl (Guest Master Student AGF)
 */

/*Project-specific includes*/
#include "../Usrp.h"
#include "../defines.h"

uint64_t ext_sample_rate;
uint64_t ext_lower_frequency;
uint64_t ext_upper_frequency;


Usrp::Usrp(): usrp_address(const_usrp_addr),center_frequency(DEF_CENT_FREQ), lower_frequency(DEF_L_FREQ),
    upper_frequency(DEF_U_FREQ),sample_rate(DEF_SAMP_RATE), gain(DEF_GAIN)
{

  buffs.resize(DEF_FFT_BINSIZE); /*Allocates memory for received data samples*/

  ext_sample_rate = uint64_t(DEF_SAMP_RATE);

  ext_lower_frequency = uint64_t(DEF_L_FREQ);

  ext_upper_frequency = uint64_t(DEF_U_FREQ);

}


Usrp::~Usrp()
{

}


Usrp::Usrp(std::string usrp_addr, uint64_t l_freq, uint64_t u_freq , int8_t Gain): usrp_address(usrp_addr), lower_frequency(l_freq),
    upper_frequency(u_freq), gain(Gain)
{

  buffs.resize(DEF_FFT_BINSIZE);  /*Allocates memory for received data samples*/


  /*calculate usrp center frequency from cmd-defined parameters*/
  center_frequency = lower_frequency + (upper_frequency - lower_frequency)/2;

  /*calculate usrp center frequency from cmd-defined parameters
   * TODO: rework -> fixed set of possible sample rates*/
  sample_rate = upper_frequency - lower_frequency + 5e5; /*TODO: rework*/

  ext_sample_rate = sample_rate;

  ext_lower_frequency = lower_frequency;

  ext_upper_frequency = upper_frequency;



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
  usrp_intern->set_rx_rate(sample_rate);  /*sets Rx sample rate*/

  std::cout << "Rx Sample Rate set to:  " << usrp_intern->get_rx_rate() << std::endl;


  /*Center Frequency*/
  uhd::tune_request_t tune_request(center_frequency); /*makes tune request to lo for mixing down to baseband in analog frontend*/

  usrp_intern->set_rx_freq(tune_request);

  std::cout << "Rx Center Frequency set to:  " << usrp_intern->get_rx_freq() << std::endl;


  /*Gain TODO: Fix*/
  usrp_intern->set_rx_gain(gain);

  std::cout << "Rx Gain set to: " << usrp_intern->get_rx_gain() << std::endl;


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

  /* RECEIVING */
  do{
      num_rx_samples = rx_stream->recv(&buffs.front(), buffs.size(), md, 1);

      if(num_rx_samples != DEF_FFT_BINSIZE)
	{
	  /*For Debugging only*/
	  if(num_rx_samples != DEF_FFT_BINSIZE)
	    {
	      std::cout << "ERROR: Number of acquired I/Q samples: "  <<  num_rx_samples   << "\nDiscard samples..."<< std::endl;
	    }

	  /*Deletes Contents of Vector*/
	  buffs.clear();

	  /*Reset recv-counter*/
	  num_rx_samples = 0;


	}

  }while(num_rx_samples != DEF_FFT_BINSIZE);

  /*incrementing extern control variable*/
  ext_num_FILE_recv_RF_samps = ext_num_FILE_recv_RF_samps + num_rx_samples;

  /*incrementing extern control variable*/
  ext_num_INT_recv_RF_samps = ext_num_INT_recv_RF_samps + num_rx_samples;



  return &buffs[0]; /*returns buffer containing the last received samples*/
}

