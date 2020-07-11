#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM)
endif

include $(DEVKITARM)/ds_rules

export TARGET		:=	$(shell basename $(CURDIR))
export TOPDIR		:=	$(CURDIR)

LIBDIRS :=      $(LIBNDS)

#---------------------------------------------------------------------------------
# path to tools - this can be deleted if you set the path in windows
#---------------------------------------------------------------------------------
export PATH		:=	$(DEVKITARM)/bin:$(PATH)

export INCLUDE		:= 	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
				$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
				-I$(CURDIR)/$(BUILD)

.PHONY: arm9/lib/$(TARGET)9.a arm7/lib/$(TARGET)7.a

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
all: arm9/lib/$(TARGET)9.a arm7/lib/$(TARGET)7.a
	@cp arm9/lib/$(TARGET)9.a arm7/lib/$(TARGET)7.a lib

arm9/lib/$(TARGET)9.a:
	$(MAKE) -C arm9

arm7/lib/$(TARGET)7.a:
	$(MAKE) -C arm7

install: all 
	cp lib/*.a $(DEVKITPRO)/libnds/lib
	cp -R include/ntxm $(DEVKITPRO)/libnds/include

#---------------------------------------------------------------------------------
clean:
	$(MAKE) -C arm9 clean
	$(MAKE) -C arm7 clean
	@rm -f lib/$(TARGET)9.a lib/$(TARGET)7.a
