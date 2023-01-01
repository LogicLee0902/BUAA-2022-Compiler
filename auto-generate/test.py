import os

input_path = "./Exam"

for file in os.listdir(input_path):
    if file.endswith(".txt"):
        print(file.split(".")[0])

exe_path = r".\Compiler.exe"

for file in os.listdir(input_path):
    if file.endswith(".txt") and "testfile" in file:
        command = "echo " + file + " | " + exe_path
        print(command)
        filename = os.path.join(input_path, file.split(".")[0])
        os.system("echo " + filename + " | " + exe_path)

