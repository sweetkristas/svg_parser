# generate an image and compare for GoF

import os, sys
from subprocess import call

rootdir = r'c:\projects\svg_parser\icons'
root2   = r'c:\projects\svg_parser\images'

for subdir, dirs, files in os.walk(rootdir):
    for file in files:
        filename = subdir + r'\\' + file
        stem, ext = os.path.splitext(filename)
        if ext == '.svg':
            sys.stdout.write(filename + ' : ')
            call(['debug/svg_parser.exe', '--no-display', filename])
            png_file = stem + '.png'            
            png2_file = png_file.replace(r'icons', r'images').replace('svg\\', 'png')
            call(['compare', '-metric', 'RMSE', png_file, png2_file, 'diff.png'])
