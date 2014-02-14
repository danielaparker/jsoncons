#!/usr/bin/env python
# -*- coding: utf-8 -*-
""""""
from SCons.Script import *
import re


__author__ = 'Pedro Larroy'
__version__ = '1.0'

def add_option(name, help, nargs):
    AddOption('--{0}'.format(name),
        dest = name,
        type = 'string',
        nargs = nargs,
        action = 'store',
        help = help)

def has_option(name):
    x = GetOption(name)
    if x == False or x is None or x == '':
        return False
    return True

def get_option(name):
    return GetOption(name)

def setup_quiet_build(env, colorblind=False):
    """Will fill an SCons env object with nice colors and quiet build strings. Makes warnings evident."""
    # colors
    c = dict()
    c['cyan']   = '\033[96m'
    c['purple'] = '\033[95m'
    c['blue']   = '\033[94m'
    c['bold_blue']   = '\033[94;1m'
    c['green']  = '\033[92m'
    c['yellow'] = '\033[93m'
    c['red']    = '\033[91m'
    c['magenta']= '\033[35m'
    c['bold_magenta']= '\033[35;1m'
    c['inverse']= '\033[7m'
    c['bold']   = '\033[1m'
    c['rst']    = '\033[0m'

    # if the output is not a terminal, remove the c
    # also windows console doesn't know about ansi c seems
        #or re.match('^win.*', plat_id())\
    if not sys.stdout.isatty()\
        or colorblind:

       for key, value in c.iteritems():
          c[key] = ''

    compile_cxx_msg = '%s[CXX]%s %s$SOURCE%s' % \
       (c['blue'], c['rst'], c['yellow'], c['rst'])

    compile_c_msg = '%s[CC]%s %s$SOURCE%s' % \
       (c['cyan'], c['rst'], c['yellow'], c['rst'])

    compile_shared_msg = '%s[SHR]%s %s$SOURCE%s' % \
       (c['bold_blue'], c['rst'], c['yellow'], c['rst'])

    link_program_msg = '%s[LNK exe]%s %s$TARGET%s' % \
       (c['bold_magenta'], c['rst'], c['bold'] + c['yellow'] + c['inverse'], c['rst'])

    link_lib_msg = '%s[LIB st]%s %s$TARGET%s' % \
       ('', c['rst'], c['cyan'], c['rst'])

    ranlib_library_msg = '%s[RANLIB]%s %s$TARGET%s' % \
       ('', c['rst'], c['cyan'], c['rst'])

    link_shared_library_msg = '%s[LNK shr]%s %s$TARGET%s' % \
       (c['bold_magenta'], c['rst'], c['bold'], c['rst'])

    pch_compile = '%s[PCH]%s %s$SOURCE%s -> %s$TARGET%s' %\
       (c['bold_magenta'], c['rst'], c['bold'], c['rst'], c['bold'], c['rst'])

    env['CXXCOMSTR']   = compile_cxx_msg
    env['SHCXXCOMSTR'] = compile_shared_msg
    env['CCCOMSTR']    = compile_c_msg
    env['SHCCCOMSTR']  = compile_shared_msg
    env['ARCOMSTR']    = link_lib_msg
    env['SHLINKCOMSTR'] = link_shared_library_msg
    env['LINKCOMSTR']  = link_program_msg
    env['RANLIBCOMSTR']= ranlib_library_msg
    env['GCHCOMSTR'] = pch_compile

def color_sample():
    """Show a sample of colors that will be used for SCons build"""
    env = dict()
    setup_quiet_build(env)
    for item in env.iteritems():
        print item[0],item[1]


def which(program):
    """Unix like which, to show where is an executable in path"""
    def is_exe(fpath):
        return os.path.exists(fpath) and os.access(fpath, os.X_OK)

    fpath, fname = os.path.split(program)
    if fpath:
        if is_exe(program):
            return program
    else:
        for path in os.environ["PATH"].split(os.pathsep):
            exe_file = os.path.join(path, program)
            if is_exe(exe_file):
                return exe_file

    return None


def sanitize_cenv(s):
    """Prevent recursions with ccache ccache ..."""
    l = s.split()
    last = None
    res = []
    for i in l:
        if i == last:
            pass
        else:
            res.append(i)
            last = i
    return ' '.join(res)


def get_compiler_only(s):
    """Get only the compiler (removing ccache) from the compiler string ($CC, $CXX)"""
    l = s.split()
    return ' '.join( [ i for i in l if i != "ccache" ] )


def isDir( node ):
    return isinstance(node, SCons.Node.FS.Dir)

def RGlob( patterns, baseDir = Dir('.') ):
    '''This function uses Glob to traverse the tree, since Glob is the only way to work properly with VariantDir'''
    files = Flatten( map( baseDir.glob, patterns ) )
    dirs = filter( isDir, baseDir.glob('*') )
    for dir in dirs:
        files.extend(RGlob(patterns, dir))
    return files

