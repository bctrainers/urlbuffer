#Urlbuffer

This znc module listens to traffic on irc channels and if an image link is detected it downloads the image to a given directory and reuploads it to imgur.com.

##Dependencies

The module needs wget, sed, curl, a unix enviroment and a directory with write access to the znc user to work.

##Usage

Compile it with 
`znc-buildmod urlbuffer.cpp` 
and move the .so file to the appropriate znc directory (usually /usr/lib/znc/).

The module takes no arguments.
Read [loading modules](http://wiki.znc.in/Modules#.28Un.29Loading_Modules) to learn more about loading modules. 

##Commands 

todo

