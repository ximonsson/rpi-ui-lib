# Raspberry Pi UI Library

Helper library to create 2D user interfaces on Raspberry Pi.

In development...

Plan is to support creating widgets for text/image/video/shapes/etc... and then supporting Lua scripting
for creating graphical applications easier.

There are some basic stuff done around creating widgets and sizing/placing them correctly relative to
their parents, as well as color.

Video widget can be created as well, but because the mediaplayer library is not closing the
video that renders to texture correctly it can't be used too much. It is also a little problematic
that the video widget requires the main rendering loop to wait for the pthread condition to signal,
there would need some form of video_playing flag that tells us if we need to wait for the condition or not.
The whole thing feels prone for bugs.


## TODO
* Image decoding and rendering
* Text rendering (start with supporting opening bitmap fonts and make a simple render to texture buffer)
* Seeing about Lua support
* A lot more stuff
