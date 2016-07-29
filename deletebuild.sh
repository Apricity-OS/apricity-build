# path=/mnt/static/public_html/freezedry-build/test/apricity_os-test.iso

while getopts 'N:U:h' arg; do
    case "${arg}" in
        N) iso_name="${OPTARG}" ;;
        U) username="${OPTARG}" ;;
        *)
           echo "Invalid argument '${arg}'" ;;
    esac
done

sudo ssh server@192.241.147.116 "rm /mnt/static/public_html/freezedry-build/${username}/apricity_os-${iso_name}.iso"
