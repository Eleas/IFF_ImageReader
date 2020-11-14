# IFF ImageReader

This is an image display program for the parsing of IFF images.

## What's an IFF image?

IFF, or Interchange File Format, is a datatype developed by Electronic Arts for use on the Commodore Amiga. Its most common usage was as an image format, but it is a container that can actually hold anything, and as such, it enjoyed broad support across all the 16/32-bit platforms and still remains in very niche roles. It is easily parsable, explicable, and well-formed. The full name of the image format is actually IFF ILBM (meaning InterLeaved BitMap), but in time, this viewer aims to support further formats under the aegis of the IFF meta-format. 

## Why write an IFF reader?

It began as an exercise in self-improvement. I decided to take stock of my weak points as a programmer, and realized I'd never fully grokked how a moderately-complex container format worked. Given that IFF files figure prominently in my childhood, I decided to give it a go. So far, no regrets.

## There's already ILBM readers on GitHub. How's yours compared to them?

Darned if I know. I haven't actually done more than glance at them yet. They seem to be fairly C-specific: old school, idiomatic, and tightly bound to the low level implementations. This repository aims to take a different route.

## Current state of project

### Viewer

The viewer is currently capable of opening single files (via CLI) or multiple files (by supplying the folder path). Arrow keys or space allows switching between them.

### Library

The library is capable of reading Amiga OCS (Original Chipset) images, including EHB (Extra HalfBrite) and HAM (Hold and Modify), as well as AGA (Advanced Graphics Architecture) regular images. It translates the data from planar (Amiga-native memory layout) form to Chunky form (VGA/SVGA) in order to do this. 

### Limitations

The library allows reading images, but does not yet write them. More obscure features are mostly unimplemented, and the list of planned features currently stands, in order, as follows: OCS color correction, parallel file loading, EHB/HAM correction, support for IFF DEEP images, color cycling, HAM8, Interlace flicker, a fold-out UI bar, CRT screen simulation, scanlines, pixel shape correction, copper bar changes, Sliced HAM, sprite-based drawing, aspect correction, and support for other graphical formats within the IFF domain (ANIM, PICS, ACBM...). The lack of facilities for storing the images as PNG or similar is also notable, as is the ability to write files according to a user based schema (for representing data in more compact form in custom projects), but the schedule for adding those features is not yet set in stone.

## Dependencies

This project is self-contained. The reader uses the excellent OlcPixelgameEngine for graphical display and keyboard control, and the equally excellent Lyra project for simple and idiomatic representation of command-line parsing. This author claims no ownership to any of these projects, and is happy to use them in accordance with their respective licenses. 
