NAME = LAYERCE
DESCRIPTION = "Layer Defense v0.1"
COMPRESSED = NO
ARCHIVED = YES
OUTPUT_MAP = YES
CFLAGS = -Wall -Wextra -Oz -std=c11
CXXFLAGS = -Wall -Wextra -Oz

include $(shell cedev-config --makefile)
