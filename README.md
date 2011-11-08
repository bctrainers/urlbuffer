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

`!showlinks <#number>` 
in your irc client displays the `<#number>` last links buffered. If you don't give a `<#number` the default (`<#buffersize>`) will be used. Each link is sent with a 2 second delay to avoid flooding users.

Module commands:

 * `/msg *urlbuffer help` displays help.
 * `/msg *urlbuffer enable` activates link buffering.
 * `/msg *urlbuffer disable` deactivates link buffering.
 * `/msg *urlbuffer enablelocal` enables downloading of each link to local directory.
 * `/msg *urlbuffer disablelocal` disables downloading of each link to local directory.
 * `/msg *urlbuffer directory <#dir>` Sets the local directory where the links will be saved. (e.g. `/msg *urlbuffer directory /home/.znc/moddata/urlbuffer/`) Make sure the directory exists and that the znc user has write privileges.
 * `/msg *urlbuffer clearbuffer` Empties the link buffer.
 * `/msg *urlbuffer buffersize <#size>` Sets the size of the link buffer. Only integers >=0.
 * `/msg *urlbuffer showsettings` Displays all the current settings.

