# Copyright (c) <2014-2017> <Newton Game Dynamics>
#
# This software is provided 'as-is', without any express or implied
# warranty. In no event will the authors be held liable for any damages
# arising from the use of this software.
#
# Permission is granted to anyone to use this software for any purpose,
# including commercial applications, and to alter it and redistribute it
# freely.

cmake_minimum_required(VERSION 3.4.0)

set (projectName "dgCore")
message (${projectName})

# low level core
file(GLOB CPP_SOURCE *.cpp)
file(GLOB HEADERS *.h)

if (UNIX OR MINGW)
	
	if(NEWTON_GENERATE_DLL STREQUAL "ON")
		message (${projectName} - STATIC LIBRARY)
		add_library(${projectName} STATIC ${CPP_SOURCE})
	else()
		
		if(NEWTON_BUILD_SHARED_LIBS STREQUAL "ON")
		   message (${projectName} - SHARED LIBRARY)
	   add_library(${projectName} SHARED ${CPP_SOURCE})
		else()
		   message (${projectName} - STATIC LIBRARY)
	   add_library(${projectName} STATIC ${CPP_SOURCE})
		endif()
		
	endif()
endif(UNIX OR MINGW)

if (MSVC)
      add_library(${projectName} STATIC ${CPP_SOURCE})

      if(CMAKE_VS_MSBUILD_COMMAND OR CMAKE_VS_DEVENV_COMMAND)
         set_target_properties(${projectName} PROPERTIES COMPILE_FLAGS "/YudgStdAfx.h")
         set_source_files_properties(dgTypes.cpp PROPERTIES COMPILE_FLAGS "/YcdgStdAfx.h")
      endif()
endif(MSVC)

target_include_directories(${projectName} PUBLIC .)
if (NEWTON_BUILD_PROFILER)
    target_link_libraries (${projectName} dProfiler)
endif()

install(TARGETS ${projectName}
       LIBRARY DESTINATION lib
       ARCHIVE DESTINATION lib
       RUNTIME DESTINATION bin)

install(FILES ${HEADERS} DESTINATION include/${projectName})
