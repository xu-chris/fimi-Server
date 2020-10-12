# Find all dependencies. To enable reuse, each package is in it's own file in Snippets
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake/Modules/)

# Clear
SET(EXTERN_LIBS "")
SET(EXTERN_INCS "")

# OpenMP
INCLUDE(${CMAKE_SOURCE_DIR}/cmake/Snippets/CFindOpenMP.cmake)

# Eigen, included in extern
INCLUDE(${CMAKE_SOURCE_DIR}/cmake/Snippets/CFindEigen.cmake)
SET(EXTERN_INCS ${EXTERN_INCS} ${Eigen_INCLUDE_DIR})
# Caffe
INCLUDE(${CMAKE_SOURCE_DIR}/cmake/Snippets/CFindCaffe.cmake)
SET(EXTERN_INCS ${EXTERN_INCS} ${Caffe_INCLUDE_DIRS} ${Caffe_DIR} ${Caffe_DIR}/include)
SET(EXTERN_LIBS ${EXTERN_LIBS} ${Caffe_LIBRARIES})

# OpenCV
INCLUDE(${CMAKE_SOURCE_DIR}/cmake/Snippets/CFindOpenCV.cmake)
SET(EXTERN_INCS ${EXTERN_INCS} ${OpenCV_INCLUDE_DIRS})
SET(EXTERN_LIBS ${EXTERN_LIBS} ${OpenCV_LIBS})

# xnect
INCLUDE(${CMAKE_SOURCE_DIR}/cmake/Snippets/CFindXNECT.cmake)
SET(EXTERN_INCS ${EXTERN_INCS} ${xnect_INCLUDE_DIR})
SET(EXTERN_LIBS ${EXTERN_LIBS} ${xnect_LIBS})
# rtpose
INCLUDE(${CMAKE_SOURCE_DIR}/cmake/Snippets/CFindRtPose.cmake)
SET(EXTERN_INCS ${EXTERN_INCS} ${RtPose_INCLUDE_DIR})

# Websocket++
INCLUDE(${CMAKE_SOURCE_DIR}/cmake/Snippets/CFindWebSocketPP.cmake)
SET(EXTERN_INCS ${EXTERN_INCS} ${WebSocketPP_INCLUDE_DIRS})