#!/usr/bin/python
# Copyright (c) 2020 Krzysztof Gabis
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.

import argparse
import os

def process_input(input, output_path):
    if os.path.exists(output_path) and os.path.isfile(output_path):
        raise Exception(output_path + " is a file")
    if not os.path.exists(output_path):
        os.makedirs(output_path)

    file_path = ""
    file_contents = ""
    for line in input:
        if line.startswith("#line 1 \""):
            file_name = line.split("\"")[1].strip()
            #file_name = file_name[:-1]
            file_path = os.path.join(output_path, file_name)
            file_contents = ""
        elif line == "#line 0\n":
            print("Writing: " + file_path)
            with open(file_path, "w") as output:
                output.write(file_contents)
        else:
            file_contents += line

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--input", required=True)
    parser.add_argument("--output-path", required=True)

    args = parser.parse_args()

    with open(args.input, "r") as f:
        process_input(f.readlines(), args.output_path)

if __name__ == "__main__":
    main()