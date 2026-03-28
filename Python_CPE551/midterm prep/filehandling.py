import pandas as pd
import math
import pickle
import pytest

predef_set = {"ID_01", "ID_05", "ID_09"}

def save_dict(dict):
    with open("save_dir.txt", "wb") as outFile:
        pickle.dump(dict, outFile)


def file_handing(path):
    extracted_data = []
    try:
        with open(path, "r") as file:
            for line in file:
                extracted_data.append(line.strip().split(';'))
        return extracted_data
    except FileNotFoundError as e:
        raise Exception(f"Error : {e}")
    except:
        raise Exception(f"An error has been encountered while processing file")
    
def extract_data(path):
    L = file_handing(path)
    s = set([L[i][1] for i in range(len(L))])
    intersect = s.intersection(predef_set)
    d = {f"ID_{n}": [int(elm) for elm in int(L[n][2])] for n in range(len(L))}
    save_dict(d)


def extract_data_with_pandas(path):
    df = pd.read_csv(path)
    # Vectorized operation: apply math to the whole column at once
    df['multiply'] = df['multiply'].apply(lambda x: x * math.pi)
    # Filter: Show rows where 'multiply' > 50
    filtered_df = df[df['multiply'] > 50]
    print(filtered_df)



def add(a, b):
    return a + b

@pytest.mark.xfail
def test_invalid_add():
    res = add(5, "WRONG")
    pass

