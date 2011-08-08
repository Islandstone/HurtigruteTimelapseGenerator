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

#ifdef __cplusplus
#define __STDC_CONSTANT_MACROS
#include <stdint.h>
#endif

#include "generator_thread.h"
#include "pngwriter.h"
#include "videoframe.h"

#include <iostream>
#include <fstream>

Mutex g_Mutex(L"host");

GeneratorHost* GeneratorHost::host = NULL;

GeneratorHost::GeneratorHost()
{
    m_flTotalDuration = 0.0f;
    m_flDelta = 0.0f;
    m_flCurrentDuration = 0.0f;
    m_flCurrentFileStartTime = 0.0f;
    m_flCurrentFileEndTime = 0.0f;
    m_flNextFrameGlobalTime = 0.0f;
    m_iFramenumber = 0;
    m_iVideoId = 0;
}

// -----------------------------------------------------------------------------
// Reads the file list over video files to be processed, calculates the numbers  
// required to create the timelapse
// -----------------------------------------------------------------------------
void GeneratorHost::Init()
{
    // read the file list and add then to the vector
    ifstream filelist( "filelist.txt" );
    std::string line;

    if (filelist.is_open())
    {
        while ( filelist.good() )
        {
            getline(filelist,line);

            std::string comment("//");
            if ( line.compare(0, comment.length(), comment) == 0 )
            {
                // Do not add this line, it was a comment
                continue;
            }
            comment = "#";
            if ( line.compare(0, comment.length(), comment) == 0 )
            {
                // Do not add this line, it was a comment
                continue;
            }

            if (line.length() == 0)
            {
                continue;
            }

            printf_s("Added file %s\n", line.c_str());
            m_VideoFiles.push_back(line);
        }
        filelist.close();
    }

    for (unsigned int i = 0; i < m_VideoFiles.size(); i++)
    {       
        MovieDecoder *decoder = NULL;

        try
        {
            decoder = new MovieDecoder( m_VideoFiles[i] );
            decoder->decodeVideoFrame();
            double duration = decoder->getDuration();
            m_flTotalDuration += duration;
        }
        catch (logic_error*)
        {
            // Something happened, don't add it
            printf_s("ERROR: Failed to read %s, file was not added to the queue\n", m_VideoFiles[i].c_str() );
            continue;
        }

        m_Decoders.push_back( decoder );
    }

    printf_s("Total duration of video files combined: %f seconds\n", m_flTotalDuration);

    // the result is supposed to last for 10 minutes times 25 fps
    m_flDelta = m_flTotalDuration / (25 * 10 * 60);

    printf_s("Frame delta: %f seconds\n", m_flDelta);

    m_flCurrentDuration = m_Decoders[0]->getDuration();
    
    m_flCurrentFileStartTime = 0.0f;
    m_flCurrentFileEndTime = m_flCurrentDuration;

    m_flNextFrameGlobalTime = 0.0f;
    m_iFramenumber = 1;
    m_iVideoId = 0;
}

// -----------------------------------------------------------------------------
// Get the next frame that a thread can process
// -----------------------------------------------------------------------------
bool GeneratorHost::GetNextFrame(int *video_id, int *framenumber, double *time)
{
    if ( m_flNextFrameGlobalTime <= m_flCurrentFileEndTime )
    {
        // We're still within the same file
        *video_id = m_iVideoId;
        *framenumber = m_iFramenumber;
        *time = (m_flNextFrameGlobalTime - m_flCurrentFileStartTime);
        
        return true;
    }
    else
    {
        // We've passed the end of the file, move into the next one
        m_iVideoId += 1;

        if ( m_iVideoId < m_Decoders.size() )
        {
            m_flCurrentFileStartTime = m_flCurrentFileEndTime;
            m_flCurrentFileEndTime = m_flCurrentFileStartTime + m_Decoders[m_iVideoId]->getDuration();

            *video_id = m_iVideoId;
            *framenumber = m_iFramenumber;
            *time = (m_flNextFrameGlobalTime - m_flCurrentFileStartTime);

            printf_s("Now processing file: %s\n", m_VideoFiles[m_iVideoId].c_str() );

            return true;
        }
        else
        {
            // We've exhausted all files, terminate the thread
            return false;
        }
    }
}

// -----------------------------------------------------------------------------
// Shift the current frame to the next file
// -----------------------------------------------------------------------------
void GeneratorHost::ShiftFrameToNext()
{
    // This is needed if the current frame is too close to the end, 
    // so just shift it by 3 seconds to move into the next file
    m_flNextFrameGlobalTime = m_flCurrentFileEndTime + 3.0f;
}

// -----------------------------------------------------------------------------
// Called by a thread to notify the host that it accepts and starts 
// processing the current frame 
// -----------------------------------------------------------------------------
void GeneratorHost::AcceptFrame()
{
    m_iFramenumber += 1;
    m_flNextFrameGlobalTime += m_flDelta;
}

// -----------------------------------------------------------------------------
// Initialize and start all threads
// -----------------------------------------------------------------------------
void GeneratorHost::Run()
{
    // Create the threads
    for (int i = 0; i < NUMBER_OF_THREADS; i++)
    {
        Thread *t = new GeneratorThread(i+1, "Thread");
        m_threads.push_back(t);
    }

    // Start the threads
    try
    {
        for (int i = 0; i < NUMBER_OF_THREADS; i++)
        {
            Thread *t = m_threads[i];

            if (t != NULL)
            {
                t->start();
            }
        }
    }
    catch(ThreadException ex) 
    {
        printf_s("%s\n",ex.getMessage().c_str());
        return;
    }

    // When we reach this point, all the threads are done, just delete them
    for (int i = 0; i < NUMBER_OF_THREADS; i++)
    {
        Thread *t = m_threads[i];

        if (t != NULL)
        {
            delete t;
            m_threads[i] = NULL;
        }
    }

    m_threads.clear();
}

// -----------------------------------------------------------------------------
// The main fuction of the thread, takes care of processing and saving the frame
// to a png file
// -----------------------------------------------------------------------------
void GeneratorThread::run()
{
    VideoFrame frame;

    while (true)
    {    
        // Get and accept new frame
        int id;
        int framenumber;
        double seekTime;

        while (true)
        {
            // Mutex host
            Thread::wait(L"host");

            if (!GeneratorHost::Get()->GetNextFrame(&id, &framenumber, &seekTime) )
            {
                Thread::release(L"host");
                return;
            }

            if (id != m_iCurrentId)
            {
                m_iCurrentId = id;

                if (m_decoder != NULL)
                {
                    delete m_decoder;
                    m_decoder = NULL;
                }

                m_strCurrentVideo = GeneratorHost::Get()->m_VideoFiles[m_iCurrentId];
                m_decoder = new MovieDecoder(m_strCurrentVideo);

                printf_s("%i: Now processing %s\n", m_iThreadId, m_strCurrentVideo.c_str() );
            }

            int64_t timestamp = int64_t(seekTime * AV_TIME_BASE);

            try
            {
                m_decoder->seek_ex(timestamp);
                m_decoder->decodeVideoFrame();
            }
            catch (logic_error e)
            {
                printf_s("Exception: %s\n", e.what());

                GeneratorHost::Get()->ShiftFrameToNext();
                Thread::release(L"host");
                continue;
            }

            GeneratorHost::Get()->AcceptFrame();
            printf_s("%i: #%05d - Time: %.2f\n", m_iThreadId, framenumber, seekTime);
            
            break;
        }

        // End mutex host
        Thread::release(L"host");

        // Process it
        m_decoder->getScaledVideoFrame(0, true, frame);

        // save the frame to a file
        vector<uint8_t*> rowPointers;
        for (int i = 0; i < frame.height; ++i)
        {
            rowPointers.push_back(&(frame.frameData[i * frame.lineSize]));
        }

        // generate the file name
        char szFileName[64];
        _snprintf_s(szFileName, sizeof(szFileName), "gather/frame-%05d.png", framenumber);

        // write the file
        try 
        {
            PngWriter writer(szFileName);
            writer.writeFrame( &(rowPointers.front()), frame.width, frame.height, 8 );
        }
        catch (logic_error e)
        {
            printf_s("%s\n", e.what());
        }
    }
}