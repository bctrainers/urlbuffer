#Urlbuffer

This znc module listens to traffic on irc channels and scans for image links (or just links). If an image link is detected it downloads the image to a given directory and reuploads it to imgur.com. Also if bufferalllinks is enabled then it will append each non image link found to a links.txt file in the current day directory. 

##Dependencies

The module needs wget, sed, curl, a unix enviroment and a directory with write access for the znc user, to work.

##Usage

Compile it with 
`znc-buildmod urlbuffer.cpp` 
and move the .so file to the appropriate znc directory (usually /usr/lib/znc/ or ~/.znc/modules).

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
 * `/msg *urlbuffer enablepublic` enables public usage of !showlinks by any user.
 * `/msg *urlbuffer disablepublic` disables public usage of !showlinks by any user.
 * `/msg *urlbuffer directory <#dir>` Sets the local directory where the links will be saved. (e.g. `/msg *urlbuffer directory /home/username/.znc/moddata/urlbuffer/`) Make sure the directory exists and that the znc user has write privileges.
 * `/msg *urlbuffer clearbuffer` empties the link buffer.
 * `/msg *urlbuffer buffersize <#size>` sets the size of the link buffer. Only integers >=0.
 * `/msg *urlbuffer showsettings` displays all the current settings.
 * `/msg *urlbuffer showlinks <#number>` displays the `<#number>` last links.
 * `/msg *urlbuffer bufferalllinks` toggles between the buffering of all links and only images.
 * `/msg *urlbuffer reupload` toggles reuploading of images to imgur.
