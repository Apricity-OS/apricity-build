iso_name=apricity_os
edition=gnome
username=apricity
remove_prev=true
verbose="-v"

while getopts 'N:U:V:L:A:D:R:E:w:o:vh' arg; do
    case "${arg}" in
        N) iso_name="${OPTARG}" ;;
        R) remove_prev="${OPTARG}" ;;
        E) edition="${OPTARG}" ;;
        U) username="${OPTARG}" ;;
        v) verbose="-v" ;;
        *)
           echo "Invalid argument '${arg}'" ;;
    esac
done

sudo rm out/*
git checkout dev
git pull origin dev
sudo ./build.sh ${verbose} -E ${edition} -R ${remove_prev} -N ${iso_name} 2>&1 | tee logs/apricity_os-${iso_name}.log
sudo ssh server@192.241.147.116 "mkdir -p /mnt/static/public_html/freezedry-build/${username}"
sudo scp out/* server@192.241.147.116:/mnt/static/public_html/freezedry-build/${username}/
sudo scp logs/apricity_os-${iso_name}.log server@192.241.147.116:/mnt/static/public_html/freezedry-build/${username}/
# sudo ssh server@192.241.147.116 "rm /mnt/static/public_html/freezedry-build/${username}/apricity_os-${iso_name}.iso"
sudo rm out/*
sudo rm -rf work
