#!/bin/bash

n_blocks=7

# Convert GIMP file to png -- imagemagick's support for this is poor
xcf2png base.xcf -o base.png

# Generate six evenly color shifted blocks from the base image
for x in `seq 1 $n_blocks`; do
  convert -flatten \
    -resize 20x20 \
    -modulate 100,100,$((200 / $n_blocks * (x - 1))) \
    base.png block$x.png
done

rm base.png
