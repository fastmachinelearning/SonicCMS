#!/usr/bin/env python

# baconBatch.py #############################################################################
# Python driver for Bacon Analyzer executable
# Original Author N.Wardle (CERN) 

# ------------------------------------------------------------------------------------

import ROOT as r

import sys, commands, os, fnmatch
from optparse import OptionParser
from optparse import OptionGroup
from numpy import arange
from itertools import product
#from BaconAna.Utils.makeFilelist import *
from makeFilelist import *

# I want to make jobs out of ANY executable and separate based on groups of files or [list] input parameters. Also need the input swap to be as generic as possible

# Ok this is dangerous since we pretty much have to assume some arguments for the exec
# Take WAnalysis as the standard command line style
# 'maxevents, input, isGen
# default_args = ['10000000','nothing.root','1'] #,output.root -> could add to analyzer
default_args = []
EOS = '/cvmfs/cms.cern.ch/slc6_amd64_gcc472/cms/cmssw/CMSSW_5_3_32/external/slc6_amd64_gcc472/bin/xrdcp'

# Options
parser = OptionParser()
parser = OptionParser(usage="usage: %prog analyzer outputfile [options] \nrun with --help to get list of options")
parser.add_option("-d", "--directory", default='',
                  help="Pick up files from a particular directory. can also pass from /eos/. Will initiate split by files (note you must also pass which index the file goes to)")
parser.add_option("-l", "--list", default='', help="Pick up files from a particular list of files")
parser.add_option("-o", "--outdir", default='bacon',
                  help="output for analyzer. This will always be the output for job scripts.")
parser.add_option("-e", "--eosoutdir", default='', help="eos output directory for analyzer files.")
parser.add_option("-a", "--args", dest="args", default=[], action="append",
                  help="Pass executable args n:arg OR named arguments name:arg. Multiple args can be passed with <val1,val2...> or lists of integers with [min,max,stepsize]")
parser.add_option("-v", "--verbose", dest="verbose", default=False, action="store_true", help="Spit out more info")
parser.add_option("", "--passSumEntries", dest="passSumEntries", default="",
                  help="x:treename Get Entries in TTree treename and pass to argument x")
parser.add_option("", "--blacklist", dest="blacklist", default=[], action="append",
                  help="Add blacklist file types (search for this string in files and ignore them")

# Make batch submission scripts options
parser.add_option("-n", "--njobs", dest="njobs", type='int', default=-1,
                  help="Split into n jobs, will automatically produce submission scripts")
parser.add_option("--njobs-per-file", dest="njobs_per_file", type='int', default=1,
                  help="Split into n jobs per file, will automatically produce submission scripts")
parser.add_option("-q", "--queue", default='1nh', help="submission queue")

parser.add_option("--dryRun", default=False, action="store_true", help="Do nothing, just create jobs if requested")

# Monitor options (submit,check,resubmit failed)  -- just pass outodir as usual but this time pass --monitor sub --monitor check or --monitor resub
parser.add_option("--monitor", default='', help="Monitor mode (sub/resub/check directory of jobs)")

cwd = os.getcwd()
(options, args) = parser.parse_args()
if len(args) < 2 and not options.monitor: sys.exit('Error -- must specify ANALYZER and OUTPUTNAME')
njobs = options.njobs if options.njobs > 0 else 1
njobs_per_file = options.njobs_per_file


def write_job(exec_line, out, analyzer, i, n, j, eosout=''):
    cwd = os.getcwd()
    analyzer_short = analyzer.split("/")[-1]

    exec_line = exec_line.replace(analyzer, analyzer_short)
    pyconfig  = exec_line.split(' ')[1]
    #print 'pyconfig', pyconfig
    sub_file = open('%s/sub_%s_job%d_subjob%d.sh' % (out, analyzer_short, i, j), 'w')
    sub_file.write('#!/bin/bash\n')
    sub_file.write('# Job Number %d, running over %d files, subjob %d \n' % (i, n, j))
    sub_file.write('pwd\n')
    sub_file.write('touch %s.run\n' % os.path.abspath(sub_file.name))
    sub_file.write('cd %s\n' % cwd)
    sub_file.write('pwd\n')
    sub_file.write('eval `scramv1 runtime -sh`\n')
    sub_file.write('cd -\n')
    sub_file.write('pwd\n')
    sub_file.write("export TWD=${PWD}/%s_job%d_subjob%d\n" % (analyzer_short, i, j))
    sub_file.write("export X509_USER_PROXY=/afs/cern.ch/user/w/woodson/x509up_u50832\n")
    sub_file.write("mkdir -p $TWD\n")
    sub_file.write("cd $TWD\n")
    sub_file.write("cp %s/%s .\n"%(cwd,pyconfig))
    # sub_file.write('cp -p $CMSSW_BASE/bin/$SCRAM_ARCH/%s .\n'%analyzer)
    # sub_file.write('cp -p %s .\n'%(os.path.abspath(analyzer)))
    sub_file.write('mkdir -p %s\n' % (out))
    if eosout:
        sub_file.write('%s mkdir -p %s\n' % (EOS, eosout))

    sub_file.write('if ( %s ) then\n' % exec_line)
    # sub_file.write('\t hadd -f Output_job%d.root %s/*.root \n'%(i,(out)))
    # sub_file.write('\t mv Output_job*.root %s\n'%os.path.abspath(out))
    # sub_file.write('\t cmsMkdir %s/%s \n'%('/store/cmst3/user/pharris/output',options.outdir))
    # sub_file.write('\t cmsStage Output_job%d.root %s/%s \n'%(i,'/store/cmst3/user/pharris/output',options.outdir))
    sub_file.write('\t rm -rf ./bacon ./Output_job* \n')
    sub_file.write('\t touch %s.done\n' % os.path.abspath(sub_file.name))
    sub_file.write('else\n')
    sub_file.write('\t touch %s.fail\n' % os.path.abspath(sub_file.name))
    sub_file.write('fi\n')
    sub_file.write('rm -f %s.run\n' % os.path.abspath(sub_file.name))
    sub_file.write('cd ..\n')
    sub_file.write('rm -rf $TWD\n')
    sub_file.close()
    os.system('chmod +x %s' % os.path.abspath(sub_file.name))


def submit_jobs(lofjobs):
    for sub_file in lofjobs:
        os.system('rm -f %s.done' % os.path.abspath(sub_file))
        os.system('rm -f %s.fail' % os.path.abspath(sub_file))
        os.system('rm -f %s.log' % os.path.abspath(sub_file))
        #os.system('bsub -q %s -o %s.log %s'%(options.queue,os.path.abspath(sub_file),os.path.abspath(sub_file)))
        os.system('bsub -q %s -o /dev/null %s' % (options.queue, os.path.abspath(sub_file)))


if options.monitor:
    if options.monitor not in ['sub', 'check', 'resub']: sys.exit('Error -- Unknown monitor mode %s' % options.monitor)
    dir = options.outdir

    if options.monitor == 'check' or options.monitor == 'resub':
        failjobs = []
        runjobs = []
        donejobs = []
        number_of_jobs = 0
        for root, dirs, files in os.walk(dir):
            for file in fnmatch.filter(files, '*.sh'):
                if os.path.isfile('%s/%s.fail' % (root, file)):
                    failjobs.append('%s/%s' % (root, file))
                elif os.path.isfile('%s/%s.done' % (root, file)):
                    if not '%s.sh' % file in failjobs: donejobs.append('%s/%s' % (root, file))
                elif os.path.isfile('%s/%s.run' % (root, file)):
                    runjobs.append('%s/%s' % (root, file))
                else:
                    failjobs.append('%s/%s' % (root, file))
                number_of_jobs += 1
        print 'Status of jobs directory ', dir
        print '  Total of %d jobs' % number_of_jobs
        print '  %d in status Fail -> (resub them with --monitor resub)' % len(failjobs)
        for job in failjobs: print '\t FAIL %s' % job
        print '  %d in status Running -> ' % len(runjobs)
        for job in runjobs: print '\t RUN %s' % job
        print '  %d in status Done -> ' % len(donejobs)
        for job in donejobs: print '\t DONE %s' % job
        print "\n  %d/%d Running, %d/%d Done, %d/%d Failed (resub with --monitor resub)" \
              % (len(runjobs), number_of_jobs, len(donejobs), number_of_jobs, len(failjobs), number_of_jobs)

    if options.monitor == 'sub' or options.monitor == 'resub':
        # pick up job scripts in output directory (ends in .sh)
        lofjobs = []
        for root, dirs, files in os.walk(dir):
            for file in fnmatch.filter(files, '*.sh'):
                if options.monitor == 'resub' and '%s/%s' % (root, file) not in failjobs: continue
                lofjobs.append('%s/%s' % (os.path.abspath(root), file))
        print 'Submitting %d jobs from directory %s' % (len(lofjobs), dir)
        submit_jobs(lofjobs)

    sys.exit('Finished Monitor -- %s' % options.monitor)


def parse_to_dict(l_list):
    if len(l_list) < 1: return {}
    ret = {}
    nkey = 0
    for item in l_list:
        vargs = item.split(':')  # should put a try here
        ni = vargs[0]
        varg = vargs[1:]
        varg = ":".join(varg)
        if not '-' in ni:
            ni = int(ni)
            nkey += 1
        if not "[" in item and "<" not in item:
            ret[(ni)] = ['', [varg]]
        else:
            if "[" in varg:
                varg = varg.replace("[", "")
                varg = varg.replace("]", "")
                min, max, step = varg.split(",")
                ret[(ni)] = ['', arange(int(min), int(max), int(step))]
            elif "<" in varg:
                varg = varg.replace("<", "")
                varg = varg.replace(">", "")
                largs = varg.split(",")
                ret[(ni)] = ['', largs]

    iskey = 0
    for kr in ret.keys():
        if type(kr) == type(''):
            ll = ret.pop(kr)

            ll[1] = [kr + ' ' + str(l) for l in ll[1]]
            ret[nkey + iskey] = ll
            iskey += 1
    return ret


def getFilesJob(dirin, job, tnjobs):
    if tnjobs == 1:
        tnjobs = -1
        job = 0
    infiles = []
    if "," in dirin:
        alldirs = dirin.split(',')
    else:
        alldirs = [dirin]
    infiles = []
    for dir in alldirs:
        if '/store/' in dir:
            infiles.extend(makeCaFiles(dir, options.blacklist, tnjobs, job))
        else:
            infiles.extend(makeFiles(dir, options.blacklist, tnjobs, job))
    if options.verbose: print "VERB -- Found following files for dir %s --> " % dir, infiles
    return infiles


def getArgsJob(interationsobject, job_id, njobs):
    injobs = []
    #   nf = 0
    ifile = 0
    for ff in iterationsobject:
        if (njobs > 0) and (ifile % njobs != job_id):
            injobs.append((ff, False))
        else:
            injobs.append((ff, True))
        ifile += 1

    return injobs


# -- MAIN
print options.outdir
os.system('mkdir -p %s' % (options.outdir))

mindeces = []
analyzer = args[0]
outfile = args[1]
print options.args
analyzer_args = parse_to_dict(options.args)
print analyzer_args
if options.passSumEntries:
    pos, treenam = options.passSumEntries.split(":")
    numEntries = 0
    if options.directory:
        files = getFilesJob(options.directory.split(":")[1], 0, -1)
    elif options.list:
        with open(options.list.split(":")[1], 'r') as mylist:
            files = [(myfile.replace('\n', ''), True) for myfile in mylist.readlines()]
            #print files
        for fi in files:
            tf = r.TFile.Open(fi[0])
            try:
                tf.IsOpen()
            except:
                continue
            hf = tf.Get(treenam)  # first try to see if its a histogram
            hist = r.TH1F("tmpA", "tmpA", 1, -1000, 1000)
            try:
                getattr(tf, treenam).Draw("Info.nPUmean>>tmpA", "GenEvtInfo.weight")
                numEntries += hist.Integral()
                print "Adding :", hist.Integral(), " -- ", numEntries
            except:
                try:
                    numEntries += int(getattr(tf, treenam).GetEntries())
                    print "Adding Old :", numEntries
                except:
                    continue
    else:
        numEntries = -1;
    analyzer_args[int(pos)] = ['', ['{:e}'.format(float(numEntries))]]
else:
    if options.directory:
        files = getFilesJob(options.directory.split(":")[1], 0, -1)
    elif options.list:
        with open(options.list.split(":")[1], 'r') as mylist:
            files = [(myfile.replace('\n', ''), True) for myfile in mylist.readlines()]

exec_line = '%s' % analyzer

if options.directory:
    filepos, options.directory = options.directory.split(':')
    analyzer_args[int(filepos)] = ['', "fileinput"]
elif options.list:
    filepos, options.list = options.list.split(':')
    analyzer_args[int(filepos)] = ['', "fileinput"]

# NEED TO ITERATE OF MAP OF ARGS, FORGET DEFAULT ARGGS I THINK, forec them set!!!!!
# for arg_i,arg in enumerate(default_args):
sortedkeys = analyzer_args.keys()
if len(sortedkeys): sortedkeys.sort()

for key in sortedkeys:
    #  if arg_i in analyzer_args.keys():
    arg = analyzer_args[key][1]
    if arg == 'fileinput':
        exec_line += ' fileinput '
    elif len(arg) > 1:
        mindeces.append(key)
        exec_line += ' MULTARG_%d ' % key
    else:
        exec_line += ' %s ' % arg[0]

# check that from max to 0 all arguments are accounted for (could always add defaults above) !

for arg_c in range(1, max(analyzer_args.keys())):
    if arg_c not in analyzer_args.keys(): sys.exit("ERROR -- missing argument %d" % arg_c)

print 'running executable -- (default call) \n\t%s' % exec_line

if not options.dryRun and njobs > 0:
    print 'Writing %d Submission Scripts to %s (submit after with --monitor sub)' % (njobs, options.outdir)

if len(files) < njobs:
    njobs = len(files)
for job_i in range(njobs):
    for subjob_i in range(njobs_per_file):
        ################################ WHY does this need to be recreate?
        # This must be the sorted set of keys from the dictionary to build the iterations
        listoflists = [analyzer_args[k][1] for k in sortedkeys]
        # itertools section, make object containing all args to be considered
        # i.e it iterates over all combinations of arguments in the args list
        iterationsobject = product(*listoflists)
        ################################
        if options.directory:
            files = getFilesJob(options.directory, job_i, njobs)
        elif options.list:
            with open(options.list, 'r') as mylist:
                allfiles = [(myfile.replace('\n', ''), True) for myfile in mylist.readlines()]
                #print allfiles
                files = [allfiles[job_i]]
        else:
            files = getArgsJob(iterationsobject, job_i, njobs)  # use itertools to split up any arglists into jobs
        # else: files=[]
        job_exec = ''

        nfiles_i = 0
        for fil_i, fil in enumerate(files):
            # if options.directory :
            if not fil[1]: continue
            if options.directory:
                exec_line_i = exec_line.replace('subjob_i','%d'%subjob_i)
                exec_line_i = exec_line_i.replace('fileinput', " " + fil[0] + " ")
            elif options.list:
                exec_line_i = exec_line.replace('subjob_i','%d'%subjob_i)
                exec_line_i = exec_line_i.replace('fileinput', " " + fil[0] + " ")
            else:
                exec_line_i = exec_line.replace('subjob_i','%d'%subjob_i)
                for i, m in enumerate(
                        fil[0]):  # no defaults so guarantee (make the check) that all of the args are there)
                    exec_line_i = exec_line_i.replace(" MULTARG_%d " % i, " " + str(
                        m) + " ")  # LIST  OVER iterated arguments and produce and replace MULTIARG_i with arguemnt at i in list ?
            if options.eosoutdir:
                if njobs_per_file > 1:                    
                    job_exec += exec_line_i + '; %s %s %s/%s_job%d_file%d_subjob%d.npy; ' % (
                    EOS, outfile, options.eosoutdir, outfile, job_i, fil_i, subjob_i)
                else:
                    job_exec += exec_line_i + '; %s %s %s/%s_job%d_file%d.npy; ' % (
                    EOS, outfile, options.eosoutdir, outfile, job_i, fil_i)
            else:
                if njobs_per_file > 1:
                    job_exec += exec_line_i + '; mv %s %s/%s_job%d_file%d_subjob%d.npy; ' % (
                    outfile, options.outdir, outfile, job_i, fil_i, subjob_i)
                else:
                    job_exec += exec_line_i + '; mv %s %s/%s_job%d_file%d.npy; ' % (
                    outfile, options.outdir, outfile, job_i, fil_i)
            nfiles_i += 1
        if options.verbose: print "VERB -- job exec line --> ", job_exec

        if options.dryRun:
            print 'job %d/%d -> ' % (job_i + 1, njobs), job_exec
        elif options.njobs > 0:
            write_job(job_exec, options.outdir, analyzer, job_i, nfiles_i, subjob_i, options.eosoutdir)
        else:
            print "Running: ", job_exec
            os.system(job_exec)
