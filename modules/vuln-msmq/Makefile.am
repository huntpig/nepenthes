# nepenthes module Makefile
# Paul Baecher, Maximillian Dornseif, Markus Koetter
# $Id$

AUTOMAKE_OPTIONS = foreign

AM_CPPFLAGS = -I$(top_srcdir)/nepenthes-core/include -I$(top_srcdir)/nepenthes-core/src -pipe -D _GNU_SOURCE
AM_CXXFLAGS = -Wall -Werror


pkglib_LTLIBRARIES = vulnmsmq.la

vulnmsmq_la_SOURCES = vuln-msmq.conf.dist 
vulnmsmq_la_SOURCES += MSMQDialogue.hpp MSMQDialogue.cpp        
vulnmsmq_la_SOURCES += vuln-msmq.hpp    vuln-msmq.cpp           
							       


vulnmsmq_la_LDFLAGS = -module -no-undefined -avoid-version
