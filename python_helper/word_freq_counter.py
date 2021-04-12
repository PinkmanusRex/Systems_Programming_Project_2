from collections import defaultdict
import sys
import re

def count_freq(file_in):
    total_words = 0
    count_dict = defaultdict(int)
    f_in = open(file_in, "r")
    file_name = re.match(r"(.*).txt", file_in)[1]
    f_clean_word = open(file_name + "_cleaned.txt", "w");
    f_clean_word.write(file_in + "\n")
    f_freq = open(file_name + "_freq.csv", "w")
    f_freq.write("word,word_count,word_freq\n")
    for line in f_in:
        mod_line = re.sub(r"[^-a-zA-Z0-9\s]", "", line)
        mod_line = mod_line.lower()
        f_clean_word.write(mod_line)
        words = mod_line.split()
        for word in words:
            count_dict[word] += 1
            total_words += 1
    f_clean_word.close()
    keys = count_dict.keys()
    keys = sorted(keys)
    for key in keys:
        freq = count_dict[key]/total_words
        f_freq.write(f"{key},{count_dict[key]},{freq}\n")
    f_freq.close()

count_freq(sys.argv[1])
