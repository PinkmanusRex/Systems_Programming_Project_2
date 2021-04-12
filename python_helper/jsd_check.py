import sys
import math
import re
import pandas as pd
import numpy as np

def jsd_checker(_df1, _df2, _f1, _f2):
    word_set = set()
    for word in _df1["word"].unique():
        word_set.add(word)
    for word in _df2["word"].unique():
        word_set.add(word)
    _df1["mean_freq"] = 0.0
    _df2["mean_freq"] = 0.0
    for word in word_set:
        if word in _df1["word"].values and word in _df2["word"].values:
            mean_freq = (0.5)*(_df1.loc[_df1["word"]==word, "word_freq"].values[0] + _df2.loc[_df2["word"]==word, "word_freq"].values[0])
            _df1.loc[_df1["word"]==word, "mean_freq"] = mean_freq
            _df2.loc[_df2["word"]==word, "mean_freq"] = mean_freq
        elif word in _df1["word"].values:
            mean_freq = (0.5)*(_df1.loc[_df1["word"]==word, "word_freq"].values[0])
            _df1.loc[_df1["word"]==word, "mean_freq"] = mean_freq
        else:
            mean_freq = (0.5)*(_df2.loc[_df2["word"]==word, "word_freq"].values[0])
            _df2.loc[_df2["word"]==word, "mean_freq"] = mean_freq
    _kld1 = 0
    print(f"{_f1} computing")
    for _, row in _df1.iterrows():
        _kld1 += row["word_freq"] * math.log((row["word_freq"]/row["mean_freq"]), 2)
        print(f"{row.word}: {row.word_freq}/{row.mean_freq}")
    _kld2 = 0
    print()
    print(f"{_f2} computing")
    for _, row in _df2.iterrows():
        _kld2 += row["word_freq"] * math.log((row["word_freq"]/row["mean_freq"]), 2)
        print(f"{row.word}: {row.word_freq}/{row.mean_freq}")
    print()
    _jsd = math.sqrt((0.5)*_kld1 + (0.5)*_kld2)
    return _kld1, _kld2, _jsd
def col_csv(_f1, _f2):
    _df1 = pd.read_csv(_f1)
    _df2 = pd.read_csv(_f2)
    _kld1, _kld2, _jsd = jsd_checker(_df1, _df2, _f1, _f2)
    print(f"kld1: {_kld1}, kld2: {_kld2}\njsd: {_jsd}")

col_csv(sys.argv[1], sys.argv[2])
