########################################################################
# This script is part of the Svalbard RF Map project (UNIS)
#
#
#
# Author: Florian Anderl
# Date: 10.10.2018
#
#########################################################################


import csv
import os
import sys
import math

import numpy
from matplotlib import pyplot
from matplotlib.colors import LogNorm
from matplotlib import ticker
from itertools import  cycle
from tkinter import filedialog
from tkinter import *

########################################################################
# README - Instructions - Manual - Anleitung
#
#   - This script processes RF Recording Data in form of csv-files
#   - It creates 24 hour plots of recorded data
#   - A single csv file comprises 2 hours of data -> But in order to work, the script needs AT LEAST 13 successive data files (NOT 12)
#   - (DO NEVER DELETE ANY CSV FILES )
#   - The script needs you to specify a root-directory, it will parse this directory AND ALL SUBDIRECTORIES for csv files and will process alll of the found files
#   - if there are csv files among the found files, THE SCRIPT WILL CRASH
#
#   - if you want SuperDARN channel markers in your plots: manually change SWITCH_CHANNEL_MARKERS to 'True'
#
#   -> The script will create directories in the specified path and move csv files that build one data set (12 files [NOT 13]) to the respective folder
#   -> The script will display a manipulatable plot for every data set + AN AVERAGE PLOT (naturally an average over all data sets)
#   -> Save from me
#
#   IMPORTANT: THE SCRIPT WILL NOT WORK IF THE 2H FILES ARE NOT EXACTLY !!!! SPACED 2 HOURS APART
#    -> This has to be improved
#   IMPORTANT: THIS SCRIPT WILL ALSO NOT WORK IF DATA FROM TWO DIFFERENT RECORDING SESSIONS ARE IN THE SPECIFIED PATH
#
#
#
# NOTES:
#
#
#
##########################################################################


SWITCH_CHANNEL_MARKERS = True




TIME_STRING = ' UTC'

# Select Directory
root = Tk()
root.directory =  filedialog.askdirectory(initialdir = "/home/unis/UNIS/report",title = "Select directory")
print (root.directory)

dPath = root.directory

dPath = dPath+"/"


avg_count = 0
dAvg = False

csv_list = []
# parses folder for usable csv files and add them to list
for root, dirList, fileList in os.walk(dPath):
    for fName in fileList:
        # checks whether file is csv file (string check)
        if fName.__contains__('.csv'):
            #print(fName, '\n')
            csv_list.append(fName)







#NOTE: check if  at least 12 new (24hours) files
if csv_list.__len__() < 13:
    print('ERROR: not enough data available... ')
    exit()

    #---------------------------FILENAME PARSER---------------------------------


else:


    # Sort Data Files for further data processing
    # TODO: add compatibility for files that are not EXACTLY 2 hours apart

    # FIXME: Make files_sorted_date one dimensional
    files_sorted_date = [[],[]]
    rf_data_dict = {}


    #sorts filenames [[day_info][time_info]]
    print("Found:\n")
    for i in csv_list:
        print(i)
        day_string = i.split('@')[1].split('_')[0]
        # if not files_sorted_date[0].__contains__(day_string):
        #     files_sorted_date[0].append(day_string)
        time_string = i.split('@')[1].split('_')[1].split('.')[0]
        time_string = time_string.lstrip('T')
        # if not files_sorted_date[0][1].__contains__(time_string) :
        #     files_sorted_date[0][1].append(time_string)
        if not rf_data_dict.__contains__(day_string):
            rf_data_dict.setdefault(day_string, [])

        if not rf_data_dict[day_string].__contains__(time_string) or time_string[1:]:
            rf_data_dict.setdefault(day_string, []).append(time_string)
            rf_data_dict[day_string] = list(map(int, rf_data_dict[day_string]))
            rf_data_dict[day_string].sort()
            rf_data_dict[day_string] = list(map(str, rf_data_dict[day_string] ) )


    file_suffix = i.split('@')[0]

    # Sort dictionary wrt keys
    # Add day do date list
    for keys in rf_data_dict.keys():
        files_sorted_date[0].append(keys)




    # Sorts date strings in ascending order
    files_sorted_date[0] = list(map(int, files_sorted_date[0]))
    files_sorted_date[0].sort()
    files_sorted_date[0] = list(map(str, files_sorted_date[0]))


    # add missing leading zeros that were sliced in previous steps
    for keys in rf_data_dict:
        for elems in rf_data_dict[keys]:
            if len(elems) == 5:
                rf_data_dict[keys][rf_data_dict[keys].index(elems)] = '0' + elems
            if len(elems) == 4:
                rf_data_dict[keys][rf_data_dict[keys].index( elems )] = '00' + elems



    avg_RF_array = []
    avg_lim = 0

    # determine number of full data sets before actual loop containing functionality
    # FIXME: UGLY HACK
    for idx in range(rf_data_dict.__len__() - 1 ):

        # day x
        idx_list = rf_data_dict[files_sorted_date[0][idx]]

        # day x+1
        next_list = list(rf_data_dict[files_sorted_date[0][idx+1]])

        #set early bird (start time of script)
        if idx == 0:
            early_bird_raw = idx_list[0]
            early_bird = file_suffix + '@'+files_sorted_date[0][idx] +'_T'+ early_bird_raw+ '.csv'



        if next_list.__contains__(early_bird_raw):


            avg_lim = avg_lim + 1


    # NOTE: range -1, because access with idx+1
    # Excludes LAST DAY
    for idx in range(rf_data_dict.__len__() - 1 ):

        # day x
        idx_list = rf_data_dict[files_sorted_date[0][idx]]

        # day x+1
        next_list = list(rf_data_dict[files_sorted_date[0][idx+1]])

        #set early bird (start time of script)
        if idx == 0:
            early_bird_raw = idx_list[0]
            early_bird = file_suffix + '@'+files_sorted_date[0][idx] +'_T'+ early_bird_raw+ '.csv'

        #If 12 hour data set is complete (13 set files have to be present !!!) DO
        # FIXME: NOT ELEGANT

        #NOTE: Condition - if next day contains early_bird
        # Condition is proper.... for now
        # Does not calculate last data set if 'early bird' for last day is missing, even though enough files are there
        # BUG

        if next_list.__contains__(early_bird_raw):




            file_string = dPath + files_sorted_date[0][idx] + "_" + files_sorted_date[0][idx+1]

            os.system('if ! [ -d '+ file_string +' ];then mkdir '+ file_string +  '\nfi')

            for index in range(len(idx_list)):
                idx_list[index] = file_suffix+"@"+files_sorted_date[0][idx]+'_T'+idx_list[index]+'.csv'

            for index in range(len(next_list)):
                next_list[index] = file_suffix +"@"+ files_sorted_date[0][idx+1] +'_T'+ next_list[index] + '.csv'




            set_list = idx_list[list(idx_list).index (file_suffix + '@'+files_sorted_date[0][idx] +'_T'+ early_bird_raw+ '.csv'):] \
                       + next_list[:(next_list.index(file_suffix + '@' +files_sorted_date[0][idx+1] +'_T'+ early_bird_raw+ '.csv'))]

            # Moves file sets to corresponding folders

            for jdx in range(len(set_list)):
                current_csv = set_list[jdx]
                bash_arg = 'mv   ' + "\"" + dPath + current_csv + "\"" +'    ' + file_string
                os.system(bash_arg)



            # Concenate data from 24 hours files and plot
            data = []
            for i in set_list:
                with open(file_string + '/' + i) as datafile:
                    csv_reader = csv.reader(datafile, delimiter=',')

                    data = data + list(csv_reader)

            RF_array = numpy.zeros( (len( data[0] ), len( data )) )



            it = 0
            for i in data:
                jt = 0
                for j in i:
                    RF_array[jt][it] = float( data[it][jt] )
                    jt += 1
                it += 1


            # NOTE: Numerical Averaging is done here


            if avg_count == 0:
                avg_RF_array = numpy.zeros((len( data[0] ), len( data )))
            avg_RF_array = numpy.add(avg_RF_array, RF_array)
            avg_count += 1


            if avg_count == avg_lim:
                dAvg = True
                avg_RF_array = avg_RF_array / avg_count







            ############### PLOTTING ####################### PLOTTING ######################## PLOTTING #################### PLOTTING #######################

            fft_bins = len( data[0] )
            freq_band = []

            if not file_suffix.__contains__('Standardband'):
                # Miscellaneous Information and Parameters

                freq_band = file_suffix.split( '_' )
                freq_band[0] = freq_band[0].split( '.' )[0]
                freq_band[1] = freq_band[1].split( '.' )[0]
                nyquist_band = int( freq_band[1] ) - int( freq_band[0] )

                ###################################################################################################################
                # NOTE: Ugly hack to get frequency resolution -> Sampling Frequency is UNKNOWN to this script
                # FIXME: THAT IS OBVIOUSLY NOT OPTIMAL -> FIX FIX FIX
                if nyquist_band <= 2500:
                    sampling_rate = 2500000
                elif nyquist_band > 2500 and nyquist_band < 5000:
                    sampling_rate = 5000000
                elif nyquist_band >= 5000 and nyquist_band <= 8000:
                    sampling_rate = 6250000
                elif nyquist_band > 8000 and nyquist_band <= 10000:
                    sampling_rate = 10000000
                elif nyquist_band > 10000 and nyquist_band < 12500:
                    sampling_rate = 12500000
                else:
                    sampling_rate = 25000000

            else:
                nyquist_band = 12000 # in kHz
                freq_band =[8000,20000] # in kHz
                sampling_rate = 12500000 # in Hz


            freq_res = sampling_rate / fft_bins

            diff = sampling_rate / 1e3 - nyquist_band
            freq_overhead = (diff / 2)

            real_freq_band = [0.0, 0.0]
            real_freq_band[0] = int( freq_band[0] ) - freq_overhead
            real_freq_band[1] = int( freq_band[1] ) + freq_overhead

            # NOTE: Draw lines in plot
            # (1) Create Mapping from data number to frequency information
            # using fft_bins  AND freq_band[]
            # TODO: move to upper position and USE, USE, USE

            SuperDARN_channel_list = [
                8050,
                9400,
                9600,
                9800,
                11600,
                11800,
                12000,
                13570,
                13770,
                15100,
                15350,
                15700,
                17480,
                17600,
                17800,
                18900
            ]

            # NORMAL PLOTTING -------------------------------------------------------------

            pyplot.figure(idx + 1)  # create new figure

            pyplot.pcolormesh(RF_array, cmap='jet', norm=LogNorm(vmin=RF_array.min(), vmax=RF_array.max() ) )

            ax = pyplot.gca()  # Get axes from figure

            yticks = ax.get_yticks()

            xticks = ax.get_xticks()

            majorticklocs_y = ax.yaxis.get_majorticklocs()

            x_range = [0., 120., 240., 360., 480., 600., 720., 840., 960., 1080., 1200., 1320., 1440.]
            ax.set_xticks( x_range )
            majorticklocs_x = ax.xaxis.get_majorticklocs()
            ax.set_xticklabels( ['0', '2', '4', '6', '8', '10', '12', '14', '16', '18', '20', '22', '24'] )

            #TODO: remove!
            #ax.yaxis.set_major_locator(pyplot.MaxNLocator(6))
            #ax.set_yticklabels( ['8Mhz',  '11Mhz', '14Mhz', '17Mhz', '20Mhz'] )


            if file_suffix.__contains__('Standardband'):
                y_range = (0.,  225. , 425. ,  625.,  825. , 1025.  , 1250.)
                ax.set_yticks(y_range)
                ax.set_yticklabels(['7.75','10','12' ,'14','16', '18', '20.25'])
                majorticklocs_y = ax.yaxis.get_majorticklocs()
                pyplot.xlabel('time (h)')
                pyplot.ylabel('Frequency (Mhz)')

                pyplot.title('HF Power Plot 8-20 MHz - Svalbard (24h overview)\n' +  files_sorted_date[0][idx]  +
                             '_' + early_bird_raw + ' - ' +
                             files_sorted_date[0][idx+1] +
                             '_' + early_bird_raw
                             +TIME_STRING)

                if dAvg == True and avg_count == avg_lim:
                    pyplot.title( 'HF Power Plot 8-20 MHz - Svalbard - Average 24h \n' +
                                  files_sorted_date[0][idx] +
                                  '_' + early_bird_raw + ' - ' +
                                  files_sorted_date[0][avg_count] +
                                  '_' + early_bird_raw + TIME_STRING)

            #mode for alternate bands
            else:


                y_range = (freq_overhead * 1e3 / freq_res,
                           (0.25 * nyquist_band * 1e3) / freq_res + freq_overhead * 1e3 / freq_res,
                           fft_bins / 2,
                           freq_overhead * 1e3 / freq_res + (0.75 * nyquist_band * 1e3) / freq_res,
                           fft_bins - (freq_overhead * 1e3 / freq_res))
                ax.set_yticks( y_range )
                ax.set_yticklabels( [
                    str( int( freq_band[0] ) ),
                    str( int( freq_band[0] ) + nyquist_band / 4 ),
                    str( int( freq_band[0] ) + nyquist_band / 2 ),
                    str( int( freq_band[0] ) + 3 * nyquist_band / 4 ),
                    str( int( freq_band[1] ) )] )
                ######################################################################################################################

                pyplot.xlabel( 'time (h)' )
                pyplot.ylabel('Frequency (khz)')
                pyplot.title(
                    'HF Power Spectral Density:' + freq_band[0]+ '-' + freq_band[1] + 'kHz - Svalbard\n' + files_sorted_date[0][idx] +
                    '_' + early_bird_raw + ' - ' +
                    files_sorted_date[0][idx + 1] +
                    '_' + early_bird_raw + TIME_STRING
                )
                # TODO: Remove that, if it proves to be unnecessary
                """
                if dAvg == True and avg_count == avg_lim:
                    pyplot.title(
                        'HF Power Spectral Density:' + freq_band[0] + '-' + freq_band[
                            1] + 'kHz - Svalbard - Average 24h\n' + files_sorted_date[0][idx] +
                        '_' + early_bird_raw + ' - ' +
                        files_sorted_date[0][avg_count-1] +
                        '_' + early_bird_raw
                    )
                """

            # Get Colorbar Handle & Change Ticks according to USRP CALIBRATION MEASUREMENTS
            color_bar = pyplot.colorbar( aspect=40)
            cb_ticks = color_bar.get_ticks()
            color_bar.set_label("Digital RF Power (dBFS)")
            # Make colorbar labels in dB
            label_list = []
            tick_locator = ticker.MaxNLocator(nbins = 6)
            cb_ticks = cb_ticks[::2]

            for i in range(len(cb_ticks)):
                label_list.append(str(round(10*math.log(cb_ticks[i], 10))))

            color_bar.set_ticks(cb_ticks)
            color_bar.set_ticklabels( label_list )




            if SWITCH_CHANNEL_MARKERS == True:




                # NOTE: determine frequency scope
                channel_scope = [0.0, 0.0]  # 2 entries
                for ch in SuperDARN_channel_list:
                    if ch >= int( freq_band[0] ) and channel_scope[0] == 0.0:
                        channel_scope[0] = ch

                    if ch >= int( freq_band[1] ) and channel_scope[1] == 0.0:
                        channel_scope[1] = SuperDARN_channel_list[SuperDARN_channel_list.index( ch ) - 1]

                if channel_scope[1] == 0.0:
                    channel_scope[1] = SuperDARN_channel_list[-1]

                # NOTE: create mapping for frequencies
                frequency_lookup_dict = {}

                for i in range( fft_bins ):
                    frequency_lookup_dict[real_freq_band[0] + i * freq_res / 1e3] = i + 0.5

                # TODO: Create Channel Markers in PLOTS with hlines
                # Create a colour code cycler e.g. 'C0', 'C1', etc.
                colour_codes = map( 'C{}'.format, cycle( range( 10 ) ) )
                for i in SuperDARN_channel_list[
                         SuperDARN_channel_list.index( channel_scope[0] ): SuperDARN_channel_list.index(
                                 channel_scope[1] )+1]:
                    # Get next colour code
                    colour_code = next( colour_codes )

                    close_enough = min( list( frequency_lookup_dict.keys() ), key=lambda x: abs( x - i ) )
                    close_enough_100 = min( list( frequency_lookup_dict.keys() ),
                                            key=lambda x: abs( x - (i + 100) ) )

                    # pyplot.hlines(frequency_lookup_dict[close_enough], 0, len(data)-100, lw = 3, colors = colour_code , linestyles='solid', label= str(close_enough))
                    # pyplot.hlines(frequency_lookup_dict[close_enough_100], 0,len(data)-100, lw = 3,colors = colour_code, linestyles='solid',
                    #             label= str(close_enough_100) )

                    # pyplot.axhline(frequency_lookup_dict[close_enough], 0, 0.7, lw = 3, color = colour_code , linestyle='solid', label= str(close_enough) )

                    pyplot.axhspan( frequency_lookup_dict[close_enough],
                                    frequency_lookup_dict[close_enough_100], 0, 1, lw=1,linestyle='dashed', fill=False,
                                    facecolor='saddlebrown',
                                    edgecolor='black',
                                    alpha=1 )

                    # NOTE: add ticks on axis
                    pyplot.text( 1450, frequency_lookup_dict[close_enough],
                                 'CH' + str( SuperDARN_channel_list.index( i )+1 ), fontstyle='italic' )



            pyplot.savefig(file_string+"/RF_Map.png", dpi = 1600)




            # NOTE: This routine is a really, really ugly hack since it basically repeats the same functionality as in
            # NOTE: non-average mode above but this is the only possible implementation
            # NOTE: .....if there is the time, pretty please fix that.... I am not proud...
            if dAvg == True:
                    pyplot.figure( idx + 2 )  # create new figure

                    pyplot.pcolormesh( avg_RF_array, cmap='jet',
                                       norm=LogNorm( vmin=RF_array.min(), vmax=RF_array.max() ) )

                    ax = pyplot.gca()  # Get axes from figure

                    yticks = ax.get_yticks()

                    xticks = ax.get_xticks()

                    majorticklocs_y = ax.yaxis.get_majorticklocs()

                    x_range = [0., 120., 240., 360., 480., 600., 720., 840., 960., 1080., 1200., 1320., 1440.]
                    ax.set_xticks( x_range )
                    majorticklocs_x = ax.xaxis.get_majorticklocs()
                    ax.set_xticklabels( ['0', '2', '4', '6', '8', '10', '12', '14', '16', '18', '20', '22', '24'] )

                    # TODO: remove!
                    # ax.yaxis.set_major_locator(pyplot.MaxNLocator(6))
                    # ax.set_yticklabels( ['8Mhz',  '11Mhz', '14Mhz', '17Mhz', '20Mhz'] )

                    if file_suffix.__contains__( 'Standardband' ):
                        y_range = (0., 225.,  625.,  1025., 1250.)
                        ax.set_yticks( y_range )
                        ax.set_yticklabels( ['7.75', '10',  '14' , '18', '20.25'] )
                        majorticklocs_y = ax.yaxis.get_majorticklocs()
                        pyplot.xlabel( 'time (h)' )
                        pyplot.ylabel( 'Frequency (Mhz)' )

                        pyplot.title( 'HF Power Plot 8-20 MHz - Svalbard (24h overview)\n' + files_sorted_date[0][idx] +
                                      '_' + early_bird_raw + ' - ' +
                                      files_sorted_date[0][idx + 1] +
                                      '_' + early_bird_raw
                                      + TIME_STRING)

                        if dAvg == True and avg_count == avg_lim:
                            pyplot.title( 'HF Power Plot 8-20 MHz - Svalbard - Average 24h \n' +
                                          files_sorted_date[0][idx] +
                                          '_' + early_bird_raw + ' - ' +
                                          files_sorted_date[0][avg_count] +
                                          '_' + early_bird_raw +TIME_STRING)

                    # mode for alternate bands
                    else:


                        y_range = (freq_overhead * 1e3 / freq_res,
                                   (0.25 * nyquist_band * 1e3) / freq_res + freq_overhead * 1e3 / freq_res,
                                   fft_bins / 2,
                                   freq_overhead * 1e3 / freq_res + (0.75 * nyquist_band * 1e3) / freq_res,
                                   fft_bins - (freq_overhead * 1e3 / freq_res))
                        ax.set_yticks( y_range )
                        ax.set_yticklabels( [
                            str( int( freq_band[0] ) ),
                            str( int( freq_band[0] ) + nyquist_band / 4 ),
                            str( int( freq_band[0] ) + nyquist_band / 2 ),
                            str( int( freq_band[0] ) + 3 * nyquist_band / 4 ),
                            str( int( freq_band[1] ) )] )
                        ######################################################################################################################

                        pyplot.xlabel( 'time (h)' )
                        pyplot.ylabel( 'Frequency (khz)' )
                        pyplot.title(
                            'HF Power Spectral Density:' + freq_band[0] + '-' + freq_band[1] + 'kHz - Svalbard\n' +
                            files_sorted_date[0][idx] +
                            '_' + early_bird_raw + ' - ' +
                            files_sorted_date[0][idx + 1] +
                            '_' + early_bird_raw +TIME_STRING
                        )
                        if dAvg == True and avg_count == avg_lim:
                            pyplot.title(
                                'HF Power Spectral Density:' + freq_band[0] + '-' + freq_band[
                                    1] + 'kHz - Svalbard - Average 24h\n' + files_sorted_date[0][idx] +
                                '_' + early_bird_raw + ' - ' +
                                files_sorted_date[0][avg_count - 1] +
                                '_' + early_bird_raw + TIME_STRING
                            )

                    # Get Colorbar Handle & Change Ticks according to USRP CALIBRATION MEASUREMENTS
                    color_bar = pyplot.colorbar(aspect=40)
                    cb_ticks = color_bar.get_ticks()
                    color_bar.set_label( "Digital RF Power (dBFS)" )
                    # Make colorbar labels in dB
                    label_list = []
                    tick_locator = ticker.MaxNLocator( nbins=6 )
                    cb_ticks = cb_ticks[::2]

                    for i in range( len( cb_ticks ) ):
                        label_list.append( str( round( 10 * math.log( cb_ticks[i], 10 ) ) ) )

                    color_bar.set_ticks( cb_ticks )
                    color_bar.set_ticklabels( label_list )

                    if SWITCH_CHANNEL_MARKERS == True:

                        fig = pyplot.figure(idx+2)



                        # NOTE: determine frequency scope
                        channel_scope = [0.0, 0.0]  # 2 entries
                        for ch in SuperDARN_channel_list:
                            if ch >= int( freq_band[0] ) and channel_scope[0] == 0.0:
                                channel_scope[0] = ch

                            if ch >= int( freq_band[1] ) and channel_scope[1] == 0.0:
                                channel_scope[1] = SuperDARN_channel_list[SuperDARN_channel_list.index( ch ) - 1]

                        if channel_scope[1] == 0.0:
                            channel_scope[1] = SuperDARN_channel_list[-1]

                        # NOTE: create mapping for frequencies
                        frequency_lookup_dict = {}

                        for i in range( fft_bins ):
                            frequency_lookup_dict[real_freq_band[0] + i * freq_res / 1e3] = i + 0.5

                        # TODO: Create Channel Markers in PLOTS with hlines
                        # Create a colour code cycler e.g. 'C0', 'C1', etc.
                        colour_codes = map( 'C{}'.format, cycle( range( 10 ) ) )
                        for i in SuperDARN_channel_list[
                                 SuperDARN_channel_list.index( channel_scope[0] ): SuperDARN_channel_list.index(
                                     channel_scope[1] )]:
                            # Get next colour code
                            colour_code = next( colour_codes )

                            close_enough = min( list( frequency_lookup_dict.keys() ), key=lambda x: abs( x - i ) )
                            close_enough_100 = min( list( frequency_lookup_dict.keys() ),
                                                    key=lambda x: abs( x - (i + 100) ) )

                            # pyplot.hlines(frequency_lookup_dict[close_enough], 0, len(data)-100, lw = 3, colors = colour_code , linestyles='solid', label= str(close_enough))
                            # pyplot.hlines(frequency_lookup_dict[close_enough_100], 0,len(data)-100, lw = 3,colors = colour_code, linestyles='solid',
                            #             label= str(close_enough_100) )

                            # pyplot.axhline(frequency_lookup_dict[close_enough], 0, 0.7, lw = 3, color = colour_code , linestyle='solid', label= str(close_enough) )

                            pyplot.axhspan( frequency_lookup_dict[close_enough],
                                            frequency_lookup_dict[close_enough_100], 0, 1, lw=1, linestyle='dashed',
                                            fill=False,
                                            facecolor='saddlebrown',
                                            edgecolor='black',
                                            alpha=1 )

                            # NOTE: add ticks on axis
                            pyplot.text( 1450, frequency_lookup_dict[close_enough],
                                         'CH' + str( SuperDARN_channel_list.index( i  ) + 1 ), fontstyle='italic' )

                    pyplot.savefig( dPath + "Average_RF_Plot.png", dpi=1600 )




    pyplot.show()


