import csv
import os
import sys

import numpy
from matplotlib import pyplot
from matplotlib.colors import LogNorm


# arguments:
# - mode (raw, csv)
# - path to data files
# - period over which data should be plotted in h

dMode = sys.argv[1]
dPath = sys.argv[2]

if len(sys.argv) > 3:
    dPeriod = sys.argv[3]


if dMode == 'csv':
    print("MODE: CSV ")
    csv_list = []
    # parses folder for usable csv files and add them to list
    for root, dirList, fileList in os.walk(dPath):
        for fName in fileList:
            # checks whether file is csv file (string check)
            if fName.__contains__('.csv'):
                #print(fName, '\n')
                csv_list.append(fName)



        #----------------------------------#
    if dPeriod == '2' :
        if not dPath.endswith('.csv'):
            print('ERROR: in SINGLE-FILE-mode please specify FILE instead of DIRECTORY')
            exit()
        else:




            ##########################################################################################################################################
            with open(dPath) as datafile:
                csv_reader = csv.reader(datafile, delimiter=',')
                #makes list from iterator
                data = list(csv_reader)


                RF_array = numpy.zeros((len(data[0]), len(data)))


                it = 0
                jt = 0
                for i in data:

                    jt = 0
                    for j in i:
                        RF_array[jt][it] = float(data[it][jt])
                        jt += 1
                    it+=1
                    #filling matrix


        pyplot.pcolor(RF_array, norm=LogNorm(vmin=1e-20, vmax=RF_array.max()))

        pyplot.colorbar()

        pyplot.show()



    ###########################################################################################################################

    # 24 data mode
    elif dPeriod == '24':

        if dPath.endswith('.csv'):
            print('ERROR: in 24h-mode please specify DIRECTORY instead of FILE')
            exit()
        else:

            #check if 12 new (24hours) files
            if csv_list.__len__() < 13:
                print('ERROR: not enough data available... Coming back later')
                exit()

                #FILENAME PARSER

                #sort by days (AND PUT INTO NEW DIRECTORY)
            else:


                # TODO: putting while loop here


                files_sorted_date = [[], []]
                rf_data_dict = {}


                #sorts filenames [[day_info][time_info]]
                for i in csv_list:
                    print(i)
                    day_string = i.split('@')[1].split('_')[0]
                    # if not files_sorted_date[0].__contains__(day_string):
                    #     files_sorted_date[0].append(day_string)
                    time_string = i.split('@')[1].split('_')[1].split('.')[0]
                    time_string = time_string.lstrip('T')
                    # if not files_sorted_date[1].__contains__(time_string) :
                    #     files_sorted_date[1].append(time_string)
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


                # add missing leading zeros
                for keys in rf_data_dict:
                    for elems in rf_data_dict[keys]:
                        if len(elems) == 5:
                            rf_data_dict[keys][rf_data_dict[keys].index(elems)] = '0' + elems


                for idx in range(rf_data_dict.__len__() - 1 ):

                    #TODO: Fai attenzione
                    idx_list = rf_data_dict[files_sorted_date[0][idx]]

                    next_list = list(rf_data_dict[files_sorted_date[0][idx+1]])

                    #set early bird
                    if idx == 0:
                        early_bird_raw = idx_list[0]
                        early_bird = file_suffix + '@'+files_sorted_date[0][idx] +'_T'+ early_bird_raw+ '.csv'

                    if next_list.__contains__(early_bird_raw):




                        # TODO: Optimize in order to not move ALL csv-files to new folder
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



                        # concenate data from files and plot
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



                        pyplot.figure(idx) #create new figure

                        pyplot.pcolormesh( RF_array, cmap='rainbow', norm=LogNorm( vmin=1E-20, vmax=RF_array.max() ) )


                        #TODO: Add Information (data\e, axis time, etc. ) to plots

                        ax=pyplot.gca()  # Get axes from figure

                        yticks = ax.get_yticks()

                        xticks = ax.get_xticks()

                        majorticklocs_y = ax.yaxis.get_majorticklocs()


                        #TODO: remove!
                        #ax.yaxis.set_major_locator(pyplot.MaxNLocator(6))
                        #ax.set_yticklabels( ['8Mhz',  '11Mhz', '14Mhz', '17Mhz', '20Mhz'] )


                        if file_suffix.__contains__('Standardband'):
                            y_range = (0., 625., 1250.)
                            ax.set_yticks(y_range)
                            ax.set_yticklabels(['7.75', '14', '20.25'])
                            majorticklocs_y = ax.yaxis.get_majorticklocs()
                            pyplot.xlabel('time (min)')
                            pyplot.ylabel('Frequency (Mhz)')

                            pyplot.title('RF Map - Svalbard (24h overview)\n' +  files_sorted_date[0][idx] +
                                     '_' + early_bird_raw + ' - ' +
                                      files_sorted_date[0][idx+1] +
                                      '_' + early_bird_raw
                                      )


                        #mode for alternate bands
                        else:
                            fft_bins = len(data[0])
                            freq_band = []
                            freq_band = file_suffix.split('_')
                            freq_band[0] = freq_band[0].split('.')[0]
                            freq_band[1] = freq_band[1].split( '.' )[0]
                            nyquist_band = freq_band[1]-freq_band[0]
                            diff = fft_bins-nyquist_band
                            freq_overhead = diff/2

                            y_range = (0., fft_bins/2, fft_bins)
                            ax.set_yticks( y_range )
                            ax.set_yticklabels( [str(freq_band[0] - freq_overhead), str(freq_band[0] + nyquist_band/2) , str(freq_band[1] + freq_overhead)] )
                            pyplot.xlabel( 'time (min)' )
                            pyplot.ylabel('Frequency (khz)')


                        pyplot.colorbar()
                        #pyplot.show()
                        #TODO: try different dpi-settings
                        pyplot.savefig(file_string+"/RF_Map.png", dpi= 1600)

                        #clear figure
                        pyplot.clf()





