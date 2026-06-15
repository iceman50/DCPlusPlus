This folder contains useful parts of libdwarf <http://www.prevanders.net/dwarf.html>.

Do not round compilation-unit offsets to a fixed boundary. GNU PE/COFF debug sections can place
the next unit at an unaligned offset, and libdwarf reads the encoded fields without host-alignment
assumptions.
