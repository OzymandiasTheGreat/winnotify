if [ "$MSYSTEM" == "MINGW64" ]; then
	export PYTHONINC="/mingw64/include/python3.6m"
elif [ "$MSYSTEM" == "MINGW32" ]; then
	export PYTHONINC="/mingw32/include/python3.6m"
else
	exit 1
fi
set -a
source ".vscode/params"
set +a
mkdir "build"
gcc -g -I"$PYTHONINC" -c "${CSRCDIR}/${FILE}.c" -o "build/${FILE}.o"
windres --input "${CSRCDIR}/${FILE}.rc" --output "build/${FILE}.res" --output-format=coff
gcc -g -shared -o "${PACKAGE}/${FILE}.pyd" "build/${FILE}.res" "build/${FILE}.o" -mwindows -lcomctl32 -lpython3.6m
rm -rf "build"
