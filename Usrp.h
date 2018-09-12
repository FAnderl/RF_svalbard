/*
 * Usrp.h
 *
 *  Created on: Sep 4, 2018
 *      Author: Florian Anderl
 */

#ifndef USRP_H_
#define USRP_H_

/*Standard Library*/
#include <string>
#include <vector>



/*Includes from https://kb.ettus.com/Getting_Started_with_UHD_and_C%2B%2B#Create_Variables (UHD c++ API Documentation)*/
#include <uhd/utils/thread.hpp>
#include <uhd/utils/safe_main.hpp>
#include <uhd/usrp/multi_usrp.hpp>
#include <uhd/exception.hpp>
#include <uhd/types/tune_request.hpp>
#include <boost/program_options.hpp>
#include <boost/format.hpp>
#include <boost/thread.hpp>
#include <iostream>

#include "defines.h"




class Usrp {
public:
  Usrp();
  virtual ~Usrp();

  Usrp(std::string usrp_addr, uint64_t l_freq, uint64_t u_freq, int8_t gain);

  int UsrpConfig();

  int UsrpStartUp();

  std::complex<double> * UsrpRFDataAcquisition();


private:

  uhd::usrp::multi_usrp::sptr  usrp_intern;  /* intern USRP instance for which USRP class is wrapper*/
  uhd::rx_streamer::sptr rx_stream; /* Receiver Streamer*/

  std::string usrp_address; /* IP address of USRP; default = 192.168.10.2 */

  uint64_t center_frequency;     /* LO oscillator/ DDC-discrete oscillator center frequency for Baseband-mixing*/
  uint64_t lower_frequency;
  uint64_t upper_frequency;

  uint64_t sample_rate;          /* target sample_rate that is sent to the PC from the FPGA, NB! Nyquist -> sample_rate determines the highest reproducible frequency*/

  int8_t gain;                 /*Gain in dB of Rx chain*/



  /*Data Acquisition*/

  std::vector<std::complex<double>> buffs;  /*Memory for received samples*/


  uhd::rx_metadata_t md;	/*Container for Metadata or received samples*/



};

#endif /* USRP_H_ */
