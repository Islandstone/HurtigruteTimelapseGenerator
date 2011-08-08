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

#include "timelapsegenerator.h"
#include "pngwriter.h"
#include "videoframe.h"

#include <iostream>
#include <fstream>

CTimelapseGenerator::CTimelapseGenerator()
{
    m_flTotalDuration = 0.0f;
}

void CTimelapseGenerator::Init()
{
    // read the file list and add then to the vector
    ifstream filelist( "filelist.txt" );
    std::string line;

    if (filelist.is_open())
    {
        while ( filelist.good() )
        {
            getline(filelist,line);
            //printf_s("%s\n", line.c_str());

            std::string comment("//");
            if ( line.compare(0, comment.length(), comment) == 0 )
            {
                // Do not add this, it was a comment
                continue;
            }
            comment = "#";
            if ( line.compare(0, comment.length(), comment) == 0 )
            {
                // Do not add this, it was a comment
                continue;
            }

            if (line.length() == 0)
            {
                continue;
            }

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
        }
        catch (logic_error*)
        {
            // Something happened, don't add it
            printf_s("ERROR: Failed to read %s, file was not added to the queue", m_VideoFiles[i].c_str() );
            continue;
        }

        m_Decoders.push_back( decoder );
    }
}

void CTimelapseGenerator::Run()
{
    for (unsigned int i = 0; i < m_Decoders.size(); i++)
    {   
        m_Decoders[i]->decodeVideoFrame();
        double duration = m_Decoders[i]->getDuration();
        m_flTotalDuration += duration;
    }

    printf_s("Total duration: %f seconds\n", m_flTotalDuration);

    // the result is supposed to last for 9 minutes and 30 sec times 25 fps
    double delta = m_flTotalDuration /(25 * ((9 * 60) + 30));
    printf_s("Frame delta: %f seconds\n", delta);


    double currentGlobalTime = 0.0f;
    double currentFileStartTime = 0.0f;
    double nextFrameGlobalTime = 0.0f;

    int framenumber = 0;

    for (unsigned int i = 0; i < m_Decoders.size(); i++)
    {
        printf_s("Current file: %s\n", m_VideoFiles[i].c_str() );

        VideoFrame frame;

        double currentDuration = m_Decoders[i]->getDuration();
        double currentFileEndTime = currentFileStartTime + currentDuration;

        printf_s("Duration: %f - Global start time: %f - Global end time: %f\n", currentDuration, currentFileStartTime, currentFileEndTime);

        while ( nextFrameGlobalTime <= currentFileEndTime && (currentFileEndTime - nextFrameGlobalTime) > 1.0f )
        {
            framenumber += 1;

            double seekTime = nextFrameGlobalTime - currentFileStartTime;
            int64_t timestamp = int64_t(seekTime * AV_TIME_BASE);

            printf_s("Frame #%05d - Global time: %f - File Time: %f\n", framenumber, nextFrameGlobalTime, seekTime);

            // seek to the frame, and decode it
            m_Decoders[i]->seek_ex(timestamp);

            try
            {
                m_Decoders[i]->decodeVideoFrame();
            }
            catch (logic_error e)
            {
                //This usually fails at the end of files, skip 10 seconds ahead and move into the next file
                nextFrameGlobalTime += 10.0f;
                framenumber -= 1; // Subtract to get the frame numbers right
                continue;
            }

            // store it in an object
            m_Decoders[i]->getScaledVideoFrame(0, true, frame);

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
            PngWriter writer(szFileName);
            writer.writeFrame( &(rowPointers.front()), frame.width, frame.height, 8 );

            // global time for the next frame
            nextFrameGlobalTime += delta;
        }
        
        // set up for the next file
        // start of next = end of current
        currentFileStartTime = currentFileEndTime;
    }
}