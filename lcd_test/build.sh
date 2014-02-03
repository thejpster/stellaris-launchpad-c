gcc -o test -Isrc src/drivers/lcd/src/lcd_sim.c src/drivers/lcd/src/lcd_test.c src/font/src/font.c src/font/src/BigFont.c src/font/src/SevenSeg_XXXL_Num.c -std=c99 -DLCD_ROTATE_DISPLAY
