henge
=====

Overview
--------
Henge is an attempt to create a simple yet functional OpenGL 3D engine. It's
designed around the concept that you may use parts of it which might be useful
for your particular program, and ignore most of the other parts when they're not
needed.

Care is taken to leave OpenGL state intact in utility functions that aren't
explicitly supposed to affect the state, and dependencies between parts are kept
to a minimum. Ideally no two unrelated parts of the engine should assume the
other one is in use and rely on its state wherever possible.

The focus of the library is simplicity and ease of use, rather than performance.
This means that most design decisions gravitate towards my subjective idea of a
clean API, avoiding extreme optimizations that would compromise simplicity and
clarity.

.. image:: http://nuclear.mutantstargoat.com/sw/henge/img/henge_terrain_thumb.jpg
.. image:: http://nuclear.mutantstargoat.com/sw/henge/img/sea_shot_thumb.jpg
.. image:: http://nuclear.mutantstargoat.com/sw/henge/img/dunged_thumb.jpg

Download
--------
 * Web site: http://nuclear.mutantstargoat.com/sw/henge
 * code repository: https://github.com/jtsiomb/henge

License
-------

Author: John Tsiombikas <nuclear@member.fsf.org>

Henge is free software. You may use, modify, and redistribute it under the terms
of the GNU Lesser General Public License (LGPL) version 3, or at your option,
any later version published by the Free Software Foundation. See COPYING_ and
COPYING.LESSER_ for details.

Installation
------------
To compile and install (default prefix is /usr/local) just go into the henge
directory and type::

 $ ./configure
 $ make
 # make install

Usage
-----
To use henge in your projects make sure you pass the output of
``pkg-config --cflags henge2`` to the compiler, and the output of
``pkg-config --libs henge2`` to the linker.

There's no documentation at the moment, try to figure out how to use henge by
looking through the header files, and the example programs in the tests
subdirectory.

.. _COPYING: http://www.gnu.org/licenses/gpl
.. _COPYING.LESSER: http://www.gnu.org/licenses/lgpl
