#!/bin/bash 
 
# script file for BASH 
# which bash
# save this file as j.sh
# chmod +x j.sh
# ./j.sh
# checked in https://www.shellcheck.net/




printf "make pgm files \n"
gcc d.c -lm -Wall -march=native 

if [ $? -ne 0 ]
then
    echo ERROR: compilation failed !!!!!!
    exit 1
fi



printf "run the compiled program\n"
time ./a.out 

export  OMP_DISPLAY_ENV="FALSE"

printf "change Image Magic settings\n"
export MAGICK_WIDTH_LIMIT=100MP
export MAGICK_HEIGHT_LIMIT=100MP

printf "convert all pgm files to png using Image Magic v 6 convert \n"
# for all pgm files in this directory
for file in *.pgm ; do
  # b is name of file without extension
  b=$(basename "$file" .pgm)
  # convert  using ImageMagic
  convert "${b}".pgm -resize 600x600 "${b}".png
  # convert "${b}".pgm "${b}".png
  echo "$file"
done


printf "convert all ppm files to png using Image Magic v 6 convert \n"
# for all pgm files in this directory
for file in *.ppm ; do
  # b is name of file without extension
  b=$(basename "$file" .ppm)
  # convert  using ImageMagic
    convert "${b}".ppm -resize 600x600 "${b}".png
  # convert "${b}".ppm "${b}".png
  echo "$file"
done


printf "delete all pgm and ppm files \n"
rm ./*.pgm
rm ./*.ppm





 
echo OK

printf "info about software \n"
bash --version
make -v
gcc --version
convert -version
convert -list resource
# end

