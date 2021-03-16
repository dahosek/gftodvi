cmake_minimum_required ( VERSION 3.17 )
project ( gftodvi )

set ( CMAKE_CXX_STANDARD 20 )
#INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )

add_executable ( gftodvi main.cpp Bitmap.cpp GFReader.cpp PDFWriter.cpp)
TARGET_LINK_LIBRARIES( gftodvi LINK_PUBLIC ${Boost_LIBRARIES} )
