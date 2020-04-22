from jobSubmitterSonic import jobSubmitterSonic

def submitJobs():  
    mySubmitter = jobSubmitterSonic()
    mySubmitter.run()
    
if __name__=="__main__":
    submitJobs()
