# walkmap

tool for generating "walkmaps", or a group of bounding boxes indicating walkable space/height in .world files.

## Why use a walkmap?

The main advantage of using a generated walkmap over using the bounding boxes of the original objects to determine walkable space is that it reduces the amount of bounding boxes which need to be checked in a single frame to determine where the user is, by only generating bounding boxes for space which can be reached by the player and storing which bounding boxes are adjacent to each other.

## Building

At the moment all that is available is a Makefile created to be executed in MSYS, but because this doesn't rely on any libraries it should cross compile easily.  The targets are:

`make` (builds walkmap to ./bin/)

`make install` (builds walkmap to ./bin/ and then installs it to /usr/local/bin/)

`make clean` (deletes objects in ./bin/obj/ and then builds walkmap to ./bin/)

## Usage

The syntax on the command line is as follows:

`walkmap <path-to-world-file> <output-path> [playerHeight] [playerRadius] [stepHeight] [maxPlayerSpeed]`

This will output a walkmap file to output-path (note: output-path should include the filename) and a .world file representing the walkmap to output-path \+ .walkmap.  For example:

```
$ ls
example.world

$ walkmap ./example.world ./example.walkmap
Parsing .world file...
Generating walkmap...
 - Sorting objects by height...
 - Calculating walkable space...
 - Eliminating unreachable boxes...
Writing walkmap to file...
Done (finished in 0.01 seconds).

$ ls
example.walkmap  example.walkmap.world  example.world
```

Sadly all arguments have to be defined in that exact order, but I'll work on making a better argument parser when I have the time.

## Examples

### Scene:

![Example 1](/assets/images/example1.PNG)

### Walkmap:

![Example 1 Walkmap](/assets/images/example1walkmap.PNG)

### Scene:

![Example 2](/assets/images/example2.PNG)

### Walkmap:

![Example 2 Walkmap](/assets/images/example2walkmap.PNG)

### Scene:

![Example 3](/assets/images/example3.PNG)

### Walkmap:

![Example 3 Walkmap](/assets/images/example3walkmap.PNG)
