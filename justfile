set quiet := true
set shell := ["bash", "-c"]

# Pretty Colors

red := "\\x1b[31m"
green := "\\x1b[32m"
yellow := "\\x1b[33m"
reset := "\\x1b[0m"
include_dir := "include"
src_dir := "src"
out_dir := "out"
bin_dir := "bin"
include_flags := '-I' + include_dir
debug_shared_flags := '-ggdb -g -Og -fsanitize=address,undefined,leak -fno-sanitize-recover=all -fno-omit-frame-pointer -fno-optimize-sibling-calls -fsanitize-address-use-after-scope -fno-common -std=gnu11'
debug_compile_flags := debug_shared_flags + ' -Wall -Wextra -Wpedantic -Wno-unused-parameter'
debug_link_flags := debug_shared_flags + ' -static-libasan'
release_shared_flags := '-Ofast -std=gnu11'
release_compile_flags := release_shared_flags
release_link_flags := release_shared_flags

# Default justfile target (lists all available targets)
default:
    just --list

# Compiles all the C files in the src directory
compile type="debug" force="false" threads=num_cpus():
    #!/usr/bin/env bash
    shopt -s globstar

    [[ -d {{ out_dir }} ]] || mkdir -p {{ out_dir }}

    WILL_COMPILE=false

    for file in {{ src_dir }}/**/*.c; do
      if [[ $file -nt  {{ out_dir }}/$(basename "${file%.c}")-{{ type }}.o ]]; then
        WILL_COMPILE=true
      fi

      if ! [[ -f {{ out_dir }}/$(basename "${file%.c}")-{{ type }}.o ]]; then
        WILL_COMPILE=true
      fi
    done

    if [[ {{ force }} == "force" || {{ force }} == "true" ]]; then
      echo -e "Compile: Forcing"
      WILL_COMPILE=true
    fi

    if [[ $WILL_COMPILE == false ]]; then
      echo -e "Compile: Nothing to do"
      exit 0
    fi

    echo -e "Using {{ red }}{{ threads }}{{ reset }} threads"
    echo -e "Target: {{ green }}{{ type }}{{ reset }}"
    if [[ {{ type }} == "debug" ]]; then
      find {{ src_dir }} -name "*.c" -print0 | xargs -0 -P{{ threads }} -n1 \
        sh -c 'if [[ "$1" -nt "{{ out_dir }}/$(basename "${1%.c}")-debug.o" || {{ force }} == "true" || {{ force }} == "force" ]]; then echo -e "Compiling {{ green }}$1{{ reset }}..."; gcc {{ include_flags }} {{ debug_compile_flags }} -c "$1" -o "{{ out_dir }}/$(basename "${1%.c}")-debug.o"; fi' sh
    else
      find {{ src_dir }} -name "*.c" -print0 | xargs -0 -P{{ threads }} -n1 \
        sh -c 'if [[ "$1" -nt "{{ out_dir }}/$(basename "${1%.c}")-release.o" || {{ force }} == "true" || {{ force }} == "force" ]]; then echo -e "Compiling {{ green }}$1{{ reset }}..."; gcc {{ include_flags }} {{ release_compile_flags }} -c "$1" -o "{{ out_dir }}/$(basename "${1%.c}")-release.o"; fi' sh
    fi

# Links all the object files in the out directory based on the type of build (debug or release)
link type="debug":
    #!/usr/bin/env bash

    [[ -d {{ out_dir }} ]] || just compile {{ type }}
    [[ -d {{ bin_dir }} ]] || mkdir -p {{ bin_dir }}

    WILL_LINK=false
    if [[ {{ type }} == "debug" ]]; then
      for file in {{ out_dir }}/*-debug.o; do
        if [[ $file -nt {{ bin_dir }}/httiny-debug ]]; then
          WILL_LINK=true
        fi
      done
    else
      for file in {{ out_dir }}/*-release.o; do
        if [[ $file -nt {{ bin_dir }}/httiny ]]; then
          WILL_LINK=true
        fi
      done
    fi

    if [[ $WILL_LINK == false ]]; then
      echo -e "Link: Nothing to do"
      exit 0
    fi

    FILES=$(find {{ out_dir }} -name "*-{{ type }}.o")

    echo -e "Linking {{ green }}{{ type }}{{ reset }}"
    echo -e "Files to link: {{ green }}"
    printf "%s\n" "${FILES[@]}" | sed 's/^/    /'
    echo -e "{{ reset }}"

    if [[ {{ type }} == "debug" ]]; then
      gcc {{ out_dir }}/*-debug.o {{ debug_link_flags }} -o {{ bin_dir }}/httiny-debug -fuse-ld=mold
    else
      gcc {{ out_dir }}/*-release.o {{ release_link_flags }} -o {{ bin_dir }}/httiny -fuse-ld=mold
    fi

    echo -e "Linking {{ green }}{{ type }}{{ reset }} complete"

# Builds the project with the given type and force flags
build type="debug" force="false" threads=num_cpus():
    just compile {{ type }} {{ force }} {{ threads }}
    just link {{ type }}

# Alias for building the project in debug mode
debug force="false" threads=num_cpus():
    just build debug {{ force }} {{ threads }}

# Alias for building the project in release mode
release force="false" threads=num_cpus():
    just build release {{ force }} {{ threads }}

# Runs the project with the given type and force flags
run type="debug" force="false" threads=num_cpus():
    #!/usr/bin/env bash
    just build {{ type }} {{ force }} {{ threads }}

    if [[ {{ type }} == "debug" ]]; then
      ./bin/httiny-debug
    else
      ./bin/httiny
    fi

# Cleans the out directory
clean:
    rm -rf {{ out_dir }}

# Uses bear to generate compile_commands.json
bear:
    bear -- just compile debug force
    sed -i 's|"/nix/store/[^"]*gcc[^"]*|\"gcc|g' compile_commands.json
