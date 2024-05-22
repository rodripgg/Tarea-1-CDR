# Nombre del compilador
CXX = g++

# Opciones del compilador
CXXFLAGS = -Wall -Wextra -std=c++17

# Nombre del ejecutable
TARGET = servidor

# Archivos fuente
SRCS = servidor.cpp

# Archivos objeto (se generan a partir de los archivos fuente)
OBJS = $(SRCS:.cpp=.o)

# Regla por defecto (primera regla)
all: $(TARGET)

# Regla para compilar el ejecutable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

# Regla para compilar archivos .cpp a .o
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Limpieza de archivos generados
clean:
	rm -f $(OBJS) $(TARGET)

# Limpieza completa, incluyendo archivos temporales
distclean: clean
	rm -f *~

# Phony targets (evitan conflictos con archivos que se llamen igual)
.PHONY: all clean distclean
