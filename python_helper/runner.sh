#!/bin/bash
# script for running some tests
python3 word_freq_counter.py test1.txt
sleep 0.25
python3 word_freq_counter.py test2.txt
sleep 0.25
python3 word_freq_counter.py test3.txt
sleep 0.25
python3 word_freq_counter.py test4.txt
sleep 0.25
python3 word_freq_counter.py test5.txt
sleep 0.25
python3 jsd_check.py test1_freq.csv test2_freq.csv > 1_2_.txt
sleep 0.25
python3 jsd_check.py test1_freq.csv test3_freq.csv > 1_3_.txt
sleep 0.25
python3 jsd_check.py test1_freq.csv test4_freq.csv > 1_4_.txt
sleep 0.25
python3 jsd_check.py test1_freq.csv test5_freq.csv > 1_5_.txt
