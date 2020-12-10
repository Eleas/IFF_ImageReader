# IFF ImageReader

This is an image display program for the parsing of IFF images. It also contains a rudimentary viewer.

## In brief

<dl>
  <dt>Author</dt> <dd>Björn Paulsen</dd>
  <dt>Type</dt> <dd>Image viewer and parsing library</dd>
  <dt>Language</dt> <dd>C++</dd>
  <dt>Dependencies</dt> <dd>olcPixelGameEngine, Lyra</dd>
  <dt>Binary size</dt> <dd>186 kB (x64 version)</dd>
  <dt>File types supported</dt> <dd>regular OCS and AGA, EHB, HAM6, HAM8</dd>
</dl>

## What's an IFF image?

IFF, or Interchange File Format, is a datatype developed by Electronic Arts for use on the Commodore Amiga. Its most common usage was as an image format, but it is a container that can actually hold anything, and as such, it enjoyed broad support across all the 16/32-bit platforms and still remains in very niche roles. It is easily parsable, explicable, and well-formed. The full name of the image format is actually IFF ILBM (meaning InterLeaved BitMap); in time, this viewer aims to support further formats under the aegis of the IFF meta-format, including IFF animations, deep images, and possibly Genlock files.

## Why write an IFF reader?

It began as an exercise in self-improvement. I decided to take stock of my weak points as a programmer, and realized I'd never fully grokked how a moderately-complex container format worked. Given that IFF files figure prominently in my childhood, I decided to give it a go, and it turned out to be surprisingly enjoyable.

## There's already ILBM readers on GitHub. How's yours compared to them?

Darned if I know. I haven't actually done more than glance at them yet. They seem to be fairly C-specific: old school, idiomatic, and tightly bound to the low level implementations. This repository aims to take a different route: the intent here is to be as high-level as I reasonably can while still emphasizing performance. 

## Current state of project

### Usage

In its current iteration, the viewer operates on the command line. Executing it simply means calling IFF_Reader on an IFF ILBM file, or on a folder containing one or more IFF ILBM files:

`IFF_Reader.exe "path/to/file(s)"`

If multiple files, arrow keys will allow you to navigate backwards and forwards. To exit, press either Return or Escape.

### Viewer

The viewer is currently capable of opening single files (via CLI) or multiple files (by supplying the folder path). Arrow keys or space allows switching between them.

### Library

The library is capable of reading Amiga OCS (Original Chipset) and AGA (Advanced Graphics Architecture) images, including EHB (Extra HalfBrite) and HAM (Hold and Modify). It translates the data from planar (Amiga-native memory layout) form to Chunky form (VGA/SVGA) in order to do this. 

### Limitations

The library allows reading images, but does not yet write them. More obscure features are mostly unimplemented, and the list of planned features currently stands, in order, as follows: color cycling, a Linux build, support for IFF DEEP images, Interlace flicker, a fold-out UI bar, CRT screen simulation, scanlines, pixel shape correction, copper bar changes, Sliced EHB, Sliced HAM, sprite-based drawing, aspect correction, and support for other graphical formats within the IFF domain (ANIM, PICS, ACBM...). 

More broadly, the library still cannot serialize to IFF or PNG, or write files according to a user based schema (for representing data in more compact form in custom projects). This is certainly in the pipeline, but the schedule is to be determined.

At present, the build is Windows only. CMake would be the logical choice going forward to change that, but as I have yet to migrate from Visual Studio, which I use and prefer, While I plan on getting an Ubuntu version up and running, I'm unmotivated to the task of reworking the olcPixelGameEngine to get it performant on most common Linux flavors, let alone install an OSX emulator to verify correctness on the Mac. As all externalities are confined to the entry points, however, getting a port to a stable state should be a matter of hours for any reasonably competent programmer.

## Dependencies

This project is self-contained. The reader uses the excellent OlcPixelgameEngine for graphical display and keyboard control, and the equally excellent Lyra project for simple and idiomatic representation of command-line parsing. This author claims no ownership to any of these projects, and confines his use to what is permitted by their respective licenses. 

## Tangential questions

### What the heck is SciLab?

Oh. Well, as far as I can tell, it's GitHub trying to parse my test files and coming up short. Since those files are huge, I'm planning to replace the storage with a build step that would recreate them. There's obvious potential for trouble there, so I'll have to give it some more thought.
