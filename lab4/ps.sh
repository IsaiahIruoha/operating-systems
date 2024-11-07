#!/bin/bash
# ELEC377 - Operating Systems
# Lab 4 - Shell Scripting, ps.sh
# Program Description: Lists running processes and specific details based on user flags.

# initialize flag variables
showRSS="no"
showComm="no"
showCommand="no"
showGroup="no"

# parse command-line args
while [[ "$#" -gt 0 ]]; do
    if [[ "$1" == "-rss" ]]; then
        showRSS="yes"
    elif [[ "$1" == "-comm" ]]; then
        if [[ "$showCommand" == "yes" ]]; then
            echo "Cannot specify both -comm and -command flags."
            exit 1
        fi
        showComm="yes"
    elif [[ "$1" == "-command" ]]; then
        if [[ "$showComm" == "yes" ]]; then
            echo "Cannot specify both -comm and -command flags."
            exit 1
        fi
        showCommand="yes"
    elif [[ "$1" == "-group" ]]; then
        showGroup="yes"
    else
        echo "Invalid flag '$1'"
        exit 1
    fi
    shift
done

# create a temporary file 
temp_file="/tmp/tmpPs$$.txt"

# iterate over numeric directories in /proc
for pid_dir in /proc/[0-9]*; do
    # extract PID from directory name
    pid=$(basename "$pid_dir")
    
    # check if the directory still exists
    if [[ ! -d "$pid_dir" ]]; then
        continue
    fi

    # define paths to status and cmdline files
    status_file="$pid_dir/status"
    cmdline_file="$pid_dir/cmdline"

    # ensure the status file exists and is readable
    if [[ ! -r "$status_file" ]]; then
        continue
    fi

    # initialize default values
    process_rss=0
    process_comm=""
    process_command=""
    uid=""
    gid=""

    # extract name using grep and sed
    process_comm=$(grep "^Name:" "$status_file" | sed -e 's/Name:\s*//')

    # extract VmRSS using grep and sed
    vmrss_line=$(grep "^VmRSS:" "$status_file")
    if [[ -n "$vmrss_line" ]]; then
        process_rss=$(echo "$vmrss_line" | sed -e 's/VmRSS:\s*\([0-9]*\) kB/\1/')
    fi

    # extract Uid using grep and sed then get the first number
    uid=$(grep "^Uid:" "$status_file" | sed -e 's/Uid:\s*//' | awk '{print $1}')

    # extract Gid using grep and sed then get the first number
    gid=$(grep "^Gid:" "$status_file" | sed -e 's/Gid:\s*//' | awk '{print $1}')

    # extract cmdline and replace null characters with spaces
    if [[ -r "$cmdline_file" ]]; then
        process_command=$(tr '\0' ' ' < "$cmdline_file")
        if [[ -z "$process_command" ]]; then
            process_command="$process_comm"
        fi
    else
        process_command="$process_comm"
    fi

    # convert UID to username
    user_name=$(grep "^[^:]*:[^:]*:$uid:" /etc/passwd | cut -d':' -f1)
    if [[ -z "$user_name" ]]; then
        user_name="$uid"
    fi

    # convert GID to group name
    group_name=$(grep "^[^:]*:[^:]*:$gid:" /etc/group | cut -d':' -f1)
    if [[ -z "$group_name" ]]; then
        group_name="$gid"
    fi

    # prepare the output line in order PID, USER, GROUP, RSS, COMMAND/COMMAND_LINE
    output=""
    printf_line="%-8s %-10s"  # Start with fixed width for PID and USER

    # add fields based on flags
    if [[ "$showGroup" == "yes" ]]; then
        output="$output $group_name"
        printf_line+=" %-10s"
    fi
    if [[ "$showRSS" == "yes" ]]; then
        output="$output $process_rss"
        printf_line+=" %-8s"
    fi
    if [[ "$showComm" == "yes" ]]; then
        output="$output $process_comm"
        printf_line+=" %-15s"
    elif [[ "$showCommand" == "yes" ]]; then
        output="$output $process_command"
        printf_line+=" %-15s"
    fi

    # write the formatted output line to the temporary file
    printf "$printf_line\n" "$pid" "$user_name" "$output" >> "$temp_file"
done

# Construct the header with appropriate spacing
header="PID     USER      "
if [[ "$showGroup" == "yes" ]]; then
    header+="GROUP      "
fi
if [[ "$showRSS" == "yes" ]]; then
    header+="RSS       "
fi
if [[ "$showComm" == "yes" ]]; then
    header+="COMMAND        "
elif [[ "$showCommand" == "yes" ]]; then
    header+="COMMAND_LINE   "
fi

# Print the header
echo "$header"

# sort and display the output with formatting
sort -n -k1 "$temp_file"

# clean up the temporary file
rm -f "$temp_file"
