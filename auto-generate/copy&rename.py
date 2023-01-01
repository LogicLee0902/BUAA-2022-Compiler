# -*- coding: utf-8 -*-

import os
import shutil

origin_path = "./Exam/"
destination_path = "./result/"

for file in os.listdir(origin_path):
    if "_" in file:
        filename = file.split("_")
        if len(filename) ==2:
            print(file)
            shutil.move(origin_path+file, destination_path+file.split(".")[0]+"_your-name_优化后中间代码.txt")
        elif len(filename) == 3:
            shutil.move(origin_path+file, destination_path+file.split(".")[0]+"your-name_优化后目标代码.txt")
