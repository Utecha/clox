#!/usr/bin/env fish

set -q build_directory || set build_directory build
set -q build_type || set build_type Debug
set -q generator || set generator "Unix Makefiles"
set -q compiler || set compiler clang
set -q nan_tagging || set nan_tagging OFF
set -q build_original || set build_original ON
set -q program || set program lox

function configure -a refresh
    if test $refresh = true
        cmake -S . \
            -B $build_directory \
            -G $generator \
            -DCMAKE_BUILD_TYPE=$build_type \
            -DCMAKE_C_COMPILER=$compiler \
            -DNAN_TAGGING=$nan_tagging \
            -DBUILD_ORIGINAL=$build_original \
            --fresh
    else
        cmake -S . \
            -B $build_directory \
            -G $generator \
            -DCMAKE_BUILD_TYPE=$build_type \
            -DCMAKE_C_COMPILER=$compiler \
            -DNAN_TAGGING=$nan_tagging \
            -DBUILD_ORIGINAL=$build_original
    end
end

function build -a refresh
    if not test -d $build_directory
        configure $refresh
    else if set -q _flag_config || set -q _flag_fresh
        configure $refresh
    end

    cmake --build $build_directory -j
end

function run -a file
    if test -e $build_directory/$build_type/$program
        if set -q file
            ./$build_directory/$build_type/$program $file
        else
            ./$build_directory/$build_type/$program
        end
    end
end

argparse b/build c/config f/fresh r/run -- $argv
or return

if set -q _flag_fresh
    if not set -q _flag_config
        build true
    else if set -q _flag_config
        configure true
    end
else if set -q _flag_config
    configure false
end

if set -q _flag_build
    if set -q _flag_fresh
        build true
    else
        build false
    end
end

if set -q _flag_run
    set matched (string match -- '*.lox' $argv)

    if test (count $matched) -ge 1
        run $matched[1]
    else
        run
    end
end
