C-unformatter
=============
Remove unnecessary whitespaces and optionally comments in your C source code.

In most cases it turns your source code into a one-liner, excluding the
preprocessor instructions. 

Installation
------------
To install, type in:

    make
    sudo make install

Uninstallation
--------------
To uninstall, type in:

    sudo make uninstall

Usage
-----

    Usage:  unformatter [options...]
    Remove unnecessary white-spaces and optionally comments in your
    C source code.

    Options
            -h              show this help text and exit
            -c              keep the comments.
            -i              the input source file
            -o              theoutput source file

    This program will use the standard input/output, if you don't
    specify an input and/or an output file.

    Note: C++ style comments will force a line break.

Copyright
---------
C-unformatter -- Remove unnecessary whitespaces and optionally comments in your
C source code.

Copyright (C) 2013  Fufu Fang

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see {http://www.gnu.org/licenses/}.
