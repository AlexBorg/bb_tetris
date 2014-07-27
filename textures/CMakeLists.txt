#add_custom_command((
#  OUTPUT block1.png block2.png block3.png block4.png block5.png block6.png
#  COMMAND gen_blocks.sh
#  DEPENDS gen_blocks.sh base.xcf)

#add_custom_command(
  #TARGET bbt PRE_BUILD
  #COMMAND gen_blocks.sh

file(GLOB TEXTURES *.png)
file(COPY ${TEXTURES} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
