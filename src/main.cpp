/*
 *  main.cpp
 *
 *
 *  This file is part of the NoiseMap Svalbard (UNIS) project
 *
 *  Created on: Sep 5, 2018
 *      Author: Florian Anderl (Guest Master Student AGF)
 */

#include "../Constants.h"


/*generic command line parser
 * DOCUMENTATION: -> https://github.com/jarro2783/cxxopts*/
#include <cxxopts.hpp>

#include "../DiscreteFourierTransformator.h"
#include "../SdrUsrp.h"
#include "../SignalProcessor.h"


uint64_t XnumRecvSamplesFileTag;
uint64_t XnumRecvSamplesIntegrationTag;
std::string XdeviceAddress;
bool XdebugMode;




/*Input Arguments: "Device Address, Start & End Frequency of frequency band of interest "*/

int main(int argc, char * argv[])
{

  /*---------------------------------------------Command Line Parser Block--------------------------------*/

  cxxopts::Options cmlo("RF_Svalbard", "UNIS Svalbard RF Background Recording (SuperDARN, KHO)");

  cmlo.add_options()

			("a, deviceAddress" , " USRP hardware address; if not defined default address (192.68.10.2) is used", cxxopts::value<std::string>())
			("l, lowerFrequency" , "Defines lower threshold for recorded band",cxxopts::value<uint64_t>())
			("u, upperFrequency","Defines upper threshold for recorded band ",cxxopts::value<uint64_t>())
			("g, gain", "Defines USRP Rx gain",cxxopts::value<int8_t>())
			("w, windowing", "If set to true, Blackmann window is applied prior to DFT")
			("inorder", "If set to true, the DFT is stored in order and NOT DC-centered")
			("d, Debug", "If set on true, Debug mode is active enabling additional console output")
			;


  auto result = cmlo.parse(argc, argv);


  uhd::set_thread_priority_safe();  /*Sets threat priority -> Can only be executed properly when in sudo/root mode*/



  XnumRecvSamplesFileTag = 0;  /*Default initialization of GLOBAL control variable*/


  XnumRecvSamplesIntegrationTag = 0;  /*Default initialization of GLOBAL control variable*/



  RFmode activeRF_mode;  /*Stores active RF mode*/


  if(result["d"].as<bool>())
    {
      XdebugMode = true;
    }
  else
    {
      XdebugMode = false;
    }

  if(result["w"].as<bool>())
    {
      std::cout << "\n********************************\nINFO: Blackmann Window applied prior to DFT (gnuradio-fft/window.h)\n"
	  "************************************\n" << std::endl;

    }


  if(!(result["inorder"].as<bool>()))
    {
      std::cout << "\n********************************\nINFO: DFT results are stored DC-centerered\n"
	  "************************************\n" << std::endl;
    }

  else
    {
      std::cout << "\n********************************\nINFO: DFT results are stored IN-ORDER"
	  "************************************\n" << std::endl;
    }

  /*------------------------- EXECUTION ----------------------------------------------*/


  SdrUsrp *usrp_wrapper = nullptr;


  /*----------------CMD ARGUMENT MODE--------------------------------------*/
  if((result.count("l") == 1) && (result.count("u") == 1))
    {
      activeRF_mode = RFmode::alternative_band;




      if(result.count("a") == 1)
	{

	  if(result["g"].count() == 1)
	    {
	      puts("gain specified");
	      usrp_wrapper = new SdrUsrp(result["a"].as<std::string>(), result["l"].as<uint64_t>(),
					 result["u"].as<uint64_t>(), result["g"].as<int8_t>() );
	    }

	  else
	    {
	      usrp_wrapper = new SdrUsrp(kConstUsrpAddress, result["l"].as<uint64_t>(),
					 result["u"].as<uint64_t>(), kDefaultGain);

	    }
	}

      else
	{


	  std::cout << "INFO: No USRP address defined -> using default address: 192.168.10.2 " << std::endl;

	  if(result["g"].count() == 1)
	    {
	      usrp_wrapper = new SdrUsrp(kConstUsrpAddress, result["l"].as<uint64_t>(),
					 result["u"].as<uint64_t>(), result["g"].as<int8_t>() );
	      puts("gain specified");
	    }

	  else
	    {
	      usrp_wrapper = new SdrUsrp(kConstUsrpAddress, result["l"].as<uint64_t>(),
					 result["u"].as<uint64_t>(), kDefaultGain);

	    }
	}


    }




  /*-----------------DEFAULT MODE--------------------*/
  else
    {

      std::cout << "INFO: Using default configuration" << std::endl;
      activeRF_mode = RFmode::standard_band;
      usrp_wrapper = new SdrUsrp();
    }









  /*-------------------------------------- STANDARD PROCEDURE -----------------------------------------------*/


  /*NEW*/
  usrp_wrapper->CalculateParameters();

  /*NEW*/
  usrp_wrapper->PrepareSampleBuffer();




  usrp_wrapper->InitializeUSRP();


  DiscreteFourierTransformator *dft_wrapper = new DiscreteFourierTransformator();


  time_t init_start_time = time(NULL);  /* Container Variable storing start_time of for first DataFile*/


  SignalProcessor *sp = nullptr;


  sp = new SignalProcessor(activeRF_mode);


  /*First Call of InitializeOutFile -> but not the last (see while-loop below)
   * -> new call to InitializeOutFile every 2h
   * -> see loop below */
  sp->InitializeOutFile(init_start_time);


  usrp_wrapper->StartUpUSRP();


  /*Temporary POINTER to Container of RF samples for hand-over to DFT-wrapper instance*/
  std::complex<double> * temp_buff_rf;



  /*------------------------------RECEIVING SAMPLES & DATA PROCESSING ----------------------------------*/


  while(true)
    {


      temp_buff_rf = usrp_wrapper->RFDataAcquisitionUSRP();  /* Issues Receive command to USRP */

      /* Checks wether number of samples exceeds hard-coded threshold
       * -> if yes, new output file is created */
      if(XnumRecvSamplesFileTag > (XsampleRate * kFileConstant))
	{

	  XnumRecvSamplesFileTag = XnumRecvSamplesFileTag - (XsampleRate * kFileConstant) ;  /* Resets Global Event Variable to (0+XfftBinNumber) */

	  delete sp;

	  sp = new SignalProcessor(activeRF_mode); /* Create new instance of Noise spectrum Handler */

	  sp->InitializeOutFile(time(NULL)); /* Initialize new file with current time tag */
	}


      /*RESET INTEGRATION VARIABLE EVERY 60 SECONDS
       * -> CONDITION If number or received samples EXCEEDS 60s-threshold EXACTLY by the number of
       * acquired samples per cycle (XfftBinNumber)
       * -> integration_control flag is reset to XfftBinNumber
       * */
      if(XnumRecvSamplesIntegrationTag == (XsampleRate * 60) + XfftBinNumber) /*TODO: UPDATE-> Changed condition (+XfftBinNumber)*/
	{
	  /*UPDATE: 0 -> XfftBinNumber*/
	  XnumRecvSamplesIntegrationTag = XfftBinNumber;
	}



      /*CONDITION:
       * IF received sample number is SMALLER OR EQUAL than the number of samples expected in defined period T
       * -> load sampls into buffer */
      if(XnumRecvSamplesIntegrationTag <= (kDefaultIntegrationConstant * XsampleRate))
	{

	  dft_wrapper->GetRFSamples(temp_buff_rf);


	  /*Applies window function in TIME DOMAIN
	   * -> TODO: Consider applying Windowing as DEFAULT*/
	  if(result["w"].as<bool>())
	    {
	      dft_wrapper->Windowing();
	    }

	  /*-------------------------------------DISCRETE FOURIER TRANSFORM------------------------------------------*/


	  dft_wrapper->ComputeDFT();


	  sp->GetDFTData(dft_wrapper->ExportDFTResults());  /* Transfer of data from DFT Class to Signal Processor Class*/


	  /*CONDITION: If inorder-flag is NOT set, the
	   * DFT is stored DC-centered*/
	  if(!(result["inorder"].as<bool>()))
	    {
	      sp->RearrangeDFT();
	    }




	  sp->ComputePowerPeriodogram();  /* Gets Power from DFT samples (+ SUMMING POWER VALUES in Buffer for subsequent averaging)*/



	  /*TODO: Update to Condition:
	   * -> Now (==) instead of (>=)
	   */
	  if((XnumRecvSamplesIntegrationTag) == (kDefaultIntegrationConstant * XsampleRate))
	    {

	      /*Computes Average*/
	      sp->IntegratePWR();

	      /*Exports spectrum Data to file*/
	      sp->ExportRawDataToFile();


	      sp->ResetIntegrationBuffer();

	    }


	}

    }


  delete sp;		/*free memory*/
  delete dft_wrapper;  /*free memory*/
  delete usrp_wrapper; /*free memory*/
  delete temp_buff_rf; /*free memory*/

  return 0;
}



