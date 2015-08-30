TEX = pdflatex -shell-escape -interaction=nonstopmode -file-line-error
PRE =  $(TEX) -ini -job-name="preamble" "&pdflatex preamble.tex\dump"
BIB = bibtex

all: TCS10.pdf

view :
	    open TCS10.pdf

TCS10.pdf : TCS10.tex TCS10.bbl TCS10.blg
	    $(TEX) TCS10.tex

TCS10.bbl TCS10.blg : TCS10.bib TCS10.aux
	    $(BIB) TCS10

TCS10.aux : TCS10.tex
	    $(TEX) TCS10.tex

TCS10.bib : TCS10.tex
	    $(TEX) TCS10.tex

clean:
	rm -f TCS10.log TCS10.out sigproc.bib.backup llncs.aux TCS10.pdf *blg *.bbl *aux
