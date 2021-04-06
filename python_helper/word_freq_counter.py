from collections import defaultdict
import sys
import re

def count_freq(file_in, file_out):
    total_words = 0
    count_dict = defaultdict(int)
    f_in = open(file_in, "r")
    f_out = open(file_out, "w")
    f_out.write(file_in + "\n")
    for line in f_in:
        mod_line = re.sub(r"[^-a-zA-Z0-9\s]", "", line)
        mod_line = mod_line.lower()
        f_out.write(mod_line)
        words = mod_line.split()
        for word in words:
            count_dict[word] += 1
            total_words += 1
    keys = count_dict.keys()
    keys = sorted(keys)
    for key in keys:
        freq = count_dict[key]/total_words
        f_out.write(f"{key}: {count_dict[key]}, {freq}\n")
    f_out.close()

count_freq(sys.argv[1], sys.argv[2])
