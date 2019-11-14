## Canny Edge Detector BOINC Project  
BOINC example Project to detect edges using Canny algorithm in BMP images.  
The work generator split a large BMP file into different pieces than are then send to the volunteer computers to be processed using the cannyedgeapp.  
  
Then work assimilator just deletes the final images to save space in the server.

The cannyedge_app performs an edge detection in the source image and also do some fake computation for a few minutes.

Builds files are provided to crosscompile the application for ARM and ARM64 archs.
  
This project uses the CannyEdgeDetector class from here: https://github.com/resset/CannyEdgeDetector and the BMP processor API from here: https://github.com/wernsey/bitmap
