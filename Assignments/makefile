###############################################################################
# Variables
###############################################################################
TEXES := $(wildcard src/*.tex) # All .tex files
PDFS := $(TEXES:src/%.tex=out/%.pdf) # All output .pdf files

###############################################################################
# Recipes
###############################################################################
.PHONY: all clean

all: $(PDFS)

# Making and placing the output pdfs is a two step process
# 1) compile it in the auxilliary directory "trash", and copy it to out/
out/%.pdf: trash/%.pdf
	mkdir -p out
	cp trash/$*.pdf out/$*.pdf

trash/%.pdf: src/%.tex
	mkdir -p trash
	latexmk $^ -pdf -outdir=trash

clean:
	rm -rf out/* trash/*
