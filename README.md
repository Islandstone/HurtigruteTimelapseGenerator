Hurtigruten Timelapse Generator
=============

Øystein Dale, 2011

First of all I wish to thank NRK and NRKBeta for the great show, and for making this possible by releasing the video footage under a CC license.

What is this?
---------

This is a project designed to efficiently create a timelapse out of the vast video material provided by NRK under a Creative Commons license from "Hurtigruten - Minutt for minutt". It is based on several other open source projects (listed below), but mostly custom software I've written myself. A copy of the licenses of the included work can be found in the licenses folder.

How does it work?
---------

The program starts by reading a list over all video files that are to be included in the processing, it reads and calculates the length of all the video files, and subsequently it also calculates the spacing between each frame in the final timelapse. Then it extracts each of these frames and saves them to a PNG file, which can be reassembled into a continuous video file by using FFMPEG.

Why not use any existing software for this?
---------

While using existing video editing software would seem like the easiest solution, it would be extremely time consuming to create a high-speed timelapse out of the almost 300 GB of 1920x1080 HD video using any existing software that I know of. I am aware that a similar timelapse has been made with conventional video editing software, but doing it that way cannot match the speed of my program by a mile (or lets say 2090 km). It also allows for greater precision when it comes to how long the final timelapse will be.

About that, how long does it take to generate a timelapse?
---------

The first stage, extracting all the images from the video files, takes about 1.5 hours. Reassembling the frames into a high-quality video takes around 2 hours. 

Why did you do this?
---------

You can consider this my thankyou note to NRK, I really appreciate what they did, and this is my way of saying thank you. Also, I enjoy writing code and thinking out solutions to problems like this project gave me; how to efficiently create a timelapse from 300 GB of video.

What do you use for this project?
---------

My program is based on a slightly modified ffmpegthumbnailer (http://code.google.com/p/ffmpegthumbnailer/), which is using FFMPEG (www.ffmpeg.org) and libpng (www.libpng.org). I used parts of ffmpegthumbnailer to decode, calculate duration, and save individual frames to files. I've also utilised a simple threading class to decrease the time it takes to generate a timelapse on a multicore CPU further (available at http://www.codeproject.com/KB/threads/SynchronizedThreadNoMfc.aspx). Other than that I've written the rest myself. The header in the code files specifies the author of that file. I do not claim to have written any of the files that does not have my name in or around the first line.

What is the future of this project?
---------

I have no intention of further developing this project, as I have other projects that I'm currently investing my time in. Though it was specifically written with the videos from Hurtigruten - Minutt for minutt in mind, it can be adapted to other situations where a timelapse is to be made out of large quantities of high definition footage. So if NRK should decide to make another marathon live show, I'm prepared to make a timelapse out of it.

Where can I see the output?
---------

You can see the final result here: http://www.youtube.com/watch?v=-9K8eCwdjF0

What about the text that was in the Youtube video?
---------

The license that NRK uses to permit anyone to use the material states that they should be referenced in the final product, in a way specified by them. I used a video editor to do this, and since I was already adding that text in the introduction, I also added the names of the places where the ship arrives.

Where can I find out more about the original show?
---------

Check out this FAQ written by NRKBeta: http://nrkbeta.no/2011/06/21/hurtigruten-faq-eng/

How can I use this?
---------

This is a release only intended for developers, but the original setup I used to generate the timelapse of Hurtigruten is included as a reference. Here's an explanation of the different files and directories:

- filelist.txt - This is where you list all the files you wish to make a timelapse out of. One line for each file, and any line starting with // will be ignored. The original contents of this file has been left as an example
- video-src - This is the folder where I put all the video files downloaded from NRK.
- gather - This is where the executable saves all the frames from the video files.
- output - This is the folder where the final output will end up.
- create_lossless.bat - This is the batch script I used to create the final output video. A similar one named create_compressed.bat will create a video file of smaller size, but with less quality.
- HurtigruteTimelapse.exe - When everything is set up correctly, you can double click this file, and images will start appearing in the folder named gather. When this process is finished, you can use one of the batch files to assemble the timelapse into a video file.

Currently there's a fixed duration of the final timelapse set to 10 minutes in the code and in the provided executable. If you want any other duration, you're gonna have to recompile the code yourself. The only supported compiler at the time of writing is MSVC-2008.

If you decide to port this to another compiler, or make changes to it that other can benefit from, I'd appreciate it if you made those changes publicly available, either by yourself or by contacting me.

=============

Thank you for taking the time to read this. If you have any questions or comments about this project, feel free to email me, my address is dale.oystein-at-gmail.com.