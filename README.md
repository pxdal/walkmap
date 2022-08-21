# walkmap

tool for generating "walkmaps", or a group of bounding boxes indicating walkable space/height in .world files.

## Why use a walkmap?

The main advantage of using a generated walkmap over using the bounding boxes of the original objects to determine walkable space is that it reduces the amount of bounding boxes which need to be checked in a single frame to determine where the user is, by only generating bounding boxes for space which can be reached by the player and storing adjacent bounding boxes per bounding box.

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
