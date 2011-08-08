// =============================================================================
//
//    Copyright (c) 2011 Øystein Dale <dale.oystein@gmail.com>
//
// =============================================================================
//
//    This file is part of the Hurtigruten Timelapse Generator.
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// =============================================================================

#define __STDC_CONSTANT_MACROS

// With this you can change between the threaded and the non-threaded version.
// You can change the number of threads to create in the file generator_thread.h
#define USE_THREADS


#ifdef USE_THREADS
    #include "generator_thread.h"
#else
    #include "timelapsegenerator.h"
#endif

int main(int argc, const char * argv[])
{
#ifdef USE_THREADS
    GeneratorHost::Get()->Init();
    GeneratorHost::Get()->Run();
#else
    CTimelapseGenerator generator;

    generator.Init();
    generator.Run();
#endif
    return 0;
}
