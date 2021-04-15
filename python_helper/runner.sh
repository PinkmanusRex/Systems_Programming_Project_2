#!/bin/bash
# script for running some tests
#word_freq_counter will output a <file_name>_freq.csv that can be input to jsd_check.py. jsd_check.py requires 2 csv files as arguments.
python3 word_freq_counter.py test1.txt
python3 word_freq_counter.py test2.txt
python3 word_freq_counter.py test3.txt
python3 word_freq_counter.py test4.txt
python3 word_freq_counter.py test5.txt
python3 jsd_check.py test1_freq.csv test2_freq.csv > 1_2_.txt
python3 jsd_check.py test1_freq.csv test3_freq.csv > 1_3_.txt
python3 jsd_check.py test1_freq.csv test4_freq.csv > 1_4_.txt
python3 jsd_check.py test1_freq.csv test5_freq.csv > 1_5_.txt
