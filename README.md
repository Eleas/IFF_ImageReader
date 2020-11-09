# IFF ImageReader

Being a viewer for IFF ILBM images.

## What's an IFF image?

IFF, or Interchange File Format, is a datatype developed by Electronic Arts for use on the Commodore Amiga. Its most common usage was as an image format, but it is a container that can actually hold anything, and as such, it enjoyed broad support across all the 16/32-bit platforms and still remains in very niche roles. It is easily parsable, explicable, and well-formed. The full name of the image format is actually IFF ILBM (meaning InterLeaved BitMap), but in time, this viewer aims to support further formats under the aegis of the IFF meta-format. 

## Why write an IFF reader?

It began as an exercise in self-improvement. I decided to take stock of my weak points as a programmer, and realized I'd never fully grokked how a moderately-complex container format worked. Given that IFF files figure prominently in my childhood, I decided to give it a go. So far, no regrets.

## There's already ILBM readers on GitHub. How's yours compared to them?

Darned if I know. I haven't actually done more than glance at them yet. They seem to be fairly C-specific: old school, idiomatic, and tightly bound to the low level implementations. This repository aims to take a different route.
