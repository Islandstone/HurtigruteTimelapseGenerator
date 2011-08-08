ffmpeg.exe -s 1920x1080 -r 25 -y -i "gather/frame-%%05d.png" -threads 0 -b 9000k -bt 9000k -vpre libx264-slow -vcodec libx264 "output/Hurtigruten_timelapse_compressed.mp4"
pause