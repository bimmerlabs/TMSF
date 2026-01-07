tilemap / spritesheet loader for VDP1

packed VDP1 format for sprites - designed for use with SRL https://github.com/ReyeMe/SaturnRingLib

supports:
-multiple sprites / animations per file
-animations
-RLE compression

Goals:
-4bpp, RGB support (currently only works on palleted sprites - you must load the pallet separately)
-different compression algorithms that work better with digitized sprites
-specified frame rate
-multiple sprite sizes per animation
