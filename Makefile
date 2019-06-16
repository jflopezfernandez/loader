
vpath %.cpp src
vpath %.hpp include

CP       = cp -f -u
RM       = rm -f

OBJS     = main.o loader.o

# CC       = gcc
# CFLAGS   = -std=c17 -Wall -Wextra -Wpedantic -O3 -mtune=intel -march=skylake
# CFLAGS   = -std=c17 -Wall -Wextra -Wpedantic
# CPPFLAGS = -D_GNU_SOURCE -D_XOPEN_SOURCE=700
# LDFLAGS  = -lm

CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Weffc++ -Wpedantic
CPPFLAGS = -D_GNU_SOURCE -D_XOPEN_SOURCE=700 -DPACKAGE=LOADER -DPACKAGE_VERSION="1.0.0"
LDFLAGS  = -lboost_program_options -lbfd

TARGET   = loader

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -I include    -o $@ $^ $(LDFLAGS)

%.o: %.cpp
#	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -I include -P -E -o $(basename $@).pp $^
#	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -I include -S -masm=intel -o $(basename $@).asm $^
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -I include -c -o $@ $^

.PHONY: clean
clean:
	$(RM) ./*.{pp,asm,o} $(TARGET)

.PHONY: install
install: $(TARGET)
	$(CP) ./$(TARGET) /usr/bin

.PHONY: uninstall
uninstall:
	$(RM) /usr/bin/$(TARGET)

