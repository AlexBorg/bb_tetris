#!/bin/bash

n_blocks=7

# Convert GIMP file to png -- imagemagick's support for this is poor
xcf2png blocks.xcf -o blocks.png

# Generate six evenly color shifted blocks from the base image
for x in `seq 1 $n_blocks`; do
  convert -flatten \
    -resize 20x20 \
    -modulate 100,100,$((200 / $n_blocks * (x - 1))) \
    blocks.png block$x.png
done
rm blocks.png

# Generate digit textures
for x in `seq 0 9`; do
  xcf2png digits.xcf -o digit$x.png Background $x
  mogrify -resize 20x40 digit$x.png
done;

# Generate top bar
xcf2png top_bar.xcf -o top_bar.png
mogrify -resize 560x80 top_bar.png
