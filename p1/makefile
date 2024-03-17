# Variables
CXX = g++
SRCDIR = src
INCDIR = include
OBJDIR = obj
BINDIR = bin
CXXFLAGS = -I$(INCDIR)
TESTS_EXECUTABLE = $(BINDIR)/tests
EXPERIMENTS_EXECUTABLE = $(BINDIR)/experiments

# Reglas
all: tests experiments

tests: $(TESTS_EXECUTABLE)

experiments: $(EXPERIMENTS_EXECUTABLE)

$(TESTS_EXECUTABLE): $(OBJDIR)/tests.o $(OBJDIR)/bigInt.o
	@mkdir -p $(BINDIR)
	$(CXX) -o $@ $^

$(EXPERIMENTS_EXECUTABLE): $(OBJDIR)/experiments.o $(OBJDIR)/bigInt.o
	@mkdir -p $(BINDIR)
	$(CXX) -o $@ $^

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJDIR)/* $(BINDIR)/*

.PHONY: all tests experiments clean

