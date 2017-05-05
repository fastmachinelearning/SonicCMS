import commands,sys,os,subprocess

siteHandling = {
   ### "cern.ch"    : { "ls"      :  "xrd eoscms.cern.ch ls %s",
   ###                  "prepend" :  "root://eoscms//eos/cms",
   ###                  "field"   : 4
   ###                  },
   "cern.ch"    : { "ls"      :  "/afs/cern.ch/project/eos/installation/cms/bin/eos.select ls %s",
                    "prepend" :  "root://eoscms//eos/cms",
                    "prepend_dir" :  True,
                    "field"   : 0
                    },
   "T2_CH_CSCS" : { "ls"      : "xrd cms01.lcg.cscs.ch ls %s",
                    "prepend" : "root://cms01.lcg.cscs.ch/",
                    "field"   : 4
                    }
   }


def makeCaFiles(dir,blist,njobs=-1,jobid=0,nf=[0],maxfiles=-1,site="cern.ch"):

   dir = str(dir)
   return_files = []

   try:
      ld_path = os.getenv("LD_LIBRARY_PATH")
   except:
      ld_path = ""
      
   if not "/afs/cern.ch/project/eos/installation/pro/lib64/" in ld_path:
      os.putenv("LD_LIBRARY_PATH", "%s:%s" % ( ld_path, "/afs/cern.ch/project/eos/installation/pro/lib64/" ) )

   replace = None
   ls = None
   prepend = None
   if dir.startswith("/castor"):
      ls = 'nsls %s'
      prepend = 'rfio://'
   else:
      sh = siteHandling[site]
      while type(sh) == str:
         sh = siteHandling[sh]         
      ls = sh["ls"]
      prepend = sh.get("prepend",None)
      prepend_dir = sh.get("prepend_dir",None)
      replace = sh.get("replace",None)
      field   = sh.get("field",None)

   sc=None
   for i in range(3):
      sc,flist = commands.getstatusoutput(ls%dir)
      if sc:
         break
      
   files = flist.split('\n')
   if field:
      tmp = []
      for f in files:
         toks = [ t for t in f.split(" ") if t != "" ]
         print toks
         if len(toks) > field:
            tmp.append( toks[field] )
      files = tmp
   files.sort()
      
   if not sc:
      ifile = 0
      for f in files:
      	 blacklisted = False
         for B in blist: 
	   if B in f:
	     blacklisted = True
	     break
         if '.root' in f and not blacklisted:
            if( maxfiles > 0 and ifile >= maxfiles):
               break
            ifile += 1
            nf[0] += 1
            fname = f
            if replace:
               fname = fname.replace( *replace )
            if prepend:
               if prepend_dir:
                  fname = "%s%s/%s" % ( prepend, dir, fname)
               else:
                  fname = "%s%s" % ( prepend, fname)
            if (njobs > 0) and (nf[0] % njobs != jobid):
               return_files.append((fname,False))
	    else:
               return_files.append((fname,True))
   else:
      sys.exit("No Such Directory: %s\n%s\n%s"%(dir,flist,str(files)))

   if nf[0]==0:
      sys.exit("No .root Files found in directory - %s:\n%s"%(dir,flist))

   return return_files


def makeDcFiles(dir,blist,njobs=-1,jobid=0,nf=[0],maxfiles=-1):

   dcache_prepend = 'root://xrootd.grid.hep.ph.ic.ac.uk/'
   dir = str(dir)
   return_files = []

   sc,flist = commands.getstatusoutput('srmls $DCACHE_SRM_ROOT/%s --count 1000'%(dir))
   
   if not sc:
      files = flist.split('\n')
      for f in files:
         if len(f) < 1: continue
         f = (f.split()[-1]).split('/')[-1]
         ifile = 0
      	 blacklisted = False
         for B in blist: 
	   if B in f:
	     blacklisted = True
	     break
         if '.root' in f and not blacklisted:
            if( maxfiles > 0 and ifile >= maxfiles):
               break
            ifile += 1
            nf[0] += 1
            if (njobs > 0) and (nf[0] % njobs != jobid):
               return_files.append((dcache_prepend+dir+'/'+f,False))
	    else:
               return_files.append((dcache_prepend+dir+'/'+f,True))

   else:
      sys.exit("No Such Directory: %s"%(dir))
      
   if nf[0]==0: sys.exit("No .root Files found in directory - %s"%dir)

   return return_files

def unmounteos(dir):
   
   unmount = 'csh -c "eosumount %s "'%dir
   sc,flist = commands.getstatusoutput(unmount) 
   if sc==0: #status must be completed for unmount
      sc,flist = commands.getstatusoutput("rmdir %s"%dir) 
   else:
      sys.exit("Unmount "+dir+" failed. Exiting.")
   

def makeEosFiles(dir,njobs=-1,jobid=0,nf=[0]):
   sys.exit("makeEosFiles not supported anymore")
   
def makeFiles(dir,blist,njobs=-1,jobid=0,nf=[0],maxfiles=-1):

   dir = str(dir)
   return_files = []
#   nf = 0
   if os.path.isdir(dir): 
      files = os.listdir(dir)
      for f in files:
         ifile = 0
      	 blacklisted = False
         for B in blist: 
	   if B in f:
	     blacklisted = True
	     break
         if '.root' in f and not blacklisted:
            if( maxfiles > 0 and ifile >= maxfiles):
               break
            ifile += 1
            nf[0] += 1
            if (njobs > 0) and (nf[0] % njobs != jobid):
               return_files.append((dir+'/'+f,False))
	    else:
               return_files.append((dir+'/'+f,True))
   else: sys.exit("No Such Directory as %s"%dir)  

   if nf[0]==0: sys.exit("No .root Files found in directory - %s"%dir)
   
   return return_files  

if __name__ == "__main__":
   from sys import argv
   site = "cern.ch"
   if len(argv) == 3:
      site = argv.pop(1)
   flist = makeCaFiles(argv[1],site=site)
   for f in flist:
      print f[0]
