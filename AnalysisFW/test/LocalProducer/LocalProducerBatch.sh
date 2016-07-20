
cwd=$(pwd)

# CMSSW environment
#cd /afs/cern.ch/user/m/mhaapale/work/public/CMSSW_5_3_20
#eval `scramv1 runtime -sh`
#cd -

# ROOT 6
echo "Setting up environment"
source /afs/cern.ch/sw/lcg/app/releases/ROOT/6.04.18/x86_64-slc6-gcc49-opt/root/bin/thisroot.sh
source /afs/cern.ch/sw/lcg/contrib/gcc/4.9/x86_64-slc6/setup.sh


# Changing directory
cd /afs/cern.ch/user/m/mhaapale/work/public/CMSSW_5_3_32/src/cms-opendata-2011-jets/AnalysisFW/test/LocalProducer


#echo "EOS environment"
#source /afs/cern.ch/project/eos/installation/cms/etc/setup.sh

# Mount EOS
#echo "Mounting EOS"
#mkdir eos
#eosmount eos
#cd eos/cms/store/group/phys_smp/mhaapale/Jet/ProcessedTree_all/160627_140719/0000

echo "Running ROOT script"
root LocalOpenDataTreeProducer.C


#echo "Unmounting EOS"
#cd $cwd
#eosumount eos/
