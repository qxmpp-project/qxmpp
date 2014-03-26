#!/usr/bin/python

import getopt
import os
import platform
import subprocess
import sys

root = os.path.dirname(__file__)
report_path = None

def usage():
    print "Usage: run.py [options]"

# parse options
try:
    opts, args = getopt.getopt(sys.argv[1:], 'hx:')
except getopt.GetoptError, err:
    print err
    usage()
    sys.exit(2)

for opt, optarg in opts:
    if opt == '-h':
        usage()
        sys.exit()
    elif opt == '-x':
        report_path = optarg
        if not os.path.exists(report_path):
            os.mkdir(report_path)

# set library path
path = os.path.join(root, '..', 'src')
if platform.system() == 'Darwin':
    os.environ['DYLD_LIBRARY_PATH'] = path
else:
    os.environ['LD_LIBRARY_PATH'] = path

# run tests
failed = False
for test in os.listdir(root):
    test_path = os.path.join(root, test)
    if os.path.isdir(test_path):
        if platform.system() == 'Darwin':
            prog = os.path.join(test_path, 'tst_' + test + '.app', 'Contents', 'MacOS', 'tst_' + test)
        elif platform.system() == 'Windows':
            prog = os.path.join(test_path, 'tst_' + test + '.exe')
        else:
            prog = os.path.join(test_path, 'tst_' + test)
        if not os.path.exists(prog):
            continue

        cmd = [ prog ]
        if report_path:
            cmd += ['-xunitxml', '-o',  os.path.join(report_path, test + '.xml') ]
        try:
            subprocess.check_call(cmd)
        except subprocess.CalledProcessError:
            failed = True

# check for failure
if failed:
    sys.exit(1)
