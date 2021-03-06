# IFF ImageReader

This is an image display program for the parsing and viewing of IFF images.

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

IFF, or Interchange File Format, is a datatype developed by Electronic Arts for use on the Commodore Amiga. It is a generic format that contains various subtypes, most common of which is the ILBM (InterLeaved BitMap). The container itself can actually hold any data, and as such, it has enjoyed broad support across all the 16/32-bit platforms and still remains in use today for niche roles.

In time, the viewer should gain the ability to support further formats under the aegis of the IFF meta-format, such as IFF animations, deep images, and possibly Genlock files.

## Why write an IFF reader?

The IFF ImageReader began as an exercise. I decided to take stock of my weak points as a programmer, and realized I'd never fully grokked how a moderately-complex container format worked. Given that IFF files figure prominently in my childhood, I decided to give it a go. Since then, it has turned into a surprisingly enjoyable project.

## There's already ILBM readers on GitHub. How's yours compared to them?

Darned if I know. I haven't actually done more than glance at them yet. They seem to be fairly C-specific: old school, idiomatic, and tightly bound to the low level implementations. 

This project has different aims. Modern C++ is all about applying clean semantics and judicious choice of algorithm to have the compiler generate efficient multiplatform code.  Thus, the intent here is to be as high-level as I reasonably can while still emphasizing performance. The benefit of this approach is that things like portability, multithreading, constness, memory safety and unit testing become practical while keeping resource usage low.

## Current state of project

### The Viewer

For the moment, the viewer operates on the command line. Usage is simple: you simply invoke IFF_Reader on an IFF ILBM file or a folder containing one or more IFF ILBM files:

`IFF_Reader.exe "path/to/file(s)"`

#### Keyboard shortcuts 

* When multiple files are open, navigating backwards and forwards is done using either arrow keys or space and backspace. 
* By pressing the I key, basic information on the image will be displayed on the console. 
* The colors of OCS images may be written incorrectly (this varies by drawing program). Pressing the O key toggles a correction algorithm, which will fix the issue, yielding slightly more vivid colors.
* To exit the viewer, press either Return or Escape, or close the window.

### Library

The library is capable of reading Amiga OCS (Original Chipset) and AGA (Advanced Graphics Architecture) images, including EHB (Extra HalfBrite) and HAM (Hold and Modify). It translates the data from planar format (Amiga-native memory layout) to Chunky format (VGA/SVGA) in order to do this. 

### Limitations

The library allows reading images, but does not yet write them. More obscure features are mostly unimplemented, and the list of planned features currently stands, in order, as follows: color cycling, a Linux build, support for IFF DEEP images, Interlace flicker, a fold-out UI bar, CRT screen simulation, scanlines, pixel shape correction, copper bar changes, Sliced EHB, Sliced HAM, sprite-based drawing, aspect correction, and support for other graphical formats within the IFF domain (ANIM, PICS, ACBM...). 

More broadly, the library still cannot serialize to IFF or PNG, or write files according to a user based schema (for representing data in more compact form in custom projects). This is certainly in the pipeline, but the schedule is to be determined.

The current build is Windows only. CMake would be the logical choice going forward to change that, but as I have yet to migrate from Visual Studio, which I use and prefer, this has yet to materialize. While I plan on getting an Ubuntu version up and running, I'm unmotivated to the task of wrangling dependencies for the olcPixelGameEngine on other Linux flavors, to say nothing of installing an OSX emulator to verify correctness on the Mac. As all externalities are confined to the entry points of the IFF class, though, getting a port to a stable state should be a matter of hours for any reasonably competent programmer.

## Dependencies

This viewer is self-contained and header-based. Future additions will also be statically compiled; the goal is to have a single binary. I may add provisions to build the IFF class as a library at some point.

The reader uses the excellent OlcPixelgameEngine for graphical display and keyboard control, and the equally excellent Lyra project for simple and idiomatic representation of command-line parsing. This author claims no ownership to any of these projects, and confines his use to what is permitted by their respective licenses. 

## Tangential questions

### What the heck is SciLab?

Oh. Well, as far as I can tell, it's GitHub trying to parse my test files and coming up short. Since those files are huge, I'm planning to replace the storage with a build step that would recreate them. There's obvious potential for trouble there, so I'll have to give it some more thought.
