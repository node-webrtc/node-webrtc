import sys
import subprocess
import re
import os

LIBS = [
    ['layout/media/', 'libgkmedias.a'],
    ['media/mtransport/standalone/', 'libmtransport_s.a'],
    ['media/webrtc/signalingtest/signaling_ecc/', 'libecc.a'],
    ['media/webrtc/signaling/signaling_sipcc/', 'libsipcc.a'],
    ['netwerk/sctp/src/', 'libnksctp_s.a'],
    ['netwerk/srtp/src/', 'libnksrtp_s.a'],
    ['modules/zlib/src/', 'libmozz.a'],
]

def die(msg):
    sys.stderr.write("ERROR: %s\n"%msg)
    sys.exit(1)

def read_archive(filename):
    f = open(filename)
    if f is None:
        die("Could not open file")
    l = f.readline()
    l.strip()
    m = re.match("([A-Z]+) = (.*)", l)
    if m is None:
        die("%s does not appear to be a .a.desc file"%file)
    return [m.group(1), m.group(2).split(" ")]

def make_archive(obj, dir, file):
    [type, lst] = read_archive(obj + dir + file + ".desc")
    if type == "LIBS":
        expand_archive_archive(obj, lst)
    else:
        expand_archive(obj, lst, file)

def expand_archive_archive(obj, lst):
    for f in lst:
        prefix = f[0:len(obj)]
        if prefix != obj:
            die("Unexpected object prefix")
        suffix = f[len(obj):]
        [dir, file] = os.path.split(suffix)
        make_archive(obj, dir + "/" , file)

def expand_archive(obj, args, file):
    outfile = obj + "/dist/lib/" + file;
    print outfile
    args.insert(0, outfile)
    args.insert(0, '-r')
    args.insert(0, 'ar')
    subprocess.check_call(args)


if len(sys.argv) != 2:
    die("Usage: make-archives.py <objdir>")

for f in LIBS:
    make_archive(sys.argv[1], f[0], f[1])


