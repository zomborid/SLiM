.PHONY: clean compile copy_resources install

# works:
# windows, using cygwin
# some linux distros

RM = rm -f
RD = rm -rf
MKDIR = mkdir -p
CP = cp -f
CPDIR = cp -rf
export RM RD MKDIR CP CPDIR

MAKE_DIR=$(shell pwd)
PLATFORM=$(shell uname)
BUILD_DATE=$(shell date "+%Y%m%d")
BUILD_DIR=build
PLATFORM_DIR=platform
SOURCE_DIR=src
RESOURCE_DIR=resources
INSTALL_DIR="C:\slim"

ifeq ($(PLATFORM),Linux)
	RUNNABLE=slim
	MAKE_HOME=make_home.sh
else
	RUNNABLE=slim.exe
	MAKE_HOME=make_home.bat
endif

export MAKE_DIR BUILD_DIR BUILD_DATE PLATFORM RUNNABLE

default: compile copy_resources

install:
	$(RD) $(INSTALL_DIR)
	$(MKDIR) $(INSTALL_DIR)
	$(CPDIR) $(BUILD_DIR)/* $(INSTALL_DIR)
	
copy_resources:
	$(MKDIR) $(BUILD_DIR)
	$(CPDIR) $(RESOURCE_DIR)/* $(BUILD_DIR)
	$(CPDIR) $(PLATFORM_DIR) $(BUILD_DIR)
	$(CP) $(SOURCE_DIR)/$(RUNNABLE) $(BUILD_DIR)/$(RUNNABLE)
	$(CP) install/$(MAKE_HOME) $(BUILD_DIR)/$(MAKE_HOME)
	$(CP) install/ReadMe.txt $(BUILD_DIR)/ReadMe.txt
	
compile:
# currently filesystem library is not supported for gcc on windows
# if you compile externally and just pack project
# place slim.exe in src and use the skipcompile target
# in this case version information will depend on the compiling system
	$(warning compilation is currently disabled, enable it by removing comment)
	$(CP) "..\SLiM\x64\Release\SLiM.exe" "$(SOURCE_DIR)\$(RUNNABLE)"
#	$(MAKE) -C $(SOURCE_DIR)
	
clean:
	$(RD) $(BUILD_DIR)
	$(MAKE) -C $(SOURCE_DIR) clean