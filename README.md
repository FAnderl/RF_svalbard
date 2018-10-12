# RF_svalbard 

This software was designed to automatize frequency band spectrum recordings with deployed USRP devices.
It was tested and put into use for its initial purpose: 
The mapping of the RF background in Svalbard in frequency bands related to the SuperDARN in Svalbard. The project was conducted in collaboration with UNIS (The University Center in Svalbard).  
 
The repository also contains a python script which processes the generated data and plots 24h power plots and average power plots 


INSTRUCTIONS - Recording Software

 The program takes as input: 
 
 - -l lower frequency of recorded band in Hz
  
 - -u upper frequency of recorded band in Hz
 
 - -a USRP address (192.168.10.2 is Default Address)
  
 - -g Specifies USRP gain in dB
  
 - -w If this flag is set, Blackman Window is applied prior to DFT 
  
 - -inorder If this flag is set, DFT output is shttps://www.ettus.com/sdr-software/detail/usrp-hardware-drivertored in-order
  
  For example:
  		
  		./RF_USRP_Recording -l 9000000 -u 13000000 -w
  		
  		for recordings between 9 and 13 MHz with applied windowing 
  
  
 The program generates:
  In the current configuration the software generates a CSV file every two hours, containing two hours 
  worth of data in the form of a power spectral densities within the specified frequency band. They are tagged
  in the following way:
  
  
  lowerFrequency_upperFrequencykHz@sampling_rate_in_Hz@yyyymmdd_Thhmmss
  
  
  Normally, the file tags should be EXACTLY 2 hours apart. 
  
  NOTE: If the specified band is rather broad (>= 15 MHz) the applied USRP sampling rate results in  
  occasional buffer overflows in the USRP and thus the loss of RF samples. This also means that the data files
  are note spaced EXACTLY 2 hours apart.  
  
  Important Parameters (Constants.h):
  
  - kDefaultFrequencyResolution -> minimal frequency resolution of DFT output
  
  - kMinDFTsize -> minimal number of DFT bins
  
  - kFileConstant -> time in seconds after which a new file is generated
  
  - kDefaultIntegrationConstant -> time in seconds defining how many seconds RF signal is integrated/
  averaged 
  
  
  
  
  
  REQUIREMENTS & LIBRARIES - Recording Software
  
  - Generic Comand Line Parser (header only)	
     -> https://github.com/jarro2783/cxxopts
     
  - UHD Driver
   		-> https://www.ettus.com/sdr-software/detail/usrp-hardware-driver
  		
  - C++ Boost Libraries in VERSION 1_65_1 (shipped in Ubuntu 18.04)  	
  
  
  - CMAKE 	
  
  
  HOW TO BUILD - Recording Software
  
  The building process of the c++ reocrding software is automatized with cmake: 
  
  $ mkdir build
  $ cd build
  $ cmake ..
  $ make
  
  
  The resulting executable links dynamically with UHD and Boost. Static Linking can be obtained by editing 
  CMakeLists.txt but is not recommended. 
  
  
   INSTRUCTIONS - Plotting Python Script 
   
   	This python script processes the generated data and automatically moves data files that belong to one  	
   	24 hour data set (12 files) into a new folder named (day1_lastday). It plots the data into 24 hour  
   	overview - power density color plots.  
   	
   	INPUT:
   		Path to data files 
   		
   	Parameters: 
   		SWITCH_CHANNEL_MARKERS (True, False) : If true, SuperDARN Svalbard channel markers are added to the 		plots
   				
   
   
   	NOTES: 
   		There are several conditions that have to be fullfilled for the script to work properly:
   		 (1) only data files of one recording session can be in the specified path AND the subdirectories
   		  (it will not work otherwise)
   		 (2) only data sets for which the FIRST immediate successive data file (12+1.) is present in the 	
   		 specified path are processed
   		 (3) The plots should be saved manually from the pyplot-plots for best quality; however 1600dpi-png 			are saved to the respective subfolders 
   		 
   COMPATIBILITY NOTES: 
   		Version 1.2 of the recording script changes the name conversion of the CSV-files; for data 	
   		files created by earlier versions use the python script version before the version that was 
   		first tagged with the compatibility note in the script header		 
   	 