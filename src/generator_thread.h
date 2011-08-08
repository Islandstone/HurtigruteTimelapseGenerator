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

#ifndef GENERATOR_THREAD_H
#define GENERATOR_THREAD_H

// Change this to define the number of threads to create
#define NUMBER_OF_THREADS 4

#include <string>
#include <vector>
#include "moviedecoder.h"

using namespace std;
#include "ou_thread.h"
using namespace openutils;

class GeneratorThread;

// Host object for all video processing threads
class GeneratorHost
{
public:
    // Constructor
    GeneratorHost();

    // Singleton pattern
    static GeneratorHost* Get()
    {
        if (host == NULL)
        {
            host = new GeneratorHost();
        }

        return host;
    }

    void    Init();     // Set up all the information needed for processing
    void    Run();      // Create and start the threads

    // Called by a thread to receive information about next frame
    bool    GetNextFrame(int *video_id, int *framenumber, double *time);

    // Shifts the current frame over to the next file. Used for cases where  
    // a frame is too close to the end of the file to be decoded
    void    ShiftFrameToNext();

    // Called whenever the current thread has found a frame to process
    void    AcceptFrame();

    // Contains all references to the video files,  
    // video id's refer to the indices of this vector
    vector<string> m_VideoFiles;

private:

    static GeneratorHost* host;         // Singleton object

    double  m_flDelta;                  // Number of seconds between each frame
    double  m_flTotalDuration;          // The duration in seconds of all files combined

    double  m_flCurrentDuration;        // Duration of current file
    double  m_flCurrentFileStartTime;   // Number of seconds from the beginning of the first file to the beginning of the current file
    double  m_flCurrentFileEndTime;     // Number of seconds from the beginning of the first file to the end of the current file

    double  m_flNextFrameGlobalTime;    // Number of seconds from the beginning of the first file to the next frame

    int     m_iFramenumber;             // The frame number for next frame to be processed
    int     m_iVideoId;                 // The ID of the video currently being processed

    vector<MovieDecoder*> m_Decoders;   // Decoder objects for each of the video files
    vector<Thread*> m_threads;          // Container for all of the video processing threads

    // Undefined and unaccessible
    GeneratorHost( const GeneratorHost &);
};

// Thread object for video processing
class GeneratorThread : public Thread
{
public:
    GeneratorThread(int id, const char *name)
    {
        m_iThreadId = id;

        Thread::setName(name);
        m_decoder = NULL;
        m_iCurrentId = -1;
    }

    void run();

private:

    int m_iThreadId;            // ID of the thread
    string m_strCurrentVideo;   // File path to the current video being processed
    int m_iCurrentId;           // ID of the current video being processed

    MovieDecoder *m_decoder;    // Pointer to a decoder object for the current video
};

#endif // GENERATOR_THREAD_H