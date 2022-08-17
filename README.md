# walkmap

tool for generating "walkmaps", or a group of bounding boxes indicating walkable space/height in .world files.

# Why use a walkmap?

The main advantage of using a generated walkmap over using the bounding boxes of the original objects to determine walkable space is that it reduces the amount of bounding boxes which need to be checked in a single frame to determine where the user is, by only generating bounding boxes for space which can be reached by the player and storing adjacent bounding boxes per bounding box.