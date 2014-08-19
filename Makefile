CFLAGS = `pkg-config --cflags gtk+-3.0 gtksourceview-3.0 webkit2gtk-3.0` -I.
LDFLAGS = `pkg-config --libs gtk+-3.0 gtksourceview-3.0 webkit2gtk-3.0` -lhoedown -Lhoedown/
DIR = build

all: clean notes

clean:
	rm -rf build/

${DIR}:
	mkdir -p ${DIR}

notes: ${DIR}
	gcc $(CFLAGS) -o $(DIR)/notes main.c $(LDFLAGS)

run: clean notes
	build/notes

.PHONY: all clean