## Process this file with automake to produce Makefile.in

SUFFIXES = .pod .1 .man .html .txt .pm .pdf .inc

#AUTOMAKE_OPTIONS        =  foreign

#ACLOCAL_M4 = $(top_srcdir)/config/aclocal.m4

CLEANFILES = *.1 *.html *.txt *-dircache RRD?.pod *.pdf *~ core *itemcache *.rej *.orig *.tmp

POD = bin_dec_hex.pod        rrddump.pod            rrdgraph_examples.pod  rrdrestore.pod         rrdupdate.pod  \
      cdeftutorial.pod       rrdfetch.pod           rrdgraph_graph.pod     rrdthreads.pod         rrdxport.pod   \
      rpntutorial.pod        rrdfirst.pod           rrdgraph_rpn.pod       rrdtool.pod                           \
      rrd-beginners.pod      rrdinfo.pod            rrdtune.pod            rrdbuild.pod                          \
      rrdcgi.pod             rrdgraph.pod           rrdlast.pod                                                  \
      rrdcreate.pod          rrdgraph_data.pod      rrdresize.pod          rrdtutorial.pod                       


PMP = RRDs.pod RRDp.pod

MAN = $(POD:.pod=.1)
TXT = $(MAN:.1=.txt)
HTML = $(POD:.pod=.html) $(PMP:.pod=.html)
PDF = $(MAN:.1=.pdf)

# what should go into the distribution
EXTRA_DIST= $(POD) $(HTML) $(MAN) $(TXT) rrdtool-dump.dtd rrdtool-xport.dtd

# some install rules if the install pat
DOCPREFIX:=$(shell echo "@prefix@" | grep rrd >/dev/null || echo "rrdtool-@VERSION@/")
idocdir = $(prefix)/share/doc/$(DOCPREFIX)txt
idoc_DATA = $(POD) $(TXT)
ihtmldir = $(prefix)/share/doc/$(DOCPREFIX)html
ihtml_DATA = $(HTML)
imandir = $(mandir)/man1
iman_DATA = $(MAN)

all-local: link txt man html-local

.src.pod:
	perl -n -e 'if (/^=include\s+(\S+)/){open F,"$$1.inc" || die $$?;print <F>; close F} else {print}'  $<  > $@

.pod.1 .pm.1 .pl.1:
	pod2man --release=$(VERSION) --center=rrdtool $<  > $@

.1.txt:
	GROFF_NO_SGR=1 @NROFF@ -man -Tlp $< > $@

.1.pdf:
	@TROFF@ -man $< | ps2pdf - $@

.pm.html .pod.html .pl.html:
	pod2html --infile=$< --outfile=$@ --noindex --htmlroot=. --podpath=. --title=$*

RRDs.pod:
	ln -s $(top_srcdir)/bindings/perl-shared/RRDs.pm RRDs.pod

RRDp.pod:
	ln -s $(top_srcdir)/bindings/perl-piped/RRDp.pm RRDp.pod

link: RRDp.pod RRDs.pod

man: $(MAN)

html-local: $(HTML)

txt: $(TXT)

pdf-local: $(PDF)

pod: $(POD)


