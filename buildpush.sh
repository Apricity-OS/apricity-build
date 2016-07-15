iso_name=apricity_os
edition=gnome
username=apricity
remove_prev=true
verbose="-v"

while getopts 'N:V:L:A:D:R:E:w:o:vh' arg; do
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

rm out/*
git checkout dev
git pull origin dev
./build.sh ${verbose} -E ${edition} -R ${remove_prev} -N ${iso_name} | tee logs/${iso_name}.log
ssh server@192.241.147.116 'mkdir -p /mnt/static/public_html/freezedry-build/${username}'
scp out/* server@192.241.147.116:/mnt/static/public_html/freezedry-build/${username}/
scp logs/${iso_name}.log server@192.241.147.116:/mnt/static/freezedry-build/${username}/
rm out/*
