#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=clang
CCC=clang++
CXX=clang++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=CLang-Linux
CND_DLIB_EXT=so
CND_CONF=Release
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/BitMapAllocator.o \
	${OBJECTDIR}/ManagePageTable.o \
	${OBJECTDIR}/Trace.o \
	${OBJECTDIR}/main.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=-m32
CXXFLAGS=-m32

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=../../Downloads/MemorySubsystemS2019/dist/Debug/CLang-Linux/libmemorysubsystems2019.a

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/programming_assignment_2

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/programming_assignment_2: ../../Downloads/MemorySubsystemS2019/dist/Debug/CLang-Linux/libmemorysubsystems2019.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/programming_assignment_2: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/programming_assignment_2 ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/BitMapAllocator.o: BitMapAllocator.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -I../../Downloads/MemorySubsystemS2019 -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/BitMapAllocator.o BitMapAllocator.cpp

${OBJECTDIR}/ManagePageTable.o: ManagePageTable.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -I../../Downloads/MemorySubsystemS2019 -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/ManagePageTable.o ManagePageTable.cpp

${OBJECTDIR}/Trace.o: Trace.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -I../../Downloads/MemorySubsystemS2019 -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Trace.o Trace.cpp

${OBJECTDIR}/main.o: main.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -I../../Downloads/MemorySubsystemS2019 -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/main.o main.cpp

# Subprojects
.build-subprojects:
	cd ../../Downloads/MemorySubsystemS2019 && ${MAKE}  -f Makefile CONF=Debug

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}

# Subprojects
.clean-subprojects:
	cd ../../Downloads/MemorySubsystemS2019 && ${MAKE}  -f Makefile CONF=Debug clean

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
