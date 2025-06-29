# recastnavigation/Makefile
# Dieses Makefile kompiliert die Recast & Detour Bibliothek als statische Library

# === 1) Konfiguration ===

# BUILD_DIR: Verzeichnis, in dem Objektdateien (*.o) und die Bibliothek abgelegt werden
# Default: "build" (kann beim Aufruf mit BUILD_DIR=<anderesVerzeichnis> überschrieben werden)
BUILD_DIR ?= build

# CXX: Pfad zum C++ Cross-Compiler
# Hier: AArch64-ELF-Compiler für die Nintendo Switch
# Überschreibt zwingend die Standard-Variable (kein "?="), um sicher den richtigen Compiler zu nutzen
CXX = /opt/devkitpro/devkitA64/bin/aarch64-none-elf-g++

# CXXFLAGS: Compiler-Optionen
#  -O2                   : mittlere Optimierungsstufe für schnellere Laufzeit
#  -march=armv8-a+crc    : Ziel-Architektur ARMv8-A mit CRC-Erweiterung
#  -fsigned-char         : "char" als signed-Typ definieren (Switch-Standard)
#  -D__SWITCH__          : Definiere Makro __SWITCH__, um Switch-spezifischen Code zu aktivieren
#  -I Recast/Include     : Include-Pfad zu Recast-Headern
#  -I Detour/Include     : Include-Pfad zu Detour-Headern
CXXFLAGS ?= -O2 \
            -march=armv8-a+crc \
            -fsigned-char \
            -D__SWITCH__ \
            -I Recast/Include \
            -I Detour/Include

# === 2) Quell- und Objekt-Dateien ===

# RECAST_SRC / DETOUR_SRC: Listen aller .cpp-Quelldateien in den jeweiligen Ordnern
RECAST_SRC := $(wildcard Recast/Source/*.cpp)
DETOUR_SRC := $(wildcard Detour/Source/*.cpp)

# SOURCES: Alle Quelldateien kombiniert
SOURCES    := $(RECAST_SRC) $(DETOUR_SRC)

# OBJECTS: Pfade der zu erzeugenden Objektdateien im BUILD_DIR
# - $(notdir $(SOURCES)) entfernt Ordnerpfad, sodass nur der Dateiname übrig bleibt
# - $(patsubst) hängt BUILD_DIR/ und .o statt .cpp an
OBJECTS    := $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(notdir $(SOURCES)))

# === 3) Phony-Targets ===

# Kennzeichnet diese Ziele als "phony" (keine echten Dateien)
.PHONY: all clean

# --- Ziel: all ---
# Standardziel, das durch "make" ohne Argumente ausgeführt wird
# Baut die finale statische Bibliothek
all: $(BUILD_DIR)/librecastdetour.a

# --- Ziel: lib ---
# Erstellt die statische Bibliothek aus allen Objektdateien
$(BUILD_DIR)/librecastdetour.a: $(OBJECTS)
	# ar: Archiv-Tool, rcs: replace/create, silent
	ar rcs $@ $^

# --- Ziel: Objektdatei Recast ---
# Regel: aus jeder einzelnen Recast-Quelldatei eine .o-Datei erzeugen
# $<  = Prerequisite (z.B. Recast/Source/Recast.cpp)
# $@  = Target (z.B. build/Recast.o)
$(BUILD_DIR)/%.o: Recast/Source/%.cpp | $(BUILD_DIR)
	# Kompiliere Quelldatei mit definiertem Cross-Compiler und Flags
	$(CXX) $(CXXFLAGS) -c $< -o $@

# --- Ziel: Objektdatei Detour ---
# Analog zur Recast-Regel für Detour-Quelldateien
$(BUILD_DIR)/%.o: Detour/Source/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# --- Ziel: BUILD_DIR ---
# Stellt sicher, dass das Verzeichnis existiert, bevor Objektdateien erzeugt werden
$(BUILD_DIR):
	mkdir -p $@

# --- Ziel: clean ---
# Entfernt alle generierten Artefakte
clean:
	rm -rf $(BUILD_DIR)
