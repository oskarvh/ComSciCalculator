# Written by Oskar von Heideken
# Copyright, 2023

# Script to convert EVE Asset Builder font converter output files
# to something usable by the comSciCalc firmware
import json
import os
import datetime

# Set this if you want user readable .h files. 
USE_RAWH = True

with open('h_header.txt') as f: header = f.read()
with open('h_footer.txt') as f: footer = f.read()

# Get a list of all files in the input directory
listdir = os.listdir("input_files/")

# Go through the list and collect all json files
jsonListTemp = []
for f in listdir:
    if ".json" in f:
        jsonListTemp.append(f)

correlated_files = []
# Go through all json files and find the correlated raw file
for f in jsonListTemp:
    filename = f.replace(".json", "")
    correlated_raw_filename = ""
    for l in listdir:
        if "rawh" in l:
            # Skip the rawh files
            continue
        if filename in l:
            correlated_raw_filename = l
    if correlated_raw_filename == "":
        print("Could not find a raw file named" + filename)
    else:
        correlated_files.append(
            {
                "json_file" : f,
                "raw_file" : correlated_raw_filename
            }
        )

# Now that we have the correlated json and raw files, 
# go through each pair and construct the output file
for correlated_file in correlated_files:
    jsonfile = correlated_file["json_file"]
    rawfile = correlated_file["raw_file"]
    print(rawfile)
    print("Converting " + jsonfile)

    # Open and parse the json file
    f = open("input_files/" + jsonfile)
    jsondata = json.load(f)
    f.close()
    # Extract the metadata from the json file:
    fontname = jsondata["name"]
    font_format = jsondata["format"]
    font_format = font_format[1:]
    font_size = jsondata["size"]
    font_baseline = jsondata["base_line"]
    font_caps_height = jsondata["caps_height"]
    font_x_width = jsondata["x_width"]
    converted_chars = jsondata["converted_chars"]

    # Construct the .h file that the data goes in to
    # Replace the <filename>, <FILENAME> and <DATE>
    # with the case sensetive filename and date. 
    
    h_header = header.replace("<filename>", fontname)
    h_header = h_header.replace("<FILENAME>", fontname.upper())
    date = datetime.datetime.now()
    h_header = h_header.replace("<DATE>", date.strftime("%Y-%m-%d %H:%M:%S"))
    
    h_footer = footer
    h_footer = h_footer.replace("<filename>", fontname)
    h_footer = h_footer.replace("<FILENAME>", fontname.upper())
    h_footer = h_footer.replace("<font_format>", str(font_format))
    h_footer = h_footer.replace("<font_size>", str(font_size))
    h_footer = h_footer.replace("<font_baseline>", str(font_baseline))
    h_footer = h_footer.replace("<font_caps_height>", str(font_caps_height))
    h_footer = h_footer.replace("<font_x_width>", str(font_x_width))
    
    # Open the .raw file
    with open("input_files/" + rawfile, mode='rb') as f: data = f.read()
    numbytes = len(data)
    h_header = h_header.replace("<NUMBYTES>", str(numbytes))
    h_footer = h_footer.replace("<NUMBYTES>", str(numbytes))

    if(USE_RAWH == True):
        with open("input_files/" + rawfile + "h") as f: data = f.read()
        h_header = h_header + data + "\n"
    else:
        h_header += "{\n"
        # Convert binary data to a list instead
        bytedata =[]
        for byte in data:
            bytedata.append(byte)

        for byte in bytedata:
            h_header = h_header + "    " +  str(byte) + ",\n"
        h_header += "}"
    h_header += ";\n"

    # Remove the last comma
    #h_header = h_header[:-2]

    # Finally, append the footer and write to the new file
    h_header = h_header + "\n" + h_footer
    with open("output_files/" + fontname + ".h", mode='w') as f: f.write(h_header)
    