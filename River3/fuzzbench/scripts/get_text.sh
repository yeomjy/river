# Commands to run
declare -a cmds=(
    "objdump -D -M intel"
    "nm"
    "readelf -a"
)

# Avoid * when globbing
shopt -s nullglob

# Get the folder name that contains an archive
# with all the executables
if ! [[ $# -eq 1 ]]
then
    echo "Usage: ./$0 /path/to/archive"
    exit 1
fi

directory_name=''
archive_name=''

# Check if the supplied file is a directory or tar file
if [[ -d "$1" ]]
then
    directory_name=$(realpath "$1")
    archive_name="$(ls "$1"*.tar | head -1)"

    if [[ -z "$archive_name" ]]
    then
        echo "No tar archive was found in $1"
        exit 2
    fi
else
    if [[ -f "$1" ]]
    then
        filename=$(basename -- "$1")
        extension="${filename##*.}"
        filename="${filename%.*}"


        if ! [[ $extension != '.tar' ]] 
        then
            echo "The specified file is not a tar archive"
            exit 3
        fi

        archive_name=$1
        directory_name=$(dirname "$1")
    fi
fi

# Extract the binaries to directory_name/binaries
mkdir $directory_name/binaries
tar ixvf $archive_name -C $directory_name/binaries

for cmd in "${cmds[@]}"
do
    cmd_name=${cmd%% *}
    mkdir -p $directory_name/$cmd_name

    for binary in `ls -1 -d $directory_name/binaries/*`
    do
        binary_name=$(basename $binary)
        echo "$cmd $binary > $directory_name/$cmd_name/$binary_name.txt"
        eval "$cmd $binary > $directory_name/$cmd_name/$binary_name.txt"
    done
done
