## Process this file with automake to produce Makefile.in

AM_CPPFLAGS = \
        -I$(top_srcdir) \
        -I$(top_srcdir)/lib/common \
        -I$(top_srcdir)/lib/gvc \
        -I$(top_srcdir)/lib/pack \
        -I$(top_srcdir)/lib/pathplan \
        -I$(top_srcdir)/lib/graph \
        -I$(top_srcdir)/lib/cdt \
	-I$(top_srcdir)/lib/libvpsc

noinst_HEADERS = adjust.h edges.h geometry.h heap.h hedges.h info.h mem.h \
	neato.h poly.h neatoprocs.h simple.h site.h voronoi.h \
	bfs.h closest.h conjgrad.h defs.h dijkstra.h embed_graph.h kkutils.h \
	matrix_ops.h pca.h stress.h quad_prog_solver.h digcola.h \
	quad_prog_vpsc.h mosek_quad_solve.h
noinst_LTLIBRARIES = libneatogen.la

libneatogen_la_SOURCES = adjust.c circuit.c edges.c find_ints.c geometry.c \
	heap.c hedges.c info.c neatoinit.c intersect.c legal.c lu.c matinv.c \
	memory.c poly.c printvis.c site.c solve.c neatosplines.c stuff.c \
	voronoi.c stress.c kkutils.c matrix_ops.c embed_graph.c dijkstra.c \
	conjgrad.c pca.c closest.c bfs.c constraint.c quad_prog_solve.c \
	smart_ini_x.c constrained_majorization.c opt_arrangement.c \
	compute_hierarchy.c \
	quad_prog_vpsc.c constrained_majorization_vsep.c \
	mosek_quad_solve.c

EXTRA_DIST = Makefile.old
