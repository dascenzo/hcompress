#!/bin/bash
#todo: shellcheck

set -euo pipefail

program_name="hcompress"
usage="usage: $0 [-h] [-p|--show-passing] [-s|--stop-on-failure] <debug|release>"

stop_on_failure="0"
show_passing="0"
build=""

test_name=""
pass_count="0"
failure_count="0"

stdout_file="out.txt"
stderr_file="err.txt"
expected_file="expected.txt"
input_dir=""

error_exit() {
  echo "$0: $1" >&2
  exit 1
}
file_size() {
  stat -f "%z" "$1"
}
cp_input_file() {
  cp "$input_dir/$1" .
}
while [ $# -gt 0 ]; do
  case "$1" in
    -h)
      echo "$usage"
      exit
      ;;
    -p|--show-passing)
      show_passing="1"
      shift
      ;;
    -s|--stop-on-failure)
      stop_on_failure="1"
      shift
      ;;
    debug|release)
      if [ -z "$build" ]; then
        build="$1"
      else
        error_exit "multiple build options listed"
      fi
      shift
      ;;
    *)
      error_exit "unknown option: $1"
      ;;
  esac
done
if [ -z "$build" ]; then
  error_exit "build option not given"
fi

# Make sure we can find the required files
if [ -d input-files ]; then
  # in test/.
  exe="../../build-$build/$program_name"
  input_dir="../input-files"
elif [ -d test/input-files ]; then
  # in test/..
  exe="../build-$build/$program_name"
  input_dir="../test/input-files"
else
  error_exit "please cd into test/.. or test/. to run the tests."
fi

temp_dir=$(mktemp -d "./tmp.d.XXXX")
if [ $stop_on_failure = "0" ]; then
  trap 'rm -rf -- "../$temp_dir"' EXIT
fi
cd $temp_dir

if ! [ -f "$exe" ]; then
  error_exit "Can't find the executable. Is the $build build built?"
fi

# === base tester functions ===
TEST() {
  local msg="$1"
  shift
  if $@ ; then
    pass_count=$((pass_count + 1))
    if [ "$show_passing" != "0" ]; then
      echo "pass($test_name) $msg"
    fi
  else
    failure_count=$((failure_count + 1))
    echo "FAIL($test_name) $msg"
    if [ $stop_on_failure = "1" ]; then
      error_exit "Exiting...test failed and \"stop on failure\" flag is true." 
    fi
  fi
}
# $1 - msg, $2 - expected exit code, $3 - expected output text, $@ program arguments
TEST_EXE() {
  local msg="$1"
  local expected_code="$2"
  local expected_output="$3"
  shift 3

  eval $exe $@ > "$stdout_file" 2> "$stderr_file" && true
  local actual_code="$?"
  TEST "$msg: result code: $expected_code = $actual_code" \
      [ $expected_code = $actual_code ]

  if [ "$expected_code" = "0" ]; then
    printf "%s" "$expected_output" > "$expected_file"
    TEST "$msg: stdout" cmp -s $stdout_file $expected_file

    printf "%s" "" > "$expected_file"
    TEST "$msg: stderr" cmp -s $stderr_file $expected_file
  else
    printf "%s" "" > "$expected_file"
    TEST "$msg: stdout" cmp -s $stdout_file $expected_file

    printf "%s" "$expected_output" > "$expected_file"
    TEST "$msg: stderr" cmp -s $stderr_file $expected_file
  fi
}
TEST_EXISTS() {
  local file=$1
  local original_exists=$2
  local compressed_exists=$3

  if [ $original_exists = "1" ]; then
    TEST "original $file exists" [ -f $file ]
  else
    TEST "original $file does not exist" [ ! -f $file ]
  fi
  if [ $compressed_exists = "1" ]; then
    TEST "compression file $file.hz exists" [ -f $file.hz ]
  else
    TEST "compression file $file.hz does not exist" [ ! -f $file.hz ]
  fi
}
# $1 - file, $2 - compare, $3 - keep, $4 - expected output, $@ - flags
TEST_COMPRESS() {
  local file=$1
  local compare=$2
  local keep=$3
  local expected_output="$4"
  local original_size=$(file_size $file)
  local golden_size=$(file_size $input_dir/$file.hz.golden)
  shift 4 

  TEST_EXE "compress $file" "0" "$expected_output" $@ $file
  TEST_EXISTS $file $keep 1

  TEST "$file.hz compressed data" \
      cmp -n $golden_size -s $file.hz "$input_dir/$file.hz.golden"
  TEST "$file.hz $compare $file" [ $(file_size $file.hz) $compare $original_size ]
}
# $1 - file, $2 - keep, $3 - expected output, $@ - flags in addition to -d
TEST_DECOMPRESS() {
  local file=$1
  local keep=$2
  local expected_output="$3"
  shift 3
  # add -d flag if needed
  case "$*" in
    *d*)
      ;;
    *)
      set -- $@ -d
      ;;
  esac
  TEST_EXE "decompress $file.hz" "0" "$expected_output" $@ $file.hz
  TEST_EXISTS $file 1 $keep
  TEST "$file decompressed correctly" cmp -s $file $input_dir/$file
}
# $1 - file, $2 - compare
TEST_COMPRESSION() {
  local file=$1
  local compare=$2
  cp_input_file $file

  TEST_COMPRESS   $file $compare 0 ""
  TEST_DECOMPRESS $file 0 ""

  rm $file*
}

# === functions that test various functionality ===
test_keep_flag() {
  test_name="test keep flag"
  local file=medium-english.txt
  cp_input_file $file

  TEST_COMPRESS $file -lt 1 "" -k

  rm $file
  TEST_DECOMPRESS $file 1 "" -k

  rm $file*
}
test_change_word_size() {
  test_name="change word size"
  local file=medium-english.txt2
  cp_input_file $file

  TEST_EXE "compress $file" "0" "" $file
  local compressed_size1=$(file_size $file.hz)
  rm $file.hz

  cp_input_file $file
  TEST_COMPRESS $file -lt 0 "" -b 2

  TEST "increasing word size for language data decreases compressed size" \
     [ $(file_size $file.hz) -lt $compressed_size1 ]

  TEST_DECOMPRESS $file 0 ""

  rm $file*
}
test_word_size_gt_file() {
  test_name="word size larger than file"
  local file=small.txt10
  cp_input_file $file

  TEST_COMPRESS $file -gt 0 "" -b 10
  TEST_DECOMPRESS $file 0 ""

  rm $file*
}
test_keep_flag() {
  test_name="test keep flag"
  local file=medium-english.txt
  cp_input_file $file

  TEST_COMPRESS $file -lt 1 "" -k

  rm $file
  TEST_DECOMPRESS $file 1 "" -k

  rm $file*
}
test_verbose_flag() {
  test_name="test verbose flag"
  local file=$1
  local compare=$2
  local compress_output=$3
  local decompress_output=$4
  cp_input_file $file

  TEST_COMPRESS $file $compare 0 "$compress_output" -v
  TEST_DECOMPRESS $file 0 "$decompress_output" -v

  rm $file*
}
test_all_flags_together() {
  test_name="test all flags together"
  local compress_flags="$1"
  local decompress_flags="$2"
  local file=medium-english.txt2
  local compress_output=\
"in:	44007
out:	24593
saving:	44.1%
"
  local decompress_output=\
"in:	24593
out:	44007
saving:	44.1%
"
  cp_input_file $file

  TEST_COMPRESS $file -lt 1 "$compress_output" $compress_flags

  rm $file
  TEST_DECOMPRESS $file 1 "$decompress_output" $decompress_flags

  rm $file*
}
test_runtime_errors() {
  test_name="runtime errors"
  local file=medium-english.txt
  cp_input_file $file

  TEST_EXE "file doesn't exist" "1" \
      "$exe: error occurred while trying to open file \"foobar.txt\": No such file or directory
"      foobar.txt

  TEST_EXE "decompress .txt file" "1" \
      "$exe: unknown extension for file: $file
"      -d $file

  touch $file.hz
  TEST_EXE "output file already exists" "1" \
      "$exe: error occurred while trying to open file \"$file\": File exists
"      -d $file.hz

  rm $file
  TEST_EXE "decompress invalid file" "1" \
      "$exe: error occurred while trying to read file \"$file.hz\": invalid compression header: error occurred while trying to read stream
"      -d $file.hz
  rm $file*
}
test_invalid_usage() {
  test_name="invalid usage"
  local file=medium-english.txt
  cp_input_file $file

  TEST_EXE "no file given" "1" "$program_usage"
  TEST_EXE "-h and file given" "1" "$program_usage" -h $file
  TEST_EXE "file not given in end position" "1" "$program_usage" $file -k
  TEST_EXE "-b at end" "1" "$program_usage" -b

  # invalid -b argument
  TEST_EXE "invalid -b argument: 1000000000" "1" \
     "$exe: invalid argument to -b option: \"1000000000\" (too big)
"   -b 1000000000 $file
  TEST_EXE "invalid -b argument: 0" "1" \
     "$exe: invalid argument to -b option: \"0\" (can't be 0)
"   -b 0 $file
  TEST_EXE "invalid -b argument: abc" "1" \
     "$exe: invalid argument to -b option: \"abc\" (contains non-digits)
"   -b abc $file

  TEST_EXE "invalid -b argument: empty" "1" \
     "$exe: invalid argument to -b option: \"\" (empty)
"   -b "''" $file

  # double flags
  TEST_EXE "invalid flags: -d -d given" "1" "$program_usage" -d -d $file
  TEST_EXE "invalid flags: -b 10 -b 10 given" "1" "$program_usage" -b 10 -b 10 $file
  TEST_EXE "invalid flags: -k -k given" "1" "$program_usage" -k -k $file
  TEST_EXE "invalid flags: -v -v given" "1" "$program_usage" -v -v $file

  # invalid flag combination
  TEST_EXE "invalid flags: -k -b 10 -v -h given " "1" "$program_usage" -k -b 10 -v -h $file
  TEST_EXE "invalid flags: -d -k -v -h given " "1" "$program_usage" -d -k -v -h $file

  rm $file
}

# === start actual tests ===

test_name="test small file"
TEST_COMPRESSION small.txt -gt

test_name="test medium language file"
TEST_COMPRESSION medium-english.txt -lt

test_name="test empty file"
TEST_COMPRESSION empty.txt -gt

# all that's stored in compressed file is the word and count
test_name="test one word file" 
TEST_COMPRESSION one-word.txt -lt

test_keep_flag
test_change_word_size
test_word_size_gt_file

test_name="test file with all possible bytes in it"
TEST_COMPRESSION all-char.txt -lt

test_verbose_flag medium-english.txt -lt \
"in:	44007
out:	24880
saving:	43.5%
" \
"in:	24880
out:	44007
saving:	43.5%
"
test_verbose_flag small.txt -gt \
"in:	4
out:	30
saving:	-650.0%
" \
"in:	30
out:	4
saving:	-650.0%
"
# shows byte change
test_verbose_flag empty.txt -gt \
"in:	0
out:	11
saving:	-11
" \
"in:	11
out:	0
saving:	-11
"
test_all_flags_together "-kv -b 2" "-dkv"
# mix up flags
test_all_flags_together "-b 2 -v -k" "-v -d -k"
test_runtime_errors

program_usage="usage: hcompress [-dkhv] [-b num] [file]
  -d     Decompress file, else compress.
  -b num Specify symbol length for encoding (during compression) to be num bytes. 1 by default.
  -k     Keep input file. Normally input file is deleted.
  -v     Verbose mode. Print statistics.
  -h     Show this help text.
"
test_name="test help flag"
TEST_EXE "-h" "0" "$program_usage" -h

test_invalid_usage

if [ $failure_count -gt 0 ]; then
  echo
  echo "There are test failures! (BUILD: $build)"
  echo "Pass: $pass_count, Fail: $failure_count"
  echo
  exit 1
else
  echo
  echo "All tests passed. (BUILD: $build)"
  echo "Pass: $pass_count, Fail: $failure_count"
  echo
  exit 0
fi
