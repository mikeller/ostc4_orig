#! /usr/bin/env python3

from __future__ import print_function
import os, sys, glob,re, codecs

##############################################################################

OSTC4=os.getcwd()
if not os.path.exists( os.path.join(OSTC4, 'Discovery') ):
    raise 'Wrong start directory...'

##############################################################################

def walk(OSTC4):
    for dir in ['BootLoader', 'Common', 'Discovery', 'FontPack', 'Small_CPU']:
        # Make sure we have the top directory...
        if not os.path.exists( os.path.join(OSTC4, dir) ):
            raise 'Missing ' + dir

        # Then walk in all its existing source sub-directories...
        for sub in ['.', 'Inc', 'Src']:
            path = os.path.join(OSTC4, dir, sub)
            if not os.path.exists(path):
                continue
            print(path + ':')
            for file in sorted( glob.iglob( os.path.join(path, '*.[chs]') ) ):
                try:
                    work(file)
                except Exception as e:
                    raise RuntimeError('Cannot process ' + file + ':\n\t' + str(e))

##############################################################################

with open(os.path.join(OSTC4, 'Documentations', 'GPL_template.txt')) as f:
    template = f.read()

def work(file):

    # Unclear what is done if encoding is not yet UTF-8...
    with open(file, 'rt', encoding='cp1252') as f:
        lines = f.read().splitlines()

    # Set defaults
    kw = {}
    kw['file']   = file.replace(OSTC4+'/', '')
    kw['brief']  = ''
    kw['date']   = '2018'

    # Try to gather info from existing header, if any:
    for line in lines:
        # Skip files that already have a GNU license:
        if 'GNU General Public License' in line:
            print('(done)\t' + file)
            return

        get(line, 'brief', kw)
        get(line, 'date', kw)

    # Replace kw in header
    header = template
    for k,v in kw.items():
        header = header.replace('$'+k, v)

    # Overwrite resulting file, normalizing EOL
    with open(file, 'wt', encoding='utf-8') as f:
        for line in header.splitlines():
            f.write(line.rstrip() + '\n')
        for line in lines:
            f.write(line.rstrip() + '\n')
    print('+\t' + file)

##############################################################################

def get(line, word, kw):
    m = re.search('(@|\\\\)' + word + '\s*(.*)', line)
    if m:
        kw[word] = m.group(2)

##############################################################################

walk(OSTC4)
