

cwd=$(pwd)

# Environment
echo "CMSSW environment"
cd /afs/cern.ch/user/m/mhaapale/work/public/CMSSW_5_3_20
eval `scramv1 runtime -sh`
cd -


# Top-level directory
cd ~/work/public/CMSSW_5_3_32/src/cms-opendata-2011-jets/AnalysisFW/test/MCTuples

# Executing script
root mergeMC.C

cd $cwd

echo "Success!"
