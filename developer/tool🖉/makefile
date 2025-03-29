
RT-INCOMMON:=$(REPO_HOME)/tool_shared/third_party/RT-project-share/release

include $(RT-INCOMMON)/make/environment_RT_0

# to compile only the example directory
# comment these out for a normal compile
SRCDIR_List=example
# if the .lib.c interfaces are needed:
CFLAGS+=-Icc🖉

CFLAGS+= -include "$(RT-INCOMMON)/make/RT_0.h"
LINKFLAGS+= -l$(PROJECT)
LIBFILE=$(LIBDIR)/lib$(PROJECT).a

include $(RT-INCOMMON)/make/targets_developer
-include $(DEPFILE)






