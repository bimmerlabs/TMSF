**Tilemap / spritesheet loader for VDP1**

A packed VDP1 format for sprites - designed for use with SRL https://github.com/ReyeMe/SaturnRingLib

This is very much a WIP.  it works but is not complete and the tooling is not finished either.


Supports:

-multiple sprites / animations per file

-animations

-RLE compression


Goals:

-4bpp, RGB support (currently only works on palleted sprites - you must load the pallet separately)

-different compression algorithms that work better with digitized sprites

-specified frame rate per animation

-multiple sprite sizes per animation


Currently Missing:

-tools to generate the tmsf format.  They exist, just not in a form suitable for public consumption.  But it would be possible to make your own based on the heade data.


Instructions:  

Clone repository and copy tmsf folder to the modules_extra folder in SRL:

<img width="202" height="53" alt="image" src="https://github.com/user-attachments/assets/40bc5757-a13a-466b-811f-6afbe85565b9" />

(further instructions needed)
