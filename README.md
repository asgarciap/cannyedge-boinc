## Canny Edge Detector BOINC Project  
BOINC Project to detect edges using Canny algorithm in BMP images.  
The work generator split a large BMP file into different pieces than are then send to the volunteer computers to be processed using the cannyedgeapp.  
  
Then work assimilator join the pieces to generate the final image.  
  
This project uses the CannyEdgeDetector class from here: https://github.com/resset/CannyEdgeDetector and the BMP processor API from here: https://github.com/wernsey/bitmap
