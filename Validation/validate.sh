
topLevelValDir=$(pwd)
testDir='Csc_valDir'

makefile='makefile'
target='tests'
targetClean='clean'

runFile='csc_testRun.sh'

cleanOut='[Cc]sc_temp_*'

topLevelOutFile=$topLevelValDir/testOut.txt
testOut='csc_testOut.txt'

topLevelFailsFile=$topLevelValDir/testFails.txt
testFail='csc_testFails.txt'


function removeIfPresent ()
{	for i in $* 
	do
		if [ -f $i -o -d $i ]
		then
			rm -r $i
		fi
	done
}


#  Any files or directories in the top level validation directory tree
#  recursively matching any of the patterns "csc_temp_*", "Csc_temp_*",
#  "csc_testOut_*.txt", or "csc_testFails_*.txt",  will be recursively
#  removed.
cd $topLevelValDir

# Remove old files.
removeIfPresent $topLevelOutFile $topLevelFailsFile
find . \( -name $cleanOut -or -name $testOut -or -name $testFail \) -delete

# Find the lower level test directories.
valDirs=$( find . -name $testDir -print )

# For each lower level test directory.
for valDir in $valDirs
do
	cd $topLevelValDir
	cd $valDir
	
#  If a file by the name of "makefile" is found in this directory (not
#  recursively), then invoke "make tests" will be invoked.
	if [ -f $makefile ]
	then
		make $target
	fi

#  If a file by the name of "csc_testRun.sh" is found in this
#  directory (not recursively), then it will be invoked.
	if [ -f $runFile ]
	then
		bash $runFile
	fi

# Find any resulting output files and concatenate them into the top level out file.
	echo >> $topLevelOutFile
	echo '['"$valDir"']' >> $topLevelOutFile
	outFiles=$( find . -name $testOut )
	for outFile in $outFiles
	do
		cat $outFile >> $topLevelOutFile
	done

# Find any resulting failure files and concatenate them into the top level failure file.
	echo >> $topLevelFailsFile
	echo '['"$valDir"']' >> $topLevelFailsFile
	failFiles=$( find . -name $testFail )
	for failFile in $failFiles
	do
		cat $failFile >> $topLevelFailsFile
	done

# Cleanup
	if [ -f $makefile ]
	then
		make $targetClean
	fi
	find . \( -name $cleanOut -or -name $testOut -or -name $testFail \) -delete

done

