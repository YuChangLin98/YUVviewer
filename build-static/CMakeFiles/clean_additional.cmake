# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Release")
  file(REMOVE_RECURSE
  "CMakeFiles\\YuvViewer_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\YuvViewer_autogen.dir\\ParseCache.txt"
  "CMakeFiles\\tst_YuvParser_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\tst_YuvParser_autogen.dir\\ParseCache.txt"
  "CMakeFiles\\tst_YuvToRgbConverter_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\tst_YuvToRgbConverter_autogen.dir\\ParseCache.txt"
  "YuvViewer_autogen"
  "tst_YuvParser_autogen"
  "tst_YuvToRgbConverter_autogen"
  )
endif()
