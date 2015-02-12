#!/usr/bin/python
import sys
import fnmatch
import os
import subprocess

# argv should be labname filename

sub_url = "http://ec2-54-173-71-20.compute-1.amazonaws.com/submit"
reg_url = "http://ec2-54-173-71-20.compute-1.amazonaws.com/reg"

def find_auth_file(dir):
    key_from_home = fnmatch.filter(os.listdir(dir), "cs202-auth-*")
    key_paths = [os.path.join(dir, f) for f in key_from_home]
    return key_paths


def read_auth():
    key_from_home_cs202 = find_auth_file(os.path.join(os.getenv("HOME"), "cs202")) # $HOME/cs202 dir
    key_from_parent = find_auth_file("./") # i.e. cs202-labs, just in case...
    key_from_home = find_auth_file(os.getenv("HOME")) # the home dir
    fs = key_from_parent + key_from_home_cs202 + key_from_home
    if len(fs) == 0:
        print "No key file found. Please go to " + reg_url + " to obtain your key."
        print "And make sure it is in your ~/cs202/ folder."
        exit()
    with open(fs[0]) as f:
        content = f.readlines()
    if len(content) != 2:
        print "Auth file is corrupted. Exit....."
        exit()
    return (content[0].strip(), content[1].strip())

def print_usage():
    print "Usage: python submit.py <lab-name> <file-name>"

def main(argv):
    if len(argv) != 2:
        print_usage()
        exit()
    l = argv[0]
    f = argv[1]
    (u, p) = read_auth()
    #print u, p
    r = subprocess.Popen(["curl", "--user", u+":"+p, "-X", "POST", "-F", "file=@" + f, "-F", "lab=" + l,  sub_url], stdout=subprocess.PIPE).communicate()[0]
    if r != "ok":
        print "Submission failed, please check if your authentication file is intact or your internet connections."
    else:
        print '\033[92m' + "Submission successful" + '\033[0m'
    #os.system("curl -X POST -F file=%s -F lab=%s --user %s:%s %s > /dev/null" % (f, l, u, p, sub_url))

if __name__ == "__main__":
    main(sys.argv[1:])
