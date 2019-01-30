from Condor.Production.jobSubmitter import *

class jobSubmitterSonic(jobSubmitter):
    def addExtraOptions(self,parser):
        super(jobSubmitterSonic,self).addExtraOptions(parser)
        
        parser.add_option("-i", "--input", dest="input", default="", help="input miniAOD file (default = %default)")
        parser.add_option("-o", "--output", dest="output", default="", help="path to output directory in which root files will be stored (required) (default = %default)")
        parser.add_option("-J", "--jobName", dest="jobName", default="sonic", help="job name (default = %default)")
        parser.add_option("-N", "--nJobs", dest="nJobs", default=1, help="number of jobs to submit (default = %default)")
        parser.add_option("-A", "--args", dest="args", default="", help="additional common args to use for all jobs (default = %default)")

    def checkExtraOptions(self,options,parser):
        super(jobSubmitterSonic,self).checkExtraOptions(options,parser)

        if len(options.input)==0:
            parser.error("Required option: --input [file]")
        if len(options.output)==0:
            parser.error("Required option: --output [directory]")

    def generateExtra(self,job):
        super(jobSubmitterSonic,self).generateExtra(job)
        job.patterns.update([
            ("JOBNAME",job.name+"_$(Process)_$(Cluster)"),
            ("EXTRAINPUTS","input/args_"+job.name+"_$(Process).txt"),
            ("EXTRAARGS","-j "+job.name+" -p $(Process)"+" -i "+self.input+" -o "+self.output),
        ])

    def generateSubmission(self):
        # create protojob
        job = protoJob()
        job.name = self.jobName
        self.generatePerJob(job)
        for iJob in range(int(self.nJobs)):
            job.njobs += 1
            if self.count and not self.prepare:
                continue
            job.nums.append(iJob)
            if self.missing and not self.prepare:
                continue
            if self.prepare:
                jname = job.makeName(job.nums[-1])
                with open("input/args_"+jname+".txt",'w') as argfile:
                    args = (self.args+" " if len(self.args)>0 else "")
                    argfile.write(args)
        # append queue comment
        job.queue = "-queue "+str(job.njobs)
        # store protojob
        self.protoJobs.append(job)
