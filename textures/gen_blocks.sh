#!/bin/bash

# Generate digit textures
for x in `seq 0 9`; do
  xcf2png digits.xcf -o digit$x.png Background $x
  mogrify -resize 20x40 digit$x.png
done;

# Generate top bar
xcf2png background.xcf -o background.png
mogrify -resize 280x480 background.png

xcf2png block.xcf -o block_bg.png     bg glow
xcf2png block.xcf -o block_inner.png  bg glow inner
xcf2png block.xcf -o block_outer.png  bg glow outer
xcf2png block.xcf -o block_top.png    bg glow top
xcf2png block.xcf -o block_bottom.png bg glow bottom
xcf2png block.xcf -o block_left.png   bg glow left
xcf2png block.xcf -o block_right.png  bg glow right

xcf2png messages.xcf -o paused.png    bg paused
xcf2png messages.xcf -o game_over.png bg game_over
