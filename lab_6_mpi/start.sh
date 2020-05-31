#!/bin/bash
config_filename=config.conf
while true; do
  case $1 in
    -f|--file)
      config_filename=$2
      shift 2
    ;;
    -np|--number_of_processes)
      number_of_processes=$2
      shift 2
    ;;
    -D|--debug-build)
      debug_build=true;
      shift
    ;;
    -d|--debug)
      debug=true;
      shift
    ;;
    -O|--optimize-build)
      optimize_build=true;
      shift
    ;;
    -h|--help)
      echo "Usage: ./start.sh [options]
  Options:
    -O   --optimize-build  Compile with optimization before executing
    -D   --debug-build     Compile with debug options
    -d   --debug           Run executable with debug symbols
    -f    --file           Config filename
    -h    --help           Show help message"
      exit 0;
    ;;
    \?)
      echo "Invalid option: -$OPTARG" >&2
      exit 1 ;;
    :)
      echo "Option -$OPTARG requires an numerical argument." >&2
      exit 1 ;;
    *)
      break
      ;;
  esac
done


rm -rf ./res;
mkdir -p ./res;

python3 field/field_generation.py "$config_filename"

if [[ "$debug_build" = true ]]; then
  mkdir -p ./cmake-build-debug;
  pushd ./cmake-build-debug  > /dev/null || exit 1
  echo Compiling...
  cmake -lpng -DCMAKE_BUILD_TYPE=Debug -G"Unix Makefiles" .. || exit 1
  make || exit 1
  popd
fi;

if [[ "$optimize_build" = true ]]; then
  mkdir -p ./cmake-build-release;
  pushd ./cmake-build-release  > /dev/null || exit 1
  echo Compiling...
  cmake -lpng -DCMAKE_BUILD_TYPE=Release -G"Unix Makefiles" .. || exit 1
  make || exit 1
  popd
fi;

if [[ "$debug_build" = true ]]; then
    mpirun --allow-run-as-root -np "$number_of_processes" ./cmake-build-debug/mpi_heat_transfer "$config_filename" || exit 1
else
    mpirun --allow-run-as-root -np "$number_of_processes" ./cmake-build-release/mpi_heat_transfer "$config_filename" || exit 1
fi
