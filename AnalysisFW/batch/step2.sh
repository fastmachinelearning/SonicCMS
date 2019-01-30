#!/bin/bash

# from https://github.com/kpedro88/utilities/blob/master/eosdu
FIXFIND='{lfn=$1; while(substr(lfn,1,6)!="/store"&&length(lfn)>=6) { lfn = substr(lfn,2) }; print lfn}'

export JOBNAME=""
export PROCESS=""
export INFILE=""
export OUTDIR=""
export OPTIND=1
while [[ $OPTIND -lt $# ]]; do
	# getopts in silent mode, don't exit on errors
	getopts ":j:p:i:o:" opt || status=$?
	case "$opt" in
		j) export JOBNAME=$OPTARG
		;;
		p) export PROCESS=$OPTARG
		;;
		o) export OUTDIR=$OPTARG
		;;
		i) export INFILE=$OPTARG
		;;
		# keep going if getopts had an error
		\? | :) OPTIND=$((OPTIND+1))
		;;
	esac
done

echo "parameter set:"
echo "INFILE:     $INFILE"
echo "OUTDIR:     $OUTDIR"
echo "JOBNAME:    $JOBNAME"
echo "PROCESS:    $PROCESS"
echo ""

cd $CMSSW_BASE/src/SonicCMS/AnalysisFW/python
scram b ExternalLinks
mkdir -p data

# prepend redirector
if [[ "$INFILE" == "/store/"* ]]; then
	INFILE=root://cmsxrootd.fnal.gov/${INFILE}
fi

# fetch input file if necessary
if [[ "$INFILE" == "root://"* ]]; then
	LFN=$(echo "$INFILE" | awk "$FIXFIND")
	# from https://github.com/kpedro88/utilities/blob/master/xrdcpLocal.sh
	FN=`echo ${LFN} | sed 's~/~_~g'`
	FN=${FN:1:${#FN}-1}
	xrdcp -f "$INFILE" data/${FN}
	INFILE=file:data/${FN}
fi

# check tools
ls -lth $CMSSW_BASE/external/slc6_amd64_gcc700/lib
scram tool info grpc
scram tool info tensorflow-serving
ldd $CMSSW_BASE/lib/slc6_amd64_gcc700/pluginJetImageProducer.so

# run CMSSW
LOG=log_${JOBNAME}_${PROCESS}.log
ARGS=$(cat $_CONDOR_SCRATCH_DIR/args_${JOBNAME}_${PROCESS}.txt)
ARGS="$ARGS inputFiles=${INFILE}"
echo "cmsRun jetImageTest_mc_cfg.py ${ARGS} >& ${LOG}"
cmsRun jetImageTest_mc_cfg.py ${ARGS} >& ${LOG}

CMSEXIT=$?

if [[ $CMSEXIT -ne 0 ]]; then
  cat *.log
  echo "exit code $CMSEXIT, skipping xrdcp"
  exit $CMSEXIT
fi

# copy output to eos
echo "xrdcp output for condor"
for FILE in *.log; do
	echo "xrdcp -f ${FILE} ${OUTDIR}/${FILE}"
	xrdcp -f ${FILE} ${OUTDIR}/${FILE} 2>&1
	XRDEXIT=$?
	if [[ $XRDEXIT -ne 0 ]]; then
		cat *.log
		echo "exit code $XRDEXIT, failure in xrdcp"
		exit $XRDEXIT
	fi
	rm ${FILE}
done

