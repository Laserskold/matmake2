mkdir -p build

CXX=$1

if [ -z "$CXX" ]
then
	echo No compiler specified: Guessing...

	# Select the latest installed version of clang++ or g++
	for ((i = 20; i >= 6; --i))
	do
		CLANG="clang++-$i"
		if ! ! command -v ${CLANG} &> /dev/null
		then
			echo found $CLANG
			CXX=$CLANG
			break;
		fi

		GCC="g++-$1"
		if ! ! command -v ${GCC} &> /dev/null
		then
			echo found $GCC
			CXX=$GCC
			break;
		fi
	done;


	# Test without numbers
	if [ -z "$CXX" ]
	then
		if ! ! command -v clang++ &> /dev/null
		then
			echo found clang++
			CXX=clang++
		elif ! ! command -v g++ &> /dev/null
		then
			echo found g++
			CXX=g++
		fi
	fi
fi


echo building with ${CXX}...

${CXX} -o build/matmake2 matmake.cpp \
        -Isrc -Ilib/json.h/include -std=c++17 -pthread -g

echo done...
