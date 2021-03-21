#!/usr/bin/env bash

# Directory where to save executables
executables_dir="/tmp/fuzzbench"
mkdir -p "$executables_dir"

# Create the list of benchmarks
declare -a benchmarks=(
    "arrow_parquet-arrow-fuzz"
    "bloaty_fuzz_target"
    "curl_curl_fuzzer_http"
    "freetype2-2017"
    "harfbuzz-1.3.2"
    "harfbuzz_hb-subset-fuzzer"
    "jsoncpp_jsoncpp_fuzzer"
    "lcms-2017-03-21"
    "libhevc_hevc_dec_fuzzer"
    "libjpeg-turbo-07-2017"
    "libpcap_fuzz_both"
    "libpng-1.2.56"
    "libxml2-v2.9.2"
    "libxslt_xpath"
    "matio_matio_fuzzer"
    "mbedtls_fuzz_dtlsclient"
    "ndpi_fuzz_ndpi_reader"
    "openexr_openexr_exrenvmap_fuzzer"
    "openh264_decoder_fuzzer"
    "openssl_x509"
    "openthread-2019-12-23"
    "php_php-fuzz-execute"
    "php_php-fuzz-parser"
    "php_php-fuzz-parser-2020-07-25"
    "proj4-2017-08-14"
    "re2-2014-12-09"
    "sqlite3_ossfuzz"
    "stb_stbi_read_fuzzer"
    "systemd_fuzz-link-parser"
    "vorbis-2017-12-11"
    "woff2-2016-05-06"
    "zlib_zlib_uncompress_fuzzer"
)

# Empty fuzzer
export FUZZER_NAME=river
CC='clang'
CXX='clang++'
CFLAGS='-static'
CXXFLAGS='-static'

# Build images and get all executables
for benchmark in "${benchmarks[@]}"
do
    # Create the docker image
    cd ~/fuzzbench/

    # Save a backup and change current compile settings
    cp -n ~/fuzzbench/fuzzers/river/fuzzer.py{,.bak}
    sed -i "s/os.environ\['CC'\] = 'clang'/os.environ\['CC'\] = '$CC'/" ~/fuzzbench/fuzzers/river/fuzzer.py
    sed -i "s/os.environ\['CXX'\] = 'clang++'/os.environ\['CXX'\] = '$CXX'/" ~/fuzzbench/fuzzers/river/fuzzer.py
    sed -i "s/os.environ\['CFLAGS'\] = '-static -fPIE'/os.environ\['CFLAGS'\] = '$CFLAGS'/" ~/fuzzbench/fuzzers/river/fuzzer.py
    sed -i "s/os.environ\['CXXFLAGS'\] = '-static -fPIE'/os.environ\['CXXFLAGS'\] = '$CXXFLAGS'/" ~/fuzzbench/fuzzers/river/fuzzer.py
    
    if ! (make -j2 build-$FUZZER_NAME-$benchmark)
    then
        echo "Error processing $benchmark" | tee -a "$executables_dir/errors"
        continue
    fi    
    cd ~

    # Start a docker container and get the executable
    image_name="gcr.io/fuzzbench/runners/$FUZZER_NAME/$benchmark"
    docker container run -dt --name temp_container --entrypoint /bin/bash "$image_name" > /dev/null

    # Get the executable name
    executable_name=$(docker exec -ti temp_container bash -c 'find . -maxdepth 1 -type f -perm 0755')
    executable_name=$(basename "$executable_name")
    executable_name=${executable_name::-1}

    # Save it in a tar and move it to the host
    docker exec -ti -e EXECUTABLE_NAME="$executable_name" temp_container bash -c 'tar cvf ${EXECUTABLE_NAME}.tar ${EXECUTABLE_NAME}'
    docker cp temp_container:/out/"$executable_name".tar "$executables_dir"
    docker rm -f temp_container > /dev/null

    # Restore the backup
    mv ~/fuzzbench/fuzzers/river/fuzzer.{py.bak,py}

    # Remove docker image
    echo 'y' | docker system prune -a
done

# Archive all executables
(cd "$executables_dir" && tar --concatenate -f executables.tar *.tar)
